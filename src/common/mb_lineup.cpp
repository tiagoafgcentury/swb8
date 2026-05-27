#include "mb_lineup.h"
#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "dvb/mb_dvb_globals.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_player.h"
#include "mb_zone_id.h"

#include <ostream>
#include <sstream>

#include <unordered_map>

#ifndef NDEBUG
#include <chrono>

using namespace std::chrono_literals;
#endif

namespace {

using namespace mb;

std::unordered_map<NID_t, std::unordered_map<TS_ID_t, std::unordered_map<NID_t, Service_Category >>> s_service_category_map;

void init_service_category_map() __attribute__((constructor));
void init_service_category_map()
{
#if __has_include("mb_service_categories.h")
#include "mb_service_categories.h"
#endif

#ifndef MBGUI_HAS_DYNAMIC_SERVICE_MAP
    DEBUG_MSG(LINEUP, WARN, TERM_RED_BOLD "USING BACKUP SERVICE MAP\n" TERM_RESET);
#include "mb_backup_service_categories.h"
#else
    DEBUG_MSG(LINEUP, INFO, TERM_GREEN_BOLD "USING DYNAMIC SERVICE MAP\n" TERM_RESET);
#endif
}

std::unordered_map<Bouquet_ID_t, std::string> s_bouquet_names;

}

