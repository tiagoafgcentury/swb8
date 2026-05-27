#include "mb_task_eit_events.h"

#include "common/mb_assert.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "hal/mb_system.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_events.h"
#include "mb_main.h"

using namespace std::chrono_literals;

namespace mb {

constexpr auto EIT_TIMEOUT = 1min;

Task_EIT_Events::EIT_Programs_Database Task_EIT_Events::s_eit_programs;

Task_EIT_Events::Task_EIT_Events()
{
    mb_assert(s_task_eit_events == nullptr);
    s_task_eit_events = this;
}

Task_EIT_Events::~Task_EIT_Events()
{
    mb_assert(s_task_eit_events == this);
    s_task_eit_events = nullptr;
}

void Task_EIT_Events::update_eit()
{
    if(g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed))
    {
        return;
    }

    using namespace std::chrono;
    m_last_eit_update = decltype(m_last_eit_update)::clock::now();
    // Cleanup database - remove all past events
    UTC_MJD now(System::get_system_time());

    for(auto sit = s_eit_programs.begin(); sit != s_eit_programs.end();)
    {
        auto events = &sit->second.events;

        for(auto eit = events->begin(); eit != events->end();)
        {
            if(eit->start_time + eit->duration < now)
            {
                DEBUG_MSG(EPG, DEBUG, "EIT " TERM_RED_BOLD "Del:" TERM_RESET " " << sit->first << "\t" << eit->start_time.time_to_str() << " - " << (eit->start_time + eit->duration).time_to_str()
                            << "\t" << eit->short_event_descriptor << "\n");
                eit = events->erase(eit);
            }
            else
            {
                eit++;
            }
        }

        if(events->empty())
        {
            sit = s_eit_programs.erase(sit);
        }
        else
        {
            sit++;
        }
    }
}

void Task_EIT_Events::handle_event_application_ready()
{
    m_do_update_eit = true;
    post_event_eit_update();
}

void Task_EIT_Events::handle_event_lineup_ready(const Event_Lineup_Ready &_event)
{
    if(_event.origin == Lineup_Origin::LO_SATELLITE)
    {
        s_eit_programs.clear();
    }
}

void Task_EIT_Events::handle_event_transponder_locked(const Event_Tuner_Lock &_event)
{
    if(_event.success and m_do_update_eit)
    {
        post_event_eit_update();
    }
}

void Task_EIT_Events::handle_event_system_memory_is_low()
{
    DEBUG_MSG(EPG, WARN, "Clear EIT database - system memory is low!\n");
    s_eit_programs.clear();
}

void Task_EIT_Events::process()
{
    if(decltype(m_last_eit_update)::clock::now() - m_last_eit_update > EIT_TIMEOUT
        and not g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed))
    {
        update_eit();
    }
}

std::tuple<std::optional<Task_EIT_Events::EIT_Event>, std::optional<Task_EIT_Events::EIT_Event >> Task_EIT_Events::get_event_for_service(const Service *_srv)
{
    if(_srv)
    {
        const auto srv_prog = Task_EIT_Events::s_eit_programs.find(_srv->service_id());

        if(srv_prog != Task_EIT_Events::s_eit_programs.end())
        {
            UTC_MJD now(System::get_system_time());
            auto endit = srv_prog->second.events.end();

            for(auto it = srv_prog->second.events.begin(); it != endit; it++)
            {
                auto end = it->start_time + it->duration;

                if(it->start_time < now and now < end)
                {
                    std::optional<Task_EIT_Events::EIT_Event> current, next;
                    current = *it;
                    it++;

                    if(it != endit)
                    {
                        next = *it;
                    }

                    return {current, next};
                }
            }
        }
        else
        {
            DEBUG_MSG(EPG, DEBUG, "EIT Events - Service not found: " << _srv->service_id() << "\n");
        }
    }

    return {};
}

Task_EIT_Events::EIT_Events Task_EIT_Events::get_all_events_for_service(const Service *_srv)
{
    if(_srv)
    {
        const auto srv_prog = Task_EIT_Events::s_eit_programs.find(_srv->service_id());

        if(srv_prog != Task_EIT_Events::s_eit_programs.end())
        {
            return srv_prog->second.events;
        }
        else
        {
            DEBUG_MSG(EPG, DEBUG, "EIT Events - Service not found: " << _srv->service_id() << "\n");
        }
    }

    return {};
}

std::map<std::string, std::string> Task_EIT_Events::get_event_for_service_str(const Service *_srv)
{
    auto [current_event, next_event] = get_event_for_service(_srv);
    std::map<std::string, std::string> content_map;

    if(current_event)
    {
        content_map["current_short_event_descriptor"] = current_event->short_event_descriptor;
        content_map["current_extended_event_descriptor"] = current_event->extended_event_descriptor;
        content_map["current_parental_rating"] = std::to_string(current_event->parental_rating);
        content_map["current_start_time"] = current_event->start_time.time_to_str();
        content_map["current_duration"] = std::to_string(current_event->duration.count());
        auto current_end_time = current_event->start_time + current_event->duration;
        content_map["current_end_time"] = current_end_time.time_to_str();
    }
    else 
    {
        content_map["current_short_event_descriptor"] = "";
        content_map["current_extended_event_descriptor"] = "";
        content_map["current_parental_rating"] = "";
        content_map["current_start_time"] = "";
        content_map["current_duration"] = "0";
        content_map["current_end_time"] = "";
    }

    if(next_event)
    {
        content_map["next_short_event_descriptor"] = next_event->short_event_descriptor;
        content_map["next_extended_event_descriptor"] = next_event->extended_event_descriptor;
        content_map["next_parental_rating"] = std::to_string(next_event->parental_rating);
        content_map["next_start_time"] = next_event->start_time.time_to_str();
        content_map["next_duration"] = std::to_string(next_event->duration.count());
        auto next_end_time = next_event->start_time + next_event->duration;
        content_map["next_end_time"] = next_end_time.time_to_str();
    }
    else
    {
        content_map["next_short_event_descriptor"] = "";
        content_map["next_extended_event_descriptor"] = "";
        content_map["next_parental_rating"] = "";
        content_map["next_start_time"] = "";
        content_map["next_duration"] = "0";
        content_map["next_end_time"] = "";
    }

    return content_map;
}



} // namespace mb