namespace mb {

Lineup s_current_lineup;
Service::AudioPid Service::s_empty_audio_pid{0, "", Audio_Codec::None};

#ifndef NDEBUG
std::timed_mutex s_current_lineup_mutex;
#else
std::mutex s_current_lineup_mutex;
#endif

bool Service::check_pids_are_valid() const
{
    if(m_pmt_pid == 0)
    {
        return false;
    }

    switch(to_basic_type(m_service_type))
    {
        case Basic_Service_Type::TV:
        {
            return m_video_pid != 0 and m_pcr_pid != 0;
        }

        case Basic_Service_Type::Radio:
        {
            return (not m_audio_pids.empty()) and (m_audio_pids[0].pid > 0);
        }

        case Basic_Service_Type::Other:
            return true;
    }

    mb_assert(false);
    return false;
}

void Service::reset_pmt_pids()
{
    m_video_pid = 0;
    m_pmt_pid = 0;
    m_epg_pid = 0;
    clear_audio_pids();
    m_subtitle_pid = 0;
}

void Lineup::filter_lineup()
{
    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();

    auto policy = static_cast<Satellite_Operator>(sat_config.network_policies);

    Satellite_Operator oper =
        (policy == Satellite_Operator::Claro ||
        policy == Satellite_Operator::Sky   ||
        policy == Satellite_Operator::Generic)
            ? policy
            : Satellite_Operator::Generic;
    DEBUG_MSG(LINEUP, INFO, "Channel List size before filtering: " << services.size() << "\n");

    const auto my_zone_id = Zone_ID::get_zone_id(oper);
    bool hide_service = false;
    bool lineup_changed = false;

    bool has_sky = std::any_of(services.begin(), services.end(), [](const auto & s) { return s.satellite_id() == 2; });
    bool has_claro = std::any_of(services.begin(), services.end(), [](const auto & s) { return s.satellite_id() == 1; });
    bool both_sky_claro = has_sky && has_claro;

    // Filter Regionalização + Non-TV services
    for(auto it = services.begin(); it != services.end();)
    {
        it->map_service_type(get_transponder(it->transponder_id()));

        switch(it->regionalizacao())
        {
            case Regionalizacao::Regionalizado:
            {
                if (both_sky_claro && it->satellite_id() == 1)
                {
                    goto HIDE_SERVICE;
                }

                if(it->zones().contains(my_zone_id))
                {
                    goto SERVICE_OK;
                }

                goto HIDE_SERVICE;
            }

            case Regionalizacao::RegionalizadoNacional:
            {
                if (both_sky_claro && it->satellite_id() == 1)
                {
                    if(it->bouquet_id() != 0)
                    {
                        goto HIDE_SERVICE;
                    }
                    else
                    {
                        goto SERVICE_OK;
                    }
                }

                for(const auto &srv : services)
                {
                   if(srv.satellite_id() == it->satellite_id() and
                      srv.bouquet_id() == it->bouquet_id() and
                      srv.regionalizacao() == Regionalizacao::Regionalizado and
                      srv.zones().contains(my_zone_id))
                    {
                        goto HIDE_SERVICE;
                    }
                }

                goto SERVICE_OK;
            }

            case Regionalizacao::Undefined:
            case Regionalizacao::NaoRegionalizado:
            default:
            {
                switch(to_basic_type(it->service_type()))
                {
                    case Basic_Service_Type::TV:
                    case Basic_Service_Type::Radio:
                        goto SERVICE_OK;

                    case Basic_Service_Type::Other:
                        goto REMOVE_SERVICE;
                }
            }

HIDE_SERVICE:
            hide_service = true;
REMOVE_SERVICE:
            lineup_changed = true;
            {
                DEBUG_MSG(LINEUP, INFO, (hide_service ? "Hide" : "Remove") << " service: " << it->name() << " " << it->service_id() << " " << it->viewer_channel() << " SatID: " << it->satellite_id() << "\n");
                auto distance = std::distance(services.begin(), it);

                if(hide_service)
                {
                    hidden_services.push_back(std::move(*it));
                    hide_service = false;
                }

                services.erase(it);
                it = services.begin();

                if(distance > 0)
                {
                    std::advance(it, distance - 1);
                }

                continue;
            }
SERVICE_OK:

            if(it->name().empty())
            {
                it->set_name(it->bouquet_name());
            }

            // Try to set viewer channel if none set
            if(it->viewer_channel() == 0)
            {
                auto name = it->name().data();
                auto end = name + it->name().size();

                for(; name < end; name++)
                {
                    if(isdigit(*name))
                    {
                        it->set_viewer_channel(atol(name));
                        goto FOUND_VIEWER_CHANNEL;
                    }
                }
                DEBUG_MSG(LINEUP, WARN, "Viewer channel not set for service: '" << it->name() << "' (SatID: " << it->satellite_id() << ")\n");
                // FALLBACK: Assign a unique-ish high number to avoid 0
                it->set_viewer_channel(10000 + (it->service_id() % 10000));
            }

FOUND_VIEWER_CHANNEL:
            DEBUG_MSG(LINEUP, DEBUG, "Keep service: " << it->name() << " " << it->service_id() << " " << it->viewer_channel() << " SatID: " << it->satellite_id() << "\n");
            it++;
            continue;
        }
    }

    std::sort(transponders.begin(), transponders.end(),
              [](const auto & rhs, const auto & lhs)
    {
        return rhs.transponder_id < lhs.transponder_id;
    }
    );

    std::sort(services.begin(), services.end(),
        [](const auto & lhs, const auto & rhs)
    {
        if (lhs.viewer_channel() != rhs.viewer_channel())
            return lhs.viewer_channel() < rhs.viewer_channel();

        return lhs.service_id() < rhs.service_id();
    });

    // Deduplicate by viewer_channel within the same satellite
    for (auto it = services.begin(); it != services.end(); )
    {
        auto next = std::next(it);
        if (next != services.end() && 
            it->viewer_channel() == next->viewer_channel() && 
            it->satellite_id() == next->satellite_id())
        {
            DEBUG_MSG(LINEUP, WARN, "Duplicate viewer channel " << it->viewer_channel() 
                      << " found for SatID " << it->satellite_id() 
                      << ". Removing SID " << it->service_id() << " in favor of SID " << next->service_id() << "\n");
            it = services.erase(it);
        }
        else
        {
            it++;
        }
    }


    DEBUG_MSG(LINEUP, INFO, "Channel List: Zone " << dec << (int)my_zone_id << "\n"
            << left
            << setfill(' ') << setw(5) << "Num"
            << setfill(' ') << setw(10) << "Frequency"
            << setfill(' ') << setw(8) << "DVBMode"
            << setfill(' ') << setw(10) << "Type"
            << setfill(' ') << setw(6) << "SID"
            << setfill(' ') << setw(6) << "VChan"
            << setfill(' ') << setw(6) << "Reg"
            << setfill(' ') << setw(8) << "Bouquet"
            << setfill(' ') << setw(4) << "Aud"
            << setfill(' ') << setw(30) << "Name");
#if defined(MBGUI_SAT_MONITOR) or not defined(NDEBUG)
    auto max_time_to_start { std::chrono::milliseconds::zero() };
    auto sum_time_to_start { std::chrono::milliseconds::zero() };
    auto count_items = 0;
#endif
    auto numberOfChannels = 0;

    for(auto &srv : services)
    {
#ifndef NDEBUG
        auto tp = get_transponder(srv.transponder_id());
        DEBUG_MSG_NL(LINEUP, INFO, "\n"
            << setfill(' ') << setw(5) << srv.viewer_channel()
            << setfill(' ') << setw(10) << srv.transponder_id()
            << setfill(' ') << setw(8) << (tp ? (tp->dvb_mode == DVB_Mode::DVBS ? "DVBS" : "DVBS2") : "??")
            << setfill(' ') << setw(3) << hex << (int)srv.service_type() << setw(7) << ( to_basic_type(srv.service_type()) == Basic_Service_Type::TV ? "TV" : "Radio") << dec
            << setfill(' ') << setw(6) << (int)srv.service_id()
            << setfill(' ') << setw(6) << (int)srv.viewer_channel()
            << setfill(' ') << setw(6) << (int)srv.regionalizacao()
            << setfill(' ') << setw(8) << srv.bouquet_id()
            << setfill(' ') << setw(4) << srv.audio_pids().size()
            << setfill(' ') << srv.name());
        numberOfChannels += 1;
#endif // NDEBUG
    }
    std::cout << std::endl;

    DEBUG_MSG_NL(LINEUP, INFO, "CHANNELS FOUND: " << numberOfChannels << "\n");
    DEBUG_MSG_NL(LINEUP, INFO, "\n"
              << setfill(' ') << setw(63) << "Max Time to Play:"
              << setfill(' ') << setw(15) << max_time_to_start.count() << "ms\n"
              << setfill(' ') << setw(63) << "Average Time to Play:"
              << setfill(' ') << setw(15) << round((static_cast<double>(sum_time_to_start.count()) / static_cast<double>(count_items)) * 100.0) / 100.0 << "ms\n"
              << endl);
    DEBUG_MSG_NL(LINEUP, DEBUG, "\nOTAs:\n");

    for(auto &tp : transponders)
    {
        DEBUG_MSG_NL(LINEUP, DEBUG, "\tTP: " << dec << tp.transponder_id << " - No OTAs\n");
    }

    if(lineup_changed)
    {
        Task::post_event_lineup_changed();
    }

    if(numberOfChannels > 0)
    {
        m_lineup_table_empty = false;
    }
}

Lineup *Lineup_Mutex_Ref::operator->()
{
    return get();
}

Lineup *Lineup_Mutex_Ref::get()
{
    return &s_current_lineup;
}

void Lineup_Mutex_Ref::reset()
{
    if(m_is_locked)
    {
        s_current_lineup_mutex.unlock();
        m_is_locked = false;
    }
}

Lineup_Mutex_Ref::Lineup_Mutex_Ref()
{
#ifndef NDEBUG
    mb_assert(s_current_lineup_mutex.try_lock_for(1s));
#else
    s_current_lineup_mutex.lock();
#endif
    m_is_locked = true;
}

Lineup_Mutex_Ref::~Lineup_Mutex_Ref()
{
    reset();
}

Lineup_Mutex_Ref Lineup_Mutex_Ref::get_current_lineup()
{
    return Lineup_Mutex_Ref{};
};

bool Lineup_Mutex_Ref::is_empty()
{
    return s_current_lineup.services.empty();
};

Service::AudioPid::AudioPid(PID_t _pid, const char *_lang, Audio_Codec _codec):
    pid(_pid),
    codec(_codec)
{
    strncpy(lang, _lang, LANG_SIZE);
}

std::string_view Service::bouquet_name() const
{
    return s_bouquet_names[m_bouquet_id];
}

void Service::set_bouquet_name(std::string_view _bouquet_name)
{
    set_bouquet_name(m_bouquet_id, _bouquet_name);
}

void Service::set_bouquet_name(Bouquet_ID_t _bouquet_id, std::string_view _bouquet_name)
{
    if(not _bouquet_name.empty())
    {
        if(_bouquet_id)
        {
            auto it = s_bouquet_names.find(_bouquet_id);

            if(it == s_bouquet_names.end())
            {
                if(not _bouquet_name.empty())
                {
                    s_bouquet_names[_bouquet_id] = _bouquet_name;
                }
            }
            else
            {
                if(strcmp(_bouquet_name.data(), it->second.data()) != 0)
                {
                    DEBUG_MSG(LINEUP, WARN, "Bouquet name mismatch: '" << _bouquet_name << "' != '" << it->second << "'\n");
                }
            }

            return;
        }

        DEBUG_MSG(LINEUP, ERROR, "Invalid bouquet_id: " << _bouquet_id << "\n");
    }
}

const Service::AudioPid &Service::next_audio()
{
    auto sz = m_audio_pids.size();
    m_current_audio = (m_current_audio + sz + 1) % sz;
    return m_audio_pids[m_current_audio];
}

void Service::map_service_type(const Transponder *_tp)
{
    if(category() != Service_Category::Undefined)
    {
        return;
    }

    if(_tp)
    {
        auto nid = _tp->network_id;
        const auto sat = s_service_category_map.find(nid);
        if (sat != s_service_category_map.end())
        {
            const auto cat = sat->second.find(_tp->transport_stream_id);
            if (cat != sat->second.end())
            {
                auto it = cat->second.find(service_id());

                if(it != cat->second.end())
                {
                    m_category = it->second;
                    return;
                }
            }
        }
    }

    m_category = Service_Category::Entretenimento;
}

Lineup::Lineup()
{
    load_satellite_list();
}

void Lineup::clear()
{
    services.clear();
    transponders.clear();
    m_lineup_table_empty = true;
}

void Lineup::clear_satellite(uint16_t _satellite_id)
{
    services.erase(std::remove_if(services.begin(), services.end(), 
        [_satellite_id](const auto& s) { return s.satellite_id() == _satellite_id; }), services.end());
    
    hidden_services.erase(std::remove_if(hidden_services.begin(), hidden_services.end(), 
        [_satellite_id](const auto& s) { return s.satellite_id() == _satellite_id; }), hidden_services.end());

    transponders.erase(std::remove_if(transponders.begin(), transponders.end(), 
        [_satellite_id](const auto& t) { return t.satellite_id == _satellite_id; }), transponders.end());
}

Audio_Codec audio_codec_from_stream_type(uint8_t _stream_type)
{
    switch(_stream_type)
    {
        case STREAM_TYPE_11172_AUDIO: // MPEG 1
            return Audio_Codec::MP1;

        case STREAM_TYPE_13818_AUDIO: // MPEG 2
            return Audio_Codec::MP2;

        case STREAM_TYPE_13818_7_AUDIO: // AAC
        case STREAM_TYPE_14496_3_AUDIO: // AAC
            return Audio_Codec::AAC;

        case STREAM_TYPE_AC3_AUDIO:
            return Audio_Codec::AC3;
    }

    DEBUG_MSG(LINEUP, WARN, "Unknown audio codec type: 0x" << hex << (int)_stream_type << endl);
    return Audio_Codec::UNDEFINED;
}

void Lineup::set_channel_list_type(Channel_List_Type _list_type)
{
    m_channel_list_type = _list_type;
    DEBUG_MSG(LINEUP, DEBUG, "Set channel list type: " << to_str(_list_type) << "\n");
}

Channel_List_Type Lineup::get_channel_list_type()
{
    return m_channel_list_type;
}

Service *Lineup::get_next_channel(int _direction, const std::vector<Channel_Info> &_srvs)
{
    if(_srvs.size())
    {
        // Carrega service id do serviço atual
        auto current_service = get_current_service();
        // Busca dentro da lista recebida o índice do serviço atual
        for(uint index = 0; index < _srvs.size(); index++)
        {
            if(current_service and _srvs[index].service_id == current_service->service_id() and _srvs[index].transponder_id == current_service->transponder_id())
            {
                // Calcula o próximo índice
                index = (index + _srvs.size() + _direction) % _srvs.size();
                // Busca dentro da lista de serviços o serviço correspondente ao índice calculado
                auto service_id = _srvs[index].service_id;

                for(auto it = 0u; it < services.size() ; it ++)
                {
                    if(services[it].service_id() == service_id)
                    {
                        return &services[it];
                    }
                }
            }
        }
    }

    return nullptr;
}

void Lineup::toggle_tv_radio_channel_list_type()
{
    Service *current = get_current_service();
    if (!current)
    {
        DEBUG_MSG(LINEUP, DEBUG, "toggle_tv_radio: no current service\n");
        return;
    }

    const bool currently_tv = is_tv_service(current->service_type());
    auto list_type = get_channel_list_type();
    Service *target = nullptr;

    if (currently_tv)
    {
        m_last_tv_service = m_current_service;
        const auto &last_radio_svc = std::get<0>(m_last_radio_service);
        if (last_radio_svc.service_id() != 0)
        {
            target = get_service(last_radio_svc.service_id(), last_radio_svc.transponder_id());
            if (target)
            {
                list_type = std::get<1>(m_last_radio_service);
                DEBUG_MSG(LINEUP, DEBUG, "Switching to last radio service: " << dec << target->service_id() << " - " << target->name() << "\n");
            }
        }

        if (!target)
        {
            DEBUG_MSG(LINEUP, DEBUG, "No last radio service, switching to first radio service\n");
            list_type = (list_type == Channel_List_Type::ALL_TV_CHANNELS)
                ? Channel_List_Type::ALL_RADIO_CHANNELS
                : Channel_List_Type::MY_RADIO_CHANNELS;
            auto list = get_list(list_type);

            if (list.empty())
            {
                list_type = Channel_List_Type::ALL_RADIO_CHANNELS;
                list = get_list(list_type);
            }

            if (!list.empty())
            {
                target = get_service(list[0].service_id, list[0].transponder_id);
            }
        }
    }

    else
    {
        m_last_radio_service = m_current_service;
        const auto &last_tv_svc = std::get<0>(m_last_tv_service);
        if (last_tv_svc.service_id() != 0)
        {
            target = get_service(last_tv_svc.service_id(), last_tv_svc.transponder_id());
            if (target)
            {
                list_type = std::get<1>(m_last_tv_service);
                DEBUG_MSG(LINEUP, DEBUG, "Switching to last tv service: " << dec << target->service_id() << " - " << target->name() << "\n");
            }
        }

        if (!target)
        {
            DEBUG_MSG(LINEUP, DEBUG, "No last tv service, switching to first tv service\n");
            list_type = (list_type == Channel_List_Type::ALL_RADIO_CHANNELS)
                ? Channel_List_Type::ALL_TV_CHANNELS
                : Channel_List_Type::MY_TV_CHANNELS;
            auto list = get_list(list_type);
            if (list.empty())
            {
                list_type = Channel_List_Type::ALL_TV_CHANNELS;
                list = get_list(list_type);
            }
            if (!list.empty())
            {
                target = get_service(list[0].service_id, list[0].transponder_id);
            }
        }
    }

    if (!target)
    {
        DEBUG_MSG(LINEUP, DEBUG, "No services found for channel list type: " << to_str(list_type) << "\n");
        return;
    }
    set_channel_list_type(list_type);
    DEBUG_MSG(LINEUP, DEBUG, "Channel list type: " << to_str(list_type) << "\n");
    channel_change(target);
    DEBUG_MSG(LINEUP, DEBUG, "Changing to service: " << target->name() << "\n");

}

int Lineup::get_current_channel_index()
{
    auto type = get_channel_list_type();
    const auto &list = get_list(type);

    if (list.empty())
        return -1;

    auto current_service = get_current_service();

    for (size_t i = 0; i < list.size(); ++i)
    {
        const Channel_Info &ch = list[i];

        if (ch.service_id == current_service->service_id() and ch.transponder_id == current_service->transponder_id())
            return static_cast<int>(i);
    }

    return -1;
}

int Lineup::get_next_channel_index_by_type(int base, int direction)
{
    auto type = get_channel_list_type();
    const auto &list = get_list(type);

    if (list.empty())
        return -1;

    direction = (direction >= 0) ? 1 : -1;

    if (base < 0 || base >= static_cast<int>(list.size()))
        base = get_current_channel_index();

    if (base < 0)
        base = 0;

    return (base + direction + list.size()) % list.size();
}

Service *Lineup::get_service(Service_ID_t _service_id, Transponder_Id _transponder_id)
{
    for(auto &srv : services)
    {
        if(srv.service_id() == _service_id && srv.transponder_id() == _transponder_id)
        {
            return &srv;
        }
    }

    return nullptr;
}

void Lineup::preview_channel(int index)
{
    auto type = get_channel_list_type();
    DEBUG_MSG(LINEUP, DEBUG, "Preview channel index: " << index << " for channel list type: " << to_str(type) << "\n");
    const auto &list = get_list(type);
    if (index < 0 || index >= static_cast<int>(list.size()))
        return;
    const Channel_Info &ch = list[index];

    Service *srv  = get_service( ch.service_id, ch.transponder_id);
    if (!srv)
        return;
    DEBUG_MSG(LINEUP, DEBUG, dec << srv->viewer_channel() << " - " << srv->name() << "\n");

    auto p_state = Task::s_task_player->get_player_state();
    if(p_state != Player::State::Idle)
    {
        Task::s_task_player->stop();
    }
    Task::post_event_channel_preview(POST_CALLER srv);
}

void Lineup::channel_change_by_index(int index)
{
    auto type = get_channel_list_type();
    DEBUG_MSG(LINEUP, DEBUG, "Preview channel index: " << index << " for channel list type: " << to_str(type) << "\n");
    const auto &list = get_list(type);
    if (index < 0 || index >= static_cast<int>(list.size()))
        return;
    const Channel_Info &ch = list[index];

    Service *srv  = get_service( ch.service_id, ch.transponder_id);
    if (!srv)
        return;

    DEBUG_MSG(LINEUP, DEBUG, dec << srv->viewer_channel() << " - " << srv->name() << "\n");
    channel_change(srv);
}

std::vector<Lineup::Channel_Info> Lineup::get_list(Channel_List_Type _type)
{
    switch(_type)
    {
        case Channel_List_Type::ALL_TV_CHANNELS:
        {
            return get_tv_list();
        }

        case Channel_List_Type::ALL_RADIO_CHANNELS:
        {
            return get_radio_list();
        }

        case Channel_List_Type::MY_RADIO_CHANNELS:
        {
            return get_favorite_radio_list();
        }

        case Channel_List_Type::MY_TV_CHANNELS:
        default:
        {
            return get_favorite_tv_list();
        }
    }
}

Lineup::Channel_Info::Channel_Info(const Service &_srv):
    channel_name(_srv.name()),
    viewer_channel(_srv.viewer_channel()),
    favorite(_srv.is_favorite()),
    transponder_id(_srv.transponder_id()),
    service_id(_srv.service_id()),
    order_in_full(_srv.get_order_in_full()),
    order_in_favorite(_srv.get_order_in_favorite())
{
};

std::vector<Lineup::Channel_Info> Lineup::get_tv_list()
{
    std::vector<Lineup::Channel_Info> list;

    for(const auto &srv : services)
    {
        if(is_tv_service(srv.service_type()))
        {
            list.emplace_back(srv);
        }
    }

    std::sort(list.begin(), list.end(),
              [](const auto & rhs, const auto & lhs)
    {
        return (rhs.order_in_full < lhs.order_in_full);
    }
             );
    return list;
}

std::vector<Lineup::Channel_Info> Lineup::get_radio_list()
{
    std::vector<Lineup::Channel_Info> list;

    for(const auto &srv : services)
    {
        if(is_radio_service(srv.service_type()))
        {
            list.emplace_back(srv);
        }
    }

    std::sort(list.begin(), list.end(),
              [](const auto & rhs, const auto & lhs)
    {
        return (rhs.order_in_full < lhs.order_in_full);
    }
             );
    return list;
}

std::vector<Lineup::Channel_Info> Lineup::get_favorite_tv_list()
{
    std::vector<Lineup::Channel_Info> list;

    for(const auto &srv : services)
    {
        if(srv.is_favorite() and is_tv_service(srv.service_type()))
        {
            list.emplace_back(srv);
        }
    }

    std::sort(list.begin(), list.end(),
              [](const auto & rhs, const auto & lhs)
    {
        return (rhs.order_in_favorite < lhs.order_in_favorite);
    }
             );
    return list;
}

std::vector<Lineup::Channel_Info> Lineup::get_favorite_radio_list()
{
    std::vector<Lineup::Channel_Info> list;

    for(const auto &srv : services)
    {
        if(srv.is_favorite() and is_radio_service(srv.service_type()))
        {
            list.emplace_back(srv);
        }
    }

    std::sort(list.begin(), list.end(),
              [](const auto & rhs, const auto & lhs)
    {
        return (rhs.order_in_favorite < lhs.order_in_favorite);
    }
             );
    return list;
}

void Lineup::set_favorite_by_viewer_channel(Viewer_Channel_t _viewer_channel, bool _is_favorite)
{
    for(auto &srv : services)
    {
        if(srv.viewer_channel() == _viewer_channel)
        {
            srv.set_is_favorite(_is_favorite);
            Task::post_event_service_favorite_changed(&srv);
            return;
        }
    }
}

void Lineup::set_current_service(Service *_srv)
{
    DEBUG_MSG(LINEUP, DEBUG, "Tipo de lista atual: " << to_str(m_channel_list_type) << "\n");
    DEBUG_MSG(LINEUP, DEBUG, "Novo serviço: " << _srv->name() << " (" << _srv->service_id() << ")\n");
    DEBUG_MSG(LINEUP, DEBUG, "Serviço atual: " << std::get<0>(m_current_service).name() << " (" << std::get<0>(m_current_service).service_id() << ")\n");
    DEBUG_MSG(LINEUP, DEBUG, "Serviço anterior: " << std::get<0>(m_last_service).name() << " (" << std::get<0>(m_last_service).service_id() << ")\n");
    // Busca serviço atual para salvar como anterior
    m_last_service = m_current_service;
    // Check if the service is in the preferred channel list type first
    auto service_in_list = [this, _srv](Channel_List_Type type) -> bool
    {
        auto services = get_list(type);
        return std::any_of(services.begin(), services.end(), [_srv](const auto & srv)
        {
            return srv.service_id == _srv->service_id() and srv.transponder_id == _srv->transponder_id();
        });
    };

    if(service_in_list(m_channel_list_type))
    {
        DEBUG_MSG(LINEUP, DEBUG, "Channel list type: " << to_str(m_channel_list_type) << "\n");
        m_current_service = std::make_tuple(*_srv, m_channel_list_type);
    }
    else
    {
        // If not found, iterate through the other channel types
        std::vector<Channel_List_Type> service_type_vector =
        {
            Channel_List_Type::MY_TV_CHANNELS,
            Channel_List_Type::ALL_TV_CHANNELS,
            Channel_List_Type::MY_RADIO_CHANNELS,
            Channel_List_Type::ALL_RADIO_CHANNELS
        };

        for(const auto &type : service_type_vector)
        {
            if(type == m_channel_list_type)
            {
                continue; // Skip the type that was already checked
            }

            if(service_in_list(type))
            {
                DEBUG_MSG(LINEUP, DEBUG, "Channel list type: " << to_str(type) << "\n");
                set_channel_list_type(type);
                m_current_service = std::make_tuple(*_srv, type);
                break;
            }
        }
    }

    DEBUG_MSG(LINEUP, DEBUG, "Serviço atual: " << std::get<0>(m_current_service).name() << " (" << std::get<0>(m_current_service).service_id() << ")\n");
    DEBUG_MSG(LINEUP, DEBUG, "Serviço anterior: " << std::get<0>(m_last_service).name() << " (" << std::get<0>(m_last_service).service_id() << ")\n");
}

void Lineup::swap_last_service()
{
    if(std::get<0>(m_last_service).service_id() == 0)
        return;

    auto service_id = std::get<0>(m_last_service).service_id();
    auto transponder_id = std::get<0>(m_last_service).transponder_id();
    auto service = get_service(service_id, transponder_id);
    channel_change(service);
}

Service *Lineup::get_first_service()
{
    return services.empty() ? nullptr : &services[0];
}

Service *Lineup::get_last_service()
{
    auto ptr = services.empty() ? nullptr : &services[services.size() - 1];
    return ptr;
}

Service *Lineup::get_current_service()
{
    // Se não há serviços, retorna nullptr
    if (services.empty())
        return nullptr;

    auto service_id = std::get<0>(m_current_service).service_id();
    auto transponder_id = std::get<0>(m_current_service).transponder_id();

    auto service = get_service(service_id, transponder_id);

    // Se encontrou, retorna normalmente
    if (service)
        return service;

    // pegar primeiro serviço válido
    for (auto &srv : services)
    {
        if (srv.service_id() != 0)
        {
            // Atualiza estado interno para evitar repetir fallback
            m_current_service = std::make_tuple(srv, m_channel_list_type);
            return &srv;
        }
    }

    return nullptr;
}


Service *Lineup::get_next_service()
{
    // Verifica se o serviço atual é um serviço de TV ou rádio
    Service *srv = nullptr;
    srv = get_next_channel(1, get_list(m_channel_list_type));

    // Verifica se o serviço encontrado é nulo
    if(srv == nullptr)
    {
        DEBUG_MSG(LINEUP, WARN, "Service not found\n");
        return nullptr;
    }

    return srv;
}

Service *Lineup::get_previous_service()
{
    // Verifica se o serviço atual é um serviço de TV ou rádio
    Service *srv = nullptr;
    // Busca serviço anterior
    srv = get_next_channel(-1, get_list(m_channel_list_type));

    // Verifica se o serviço encontrado é nulo
    if(srv == nullptr)
    {
        DEBUG_MSG(LINEUP, WARN, "Service not found\n");
        return nullptr;
    }

    return srv;
}

Service *Lineup::get_service_by_viewer_channel(Viewer_Channel_t _viewer_channel)
{
    for(auto &service : services)
    {
        if(service.viewer_channel() == _viewer_channel)
        {
            return &service;
        }
    }

    return nullptr;
}

bool Lineup::channel_change_by_viewer_channel(Viewer_Channel_t _viewer_channel)
{
    auto service = get_service_by_viewer_channel(_viewer_channel);
    if(service)
    {
        DEBUG_MSG(LINEUP, DEBUG, dec << _viewer_channel << " - " << service->name() << "\n");
        channel_change(service);
        return true;
    }

    return false;
}

void Lineup::channel_change(Service *_srv)
{
    if(_srv)
    {
        DEBUG_MSG(LINEUP, DEBUG, "Changing channel to: " << dec << _srv->viewer_channel() << " - " << _srv->name() << "\n");
        Task::post_event_channel_change(POST_CALLER _srv);
    }
}

void Lineup::delete_current_service()
{
    Service current_service = *get_current_service();
    for(auto it = services.begin(); it != services.end();)
    {
        if(it->service_id() == current_service.service_id() and it->transponder_id() == current_service.transponder_id())
        {
            DEBUG_MSG(LINEUP, DEBUG, "Deleting service: " << dec << it->viewer_channel() << " - " << it->name() << "\n");
            services.erase(it);
            set_current_service(&services[0]);
            Task::post_event_lineup_changed();
            return;
        }
        else
        {
            it++;
        }
    }
}

void Lineup::clone_current_service()
{
    auto current_service = get_current_service();
    if(current_service)
    {
        Service new_service = *current_service;
        new_service.set_viewer_channel(get_last_available_viewer_channel());
        services.push_back(new_service);
        Task::post_event_lineup_changed();
    }
}

Viewer_Channel_t Lineup::get_last_available_viewer_channel()
{
    return get_max_viewer_channel() + 1;
}

Viewer_Channel_t Lineup::get_max_viewer_channel()
{
    Viewer_Channel_t last_channel = 0;
    for (const auto &service : services)
    {
        if (service.viewer_channel() > last_channel)
        {
            last_channel = service.viewer_channel();
        }
    }
    return last_channel;
}

Order_In_Full_t Lineup::get_max_order_in_full()
{
    Order_In_Full_t max_order = 0;
    for (const auto &service : services)
    {
        if (service.get_order_in_full() > max_order)
        {
            max_order = service.get_order_in_full();
        }
    }
    return max_order;
}

Order_In_Favorite_t Lineup::get_max_order_in_favorite()
{
    Order_In_Favorite_t max_order = 0;
    for (const auto &service : services)
    {
        if (service.get_order_in_favorite() > max_order)
        {
            max_order = service.get_order_in_favorite();
        }
    }
    return max_order;
}

std::map<std::string, std::string> Lineup::get_current_lock_info()
{
    DEBUG_MSG(LINEUP, DEBUG, "Current lineup lock info:\n");
    std::map<std::string, std::string> lock_info = {};
    auto service = get_current_service();
    if(!service) return {};

    // Carrega variáveis do serviço atual
    lock_info["service_name"] = service->name();
    lock_info["viewer_channel"] = std::to_string(service->viewer_channel());
    lock_info["service_id"] = std::to_string(service->service_id());
    lock_info["service_type"] = to_str(service->service_type());
    lock_info["is_favorite"] = service->is_favorite() ? "1" : "0";
    lock_info["video_codec"] = to_str(service->video_codec()).data();
    lock_info["video_pid"] = std::to_string(service->video_pid());
    lock_info["pcr_pid"] = std::to_string(service->pcr_pid());
    if (!service->audio_pids().empty()) {
        lock_info["audio_pid"] = std::to_string(service->audio_pids()[service->current_audio_index()].pid);
        lock_info["audio_codec"] = to_str(service->audio_pids()[service->current_audio_index()].codec).data();
        lock_info["audio_lang"] = service->audio_pids()[service->current_audio_index()].lang;
        lock_info["total_audios"] = std::to_string(service->audio_pids().size());
    } else {
        lock_info["audio_pid"] = "";
        lock_info["audio_codec"] = "";
        lock_info["audio_lang"] = "";
        lock_info["total_audios"] = "0";
    }

    // Get current transponder
    auto tp = get_transponder(service->transponder_id());
    if(tp)
    {
        lock_info["frequency"] = std::to_string(tp->transponder_id.frequency()/1000) + "MHz";
        lock_info["polarity"] = tp->transponder_id.polarity() == Polarity::Horizontal ? "H" : "V";
        lock_info["polarity_str"] = tp->transponder_id.polarity() == Polarity::Horizontal ? "Horizontal" : "Vertical";
        lock_info["symbol_rate"] = std::to_string(tp->symbol_rate) + "Kbps";

        auto satellite_id = tp->transponder_id.satellite_id();
        auto satellites = Config::get_config()->get_satellite_list();
        if(not satellites.empty())
        {
            Satellite satellite;
            for(const auto &sat : satellites)
            {
                if(sat.id == satellite_id)
                {
                    satellite = sat;
                    break;
                }
            }

            lock_info["satellite_name"] = satellite.name;
            lock_info["band"] = to_str(satellite.band);
            lock_info["lnbf_type"] = to_str(satellite.type);
        }
        else 
        {
            DEBUG_MSG(LINEUP, ERROR, "Satellite list is empty, cannot load satellite info for lock screen\n");
        }

        for(const auto &[key, value] : lock_info)
        {
            DEBUG_MSG(LINEUP, DEBUG, "\t" << key << ": " << value << "\n");
        }
    }
    return lock_info;
}

void Lineup::load_satellite_list()
{
    DEBUG_MSG(LINEUP, WARN, TERM_YELLOW_BOLD "Select_Satellite::load_satellite_list()\n" TERM_RESET);
    auto callback = [this](const std::vector<Satellite> &sats)
    {
        if(sats.size() > 0)
        {
            this->m_satellites.clear();
            this->m_satellites = sats;
        }
    };    
    Task::post_event_satellite_list_load(std::bind(callback, std::placeholders::_1));
}

std::string Lineup::get_sattelite_name(Satellite_ID_t satellite_id)
{
    Satellite satellite = {};
    auto satellites = Config::get_config()->get_satellite_list();
    if(not satellites.empty())
    {
        for(const auto &sat : satellites)
        {
            if(sat.id == satellite_id)
            {
                satellite = sat;
                break;
            }
        }
    }
    else 
    {
        DEBUG_MSG(LINEUP, ERROR, "Satellite list is empty, cannot load satellite info for lock screen\n");
    }

    return satellite.name;
}

} // namespace mb
