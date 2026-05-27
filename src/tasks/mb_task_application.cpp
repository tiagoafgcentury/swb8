#include "mb_task_application.h"
#include "mb_main.h"
#include "mb_task_osd.h"
#include "mb_task_player.h"
#include "mb_task_database.h"
#include "mb_task_tuner.h"
#include "mb_demux_lineup.h"
#include "mb_zone_id.h"

#include "common/mb_globals.h"
#include "common/mb_state_file.h"

#include "hal/mb_display.h"
#include "hal/mb_hdmi.h"
#include "hal/mb_sound.h"
#include "hal/mb_system.h"
#include "hal/mb_watchdog.h"

#include "tasks/mb_task_eit_events.h"

#include "ui/lvgl/mb_osd_translate.h"

#include "mb_events.h"
#include "fw_env.h"

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>

#include <chrono>
#include <thread>

#if __has_include(<cJSON.h>)
#include <cJSON.h>
#else
#include <cjson/cJSON.h>
#endif

using namespace std::chrono_literals;

namespace {

template<typename T>
void check_system_time(const T &_file)
{
    using namespace mb;

    if(static_cast<Clock_Type>(_file.clock_status) != Clock_Type::Manual)
    {
        goto SET_DEFAULT_SYSTEM_TIME;
    }
    else
    {
        std::ifstream uptime("/proc/uptime");

        if(!uptime.is_open())
        {
            DEBUG_MSG(TASK, ERROR, "Unable to open uptime\n");
            return;
        }

        std::string line;

        while(std::getline(uptime, line))
        {
            auto time = strtoull(line.data(), nullptr, 10);

            if(time < 20)  // First boot, so uptime should be small, < 20s
            {
                goto SET_DEFAULT_SYSTEM_TIME;
            }
        }
    }

    return;
SET_DEFAULT_SYSTEM_TIME:
    System::set_system_time(UTC_MJD{2025, 01, 01, 00, 00, 00});
}

}

namespace mb {

Task_Application::Task_Application():
    m_hdmi(std::make_unique<HDMI>()),
    m_display(std::make_unique<Display>()),
    m_sound(std::make_unique<Sound>())
{
    mb_assert(s_task_application == nullptr);
    s_task_application = this;
    set_focus();
    State_File::App_State_File file;
    post_event_system_display_settings_load();
    read_total_system_memory();
    check_system_time(file);
}

Task_Application::~Task_Application()
{
    mb_assert(s_task_application == this);
    s_task_application = nullptr;
}

void Task_Application::change_state(State _state)
{
    switch(_state)
    {
        case ST_IDLE:
        {
            if(!m_started)  // Application is now started
            {
                m_started = true;
                post_event_application_ready();
            }

            break;
        }

        case ST_STARTING:
        case ST_WAITING_FOR_LINEUP:
        case ST_PROCESS_LINEUP_LOAD:
        case ST_WAITING_FOR_APP_STATE:
        case ST_PROCESS_AUTOMATIC_SEARCH:
        case ST_EASY_INSTALL:
        case ST_PRODUCTION_FINAL_TEST:
        case ST_WAINTING_FOR_FACTORY_RESET_DONE:
        case ST_STAND_BY_MODE:
#ifdef MBGUI_FORCED_UPDATE
        case ST_FORCED_UPDATE:
#endif
            break;
    }

    m_state = _state;
}

bool Task_Application::is_in_stand_by()
{
    if(g_production_final_test)
    {
        State_File::App_State_File file;
        DEBUG_MSG(TASK, DEBUG, "START STAND_BY_PRODUCTION_MODE_FLAG " << file.stand_by_in_production_mode << "\n");

        if(file.stand_by_in_production_mode)
        {
            change_state(ST_STAND_BY_MODE);
        }

        System::check_standby_mode(g_production_final_test);
        return file.stand_by_in_production_mode;
    }

    State_File::App_State_File file;

    if(file.stand_by)
    {
        change_state(ST_STAND_BY_MODE);
    }

    return file.stand_by;
}

bool Task_Application::verify_production_update_status()
{
    if(mb::g_production_final_test)
    {
        load_lineup_production_data();
        change_state(ST_PRODUCTION_FINAL_TEST);
        return true;
    }
    else
    {
#if MBGUI_FORCED_UPDATE
        {

            auto upg_status = fw_getenv_safe("upg_status");

            if((upg_status and strcmp(upg_status, "1") == 0) or
                    (upg_status and strcmp(upg_status, "3") == 0))
            {
                s_task_osd->software_updated_finish();
            }
            else
            {
                s_task_osd->software_updated_forced();
            }

            change_state(ST_FORCED_UPDATE);
            return true;
        }
#else  // MBGUI_FORCED_UPDATE
        {
            auto sw_updt_status_flags = fw_getenv_safe("sw_updt_status_flag");
            auto sw_updt_status_flag = fw_getenv_safe("sw_updt_status_flag");

            if((sw_updt_status_flags and strcmp(sw_updt_status_flags, "1") == 0) or
                    (sw_updt_status_flag and strcmp(sw_updt_status_flag, "3") == 0))
            {
                s_task_osd->software_updated_finish();
                change_state(ST_EASY_INSTALL);
                return true;
            }

            auto frequency   = fw_getenv_safe("frequency");

            if (!frequency || atoi(frequency) == 0)
            {
                fw_env_open();
                fw_env_write("frequency", "1520");
                fw_env_write("symbol_rate", "29892");
                fw_env_write("pol", "v");
                fw_env_write("22khz", "on");
                fw_env_write("PID", "6041");
                fw_env_close();
            }

            auto ota_found  = fw_getenv_safe("ota_found");
            if(ota_found and strcmp(ota_found, "1") == 0)
            {
                post_event_ota_found();
                fw_env_open();
                fw_env_write("ota_found", "0");
                fw_env_close();
            }
        }
#endif // MBGUI_FORCED_UPDATE
    }

    return false;
}

void Task_Application::process()
{
    if (m_zapping_active)
    {
        auto now = std::chrono::steady_clock::now();
        if (now - m_last_zap > std::chrono::milliseconds(800))
        {
            auto lineup = Lineup_Mutex_Ref::get_current_lineup();
            lineup->channel_change_by_index(m_zap_channel_index);
            m_zapping_active = false;
            m_zap_channel_index = -1;
        }
    }

    if(m_need_update_datetime_to_save_terms_of_use and m_time_is_set)
    {
        m_need_update_datetime_to_save_terms_of_use = false;
        save_datetime_terms_of_use();
    }

    switch(state())
    {
        case ST_STARTING:
            check_is_production_final_test();

            if(!is_in_stand_by())
            {
                if(verify_production_update_status() == false)
                {
                    if(Task::s_task_database->is_empty())
                    {
                        s_task_osd->show_instala_facil();
                        change_state(ST_EASY_INSTALL);
                    }
                    else
                    {
                        post_event_application_state_load();
                        change_state(ST_PROCESS_LINEUP_LOAD);
                    }
                }
            }
            else
            {
                // This will trigger EMM filtering which we *NEED*
                change_state(ST_WAITING_FOR_APP_STATE);
                post_event_application_state_load();
            }

            break;

        case ST_IDLE:
            if(m_application_state_save_time.time_since_epoch() > 0s and
                    decltype(m_application_state_save_time)::clock::now() > m_application_state_save_time)
            {
                application_state_save();
                m_application_state_save_time = {};
            }

            if(decltype(m_next_memory_check)::clock::now() > m_next_memory_check)
            {
                check_free_system_memory();
            }

            break;

        case ST_PROCESS_LINEUP_LOAD:
            post_event_lineup_load();
            change_state(ST_WAITING_FOR_LINEUP);
            break;

        case ST_EASY_INSTALL:
        case ST_WAITING_FOR_LINEUP:
        case ST_WAITING_FOR_APP_STATE:
        case ST_PROCESS_AUTOMATIC_SEARCH:
        case ST_PRODUCTION_FINAL_TEST:
        case ST_STAND_BY_MODE:
#ifdef MBGUI_FORCED_UPDATE
        case ST_FORCED_UPDATE:
#endif
            break;

        case ST_WAINTING_FOR_FACTORY_RESET_DONE:
        {
            auto now = decltype(m_wait_for_reset)::clock::now();

            if(now - m_wait_for_reset > m_application_state_save_timeout)
            {
                m_wait_for_reset = now;
                post_event_system_factory_reset_done();
            }

            break;
        }
    }
}

void Task_Application::application_state_save()
{
    if(g_production_final_test == false)
    {
        Event_Save_Application_State state;
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        auto current_service = current_lineup->get_current_service();

        if(current_service)
        {
            state.current_channel = current_service->viewer_channel();
        }

        state.mute = m_sound->mute_state();
        state.volume = m_sound->get_volume();
        state.stand_by = m_state == ST_STAND_BY_MODE;
        state.channel_list_type = current_lineup->get_channel_list_type();
        state.current_satellite_id = Config::get_config()->get_current_satellite();
        post_event_application_state_save(std::move(state));
    }
}

void Task_Application::handle_event_lineup_satellite_found()
{
    //if(state() != ST_PROCESS_AUTOMATIC_SEARCH)
    {
        change_state(ST_PROCESS_LINEUP_LOAD);
    }
}

void Task_Application::handle_event_process_automatic_search()
{
    change_state(ST_PROCESS_AUTOMATIC_SEARCH);
}

void Task_Application::handle_event_starting_application()
{
    change_state(ST_STARTING);
}

void Task_Application::handle_event_clock_set_time(const Event_Time &_event)
{
    m_time_is_set = System::set_system_time(UTC_MJD{_event.year, _event.month, _event.day,
                       _event.hour, _event.minute, _event.second});
    post_event_clock_time_set(m_time_is_set);
}

void Task_Application::handle_event_save_terms_of_use()
{
    m_need_update_datetime_to_save_terms_of_use = true;
}

void Task_Application::handle_event_lineup_ready(const Event_Lineup_Ready &_event)
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    current_lineup->filter_lineup();

    //if(_event.origin == Lineup_Origin::LO_SATELLITE)
    {
        for(auto &s : current_lineup->services)
        {
            uint32_t idx = s.viewer_channel();
            idx = (idx << 16u) + static_cast<uint32_t>(s.service_id());
            s.set_order_in_full(idx);
            s.set_order_in_favorite(idx);
        }

        if(_event.origin == Lineup_Origin::LO_SATELLITE)
        {
            post_event_lineup_save();
        }
    }

    auto old_state = state();
    change_state(ST_IDLE);

    if(old_state != ST_EASY_INSTALL)
    {
        if(not current_lineup->services.empty())
        {
            auto first_viewer_channel = current_lineup->services[0].viewer_channel();
            current_lineup.reset();

            if(m_channel_after_process.has_value())
            {
                change_channel_to(m_channel_after_process.value());
                m_channel_after_process = {};
            }
            else
            {
                DEBUG_MSG(TASK, INFO, TERM_GREEN_BOLD "Changing to first channel: " << first_viewer_channel << "\n" TERM_RESET);
                if (_event.restart)
                {
                    DEBUG_MSG(TASK, INFO, TERM_GREEN_BOLD "Restarting scan after lock, changing to first channel: " << first_viewer_channel << "\n" TERM_RESET);
                    change_channel_to(first_viewer_channel);
                }
                else
                {
                    DEBUG_MSG(TASK, INFO, TERM_GREEN_BOLD "Not restarting scan after lock, not changing channel to: " << first_viewer_channel << "\n" TERM_RESET);
                }
            }
        }
    }
}

bool Task_Application::change_channel_to(Viewer_Channel_t _viewer_channel)
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    return current_lineup->channel_change_by_viewer_channel(_viewer_channel);
}

void Task_Application::handle_event_application_state_loaded(const Event_Save_Application_State &_event)
{
    //if(state() != ST_PROCESS_AUTOMATIC_SEARCH)
    {
        if(state() == ST_WAITING_FOR_APP_STATE)
        {
            change_state(ST_IDLE);
        }

        auto default_volume = 50;
        bool tp_locked = false;

        if(_event.got_state)
        {
            auto snd = m_sound.get();
            snd->set_volume(_event.volume);

            if(snd->mute_state() != _event.mute)
            {
                snd->mute_toggle();
            }

            post_event_sound_changed({ .vol = snd->get_volume(), .muted = snd->mute_state() });

            if(_event.stand_by)
            {
                if(_event.current_channel)
                {
                    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

                    for(const auto &s : current_lineup->services)
                    {
                        if(s.viewer_channel() == _event.current_channel)
                        {
                            auto tp = current_lineup->get_transponder(s.transponder_id());
                            post_event_cas_start_emm_filtering(tp);
                            post_event_transponder_lock(POST_CALLER tp); // Explicitly lock tuner
                            tp_locked = true; // Mark as locked
                            goto CURRENT_TP_FOUND;
                        }
                    }

                    if(not current_lineup->transponders.empty())
                    {
                        auto tp = &current_lineup->transponders.at(0);
                        post_event_cas_start_emm_filtering(tp);
                    }

CURRENT_TP_FOUND:
                    {};
                }
            }
            else if(_event.current_channel)
            {
                m_channel_after_process = _event.current_channel;
                return;
            }
        }
        else
        {
            // Set default volume
            if(m_sound->mute_state())
            {
                m_sound->mute_toggle();
            }

            m_sound->set_volume(default_volume);
        }

        // Load default service
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

        if(not current_lineup->services.empty())
        {
            if(_event.stand_by)
            {
                // Only use default first transponder if we haven't locked yet
                if(not current_lineup->transponders.empty() and !tp_locked)
                {
                    // Start EMM Filtering
                    post_event_transponder_lock(POST_CALLER &current_lineup->transponders[0]);
                }
            }
            else
            {
                m_channel_after_process = 0;
            }

            current_lineup->set_channel_list_type(_event.channel_list_type);
        }

        DEBUG_MSG(TASK, WARN, "No default service\n");
    }
}

static bool s_first_date_check = true;

void Task_Application::check_system_clock()
{
    auto system_time = System::get_system_time();

    if(system_time.year() < 2025 or system_time.year() > 2038 or s_first_date_check)
    {
        State_File::App_State_File file;

        if(file.clock_status == static_cast<uint8_t>(Clock_Type::Auto))
        {
            s_first_date_check = false;
            post_event_clock_need_update();
        }
    }
}

void Task_Application::handle_event_transponder_locked(const Event_Tuner_Lock &_event)
{
    if(_event.success)
    {
        check_system_clock();
    }
}

void Task_Application::handle_event_system_factory_reset()
{
    change_state(ST_WAINTING_FOR_FACTORY_RESET_DONE);
    g_factory_reset_enabled = true;
    m_wait_for_reset = decltype(m_wait_for_reset)::clock::now();
}

void Task_Application::handle_event_system_factory_reset_done()
{
    g_mbgui_do_factory_reset.store(true, std::memory_order_release);
    system_exit();
}

void Task_Application::handle_event_display_clear()
{
    m_display->clear();
}

void Task_Application::handle_event_zone_id_changed(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id)
{
    auto config = Config::get_config();
    const auto oper = config->selected_satellite_config().network_id;
    // Sky: Task_OSD::handle_event_zone_id_changed already starts OSD_Channel_List_Update
    // (post_event_lineup_build). Reloading the DB here stops the player and filters stale
    // services against the new zone before the scan finishes — redundant with Instala Fácil.
    if (oper != Network_Id_Sky)
    {
        post_event_lineup_load();
        change_state(ST_WAITING_FOR_LINEUP);
    }
    char buffer[128];
    size_t sz;
    if (oper == Network_Id_Sky)
    {
        sz = snprintf(buffer, sizeof(buffer), tr(__Alterado_segment_bouquet_id).data(), _from_zone_id, _to_zone_id);
    }
    else if (oper == Network_Id_Claro)
    {
        sz = snprintf(buffer, sizeof(buffer), tr(__Alterado_zone_id).data(), _from_zone_id, _to_zone_id);
    }
    
    Event_Display_Message message;
    message.message = std::string(buffer, sz);
    if (oper == Network_Id_Sky)
    {
        message.timeout = std::chrono::milliseconds(-1);
    }
    else
    {
        message.timeout = 10s;
    }
    message.category = Message_Categories::Event_Popup;
    post_event_osd_display_message(std::move(message));
    // Save last activation date
    // Get the current time
    auto now = System::get_system_time().to_local_time();
    std::ofstream last_activation(MBGUI_LAST_ACTIVATION_DATE_FILE);
    using namespace std;
    last_activation << setfill('0') << setw(2) << (int)now.day() << '/'
                    << setfill('0') << setw(2) << (int)now.month() << '/'
                    << setfill('0') << setw(4) << now.year();
}

void Task_Application::handle_event_channel_changed(Service *)
{
    application_state_reset_timer();
}

void Task_Application::system_exit()
{
    g_mbgui_reboot_after_exit.store(false, std::memory_order_release);
    g_mbgui_keep_running.store(false, std::memory_order_release);
    g_mbgui_restart_on_exit.store(true, std::memory_order_release);
}

void Task_Application::handle_event_toggle_power()
{
    auto stand_by = System::stand_by(g_production_final_test);
    State_File::App_State_File file;

    if(g_production_final_test)
    {
        file.stand_by_in_production_mode = stand_by;
        DEBUG_MSG(TASK, DEBUG, "SAVE STAND_BY_PRODUCTION_MODE_FLAG " << file.stand_by_in_production_mode << "\n");
    }
    else
    {
        file.stand_by = stand_by;
        DEBUG_MSG(TASK, DEBUG, "SAVE STAND_BY_FLAG " << file.stand_by << "\n");
    }

    file.write();
    system_exit();
}

static void timer_check_event_cb(lv_timer_t *timer)
{

    //Task_Application *thiz = static_cast<Task_Application *>(lv_timer_get_user_data(timer));
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto _srv = current_lineup->get_current_service();

    if(_srv)
    {
        auto events = Task_EIT_Events::get_all_events_for_service(_srv);

        if (not events.empty())
        {
            auto system_time = std::chrono::system_clock::from_time_t(System::get_system_time().to_unix_epoch());
            for (const auto &event : events)
            {
                auto start_time = std::chrono::system_clock::from_time_t(event.start_time.to_unix_epoch());
                auto end_time = std::chrono::system_clock::from_time_t(event.start_time.to_unix_epoch() + (event.duration.count()));

                if ((system_time > start_time) and (system_time < end_time) and event.ca_rs_descriptor_bseid.has_value())
                {
                    DEBUG_MSG(TASK, WARN, "\nPROGRAMA NÂO AUTORIZADO!\n");
                    auto ca_rs_des_bseid = event.ca_rs_descriptor_bseid.value();
                    if (ca_rs_des_bseid > 0)
                    {
                        DEBUG_MSG(DEMUX, DEBUG, "\nCA_RS_Descriptor:" <<
                        "\n BSE_ID:" << hex << setw(4) << ca_rs_des_bseid << "\n\n");
                        Service *_next_srv = &std::get<0>(current_lineup->m_current_service);
                        if (_next_srv == nullptr)
                        {
                            Task::post_event_channel_change(POST_CALLER & current_lineup->services[0]);
                            DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << current_lineup->services[0].viewer_channel() << " - " << current_lineup->services[0].name() << "\n");
                        }
                        else
                        {
                            Task::post_event_channel_change(POST_CALLER _next_srv);
                            DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << _next_srv->viewer_channel() << " - " << _next_srv->name() << "\n");
                        }

                        char buffer[128];
                        auto sz = snprintf(buffer, sizeof(buffer), tr(__Programa_nao_autorizado_para_sua_regiao).data());
                        Event_Display_Message message;
                        message.message = std::string(buffer, sz);
                        message.timeout = 5s;
                        message.category = Message_Categories::Event_Popup;
                        Task::post_event_osd_display_message(std::move(message));
                        lv_timer_pause(timer);
                        lv_timer_del(timer);
                    }
                }
            }
        }
    }
}

void Task_Application::handle_event_program_access_denied()
{
    m_tmr_evt = lv_timer_create(timer_check_event_cb, 2000, this);
    lv_timer_set_repeat_count(m_tmr_evt, 5);
}

bool Task_Application::handle_event_remote_control(const Event_Remote_Control &_event)
{
    using namespace std::placeholders;

    switch(_event.key)
    {
        case Remote_Control_Key::KEY_MENU:
            //if (state() == ST_IDLE)
        {
#ifndef MBGUI_FORCED_UPDATE
            post_event_osd_mainmenu_show();
#endif
            return true;
        }
        break;

        case Remote_Control_Key::KEY_PLUS:
        {
            if(state() == ST_IDLE)
            {
                typedef std::chrono::system_clock::time_point Time_Point;
                post_event_osd_menu_plus(false, false, Time_Point::max());
                return true;
            }
            break;
        }

        case Remote_Control_Key::KEY_CC:
        {
            if(state() == ST_IDLE)
            {
                post_event_osd_closed_caption();
                return true;
            }
            break;
        }

        case Remote_Control_Key::KEY_SLEEP:
        {
            if(state() == ST_IDLE)
            {
                typedef std::chrono::system_clock::time_point Time_Point;
                post_event_osd_menu_plus(false, true, Time_Point::max());
                return true;
            }

            break;
        }

        case Remote_Control_Key::KEY_TVRADIO:
        {
            if(state() == ST_IDLE)
            {
                auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                current_lineup->toggle_tv_radio_channel_list_type();
                return true;
            }

            break;
        }

        case Remote_Control_Key::KEY_CHDOWN:
        case Remote_Control_Key::KEY_CHUP:
        {
            switch(state())
            {
                case ST_IDLE:
                case ST_PRODUCTION_FINAL_TEST:
                {
                    int dir = (_event.key == Remote_Control_Key::KEY_CHUP) ? 1 : -1;
                    auto lineup = Lineup_Mutex_Ref::get_current_lineup();

                    if (!m_zapping_active)
                    {
                        m_zap_channel_index = lineup->get_current_channel_index();
                        m_zapping_active = true;
                    }

                    m_zap_channel_index = lineup->get_next_channel_index_by_type(m_zap_channel_index, dir);
                    lineup->preview_channel(m_zap_channel_index);
                    m_last_zap = std::chrono::steady_clock::now();
                    return true;
                }

                default:
                    break;
            }
            break;
        }

        case Remote_Control_Key::KEY_0:
        case Remote_Control_Key::KEY_1:
        case Remote_Control_Key::KEY_2:
        case Remote_Control_Key::KEY_3:
        case Remote_Control_Key::KEY_4:
        case Remote_Control_Key::KEY_5:
        case Remote_Control_Key::KEY_6:
        case Remote_Control_Key::KEY_7:
        case Remote_Control_Key::KEY_8:
        case Remote_Control_Key::KEY_9:
        {
            char digit = to_int(_event.key);
            s_task_osd->show_menu_digit(digit, std::bind(&Task_Application::change_channel_to, this, _1));
            return true;
        }
        
        case Remote_Control_Key::KEY_OK:
            if(state() == ST_IDLE)
            {
                post_event_osd_channel_list_show();
                return true;
            }

            return false;

        case Remote_Control_Key::KEY_VOLUP:
        {
            auto snd = m_sound.get();
            snd->increment_volume(m_sound_increment);
            post_event_sound_changed({ .vol = snd->get_volume(), .muted = snd->mute_state() });
            application_state_reset_timer();
            return true;
        }
        break;

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            auto snd = m_sound.get();
            m_sound->increment_volume(m_sound_increment * -1);
            post_event_sound_changed({ .vol = snd->get_volume(), .muted = snd->mute_state() });
            application_state_reset_timer();
            return true;
        }
        break;

        case Remote_Control_Key::KEY_MUTE:
            if(g_production_final_test)
            {
                post_event_osd_factory_reset();
            }
            else
            {
                auto snd = m_sound.get();
                snd->mute_toggle();
                post_event_sound_changed({ .vol = snd->get_volume(), .muted = snd->mute_state() });
                application_state_reset_timer();
            }

            return true;
            break;

        case Remote_Control_Key::KEY_LR:
        {
            if(state() == ST_IDLE)
            {
                post_event_osd_audio_lr();
                return true;
            }

            break;
        }

        case Remote_Control_Key::KEY_LAST:
        {
            auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
            current_lineup->swap_last_service();
            return true;
        }

        case Remote_Control_Key::KEY_INFO:
        {
            if(s_task_osd->has_menu_info())
            {
                s_task_osd->show_menu_detail();
            }
            else
            {
                s_task_osd->show_menu_info();
            }

            return true;
        }

        case Remote_Control_Key::KEY_VOLTAR:
        {
            if(s_task_osd->has_menu_info())
            {
                s_task_osd->close_menu_info();
            }

            return true;
        }

        case Remote_Control_Key::KEY_UPGRADE:
        {
            return true;
        }

        default:
            DEBUG_MSG(TASK, WARN, "Tecla não mapeada: 0x" << hex << setfill('0') << setw(8) << static_cast<unsigned int>(_event.key) << "\n");
    }

    return false;
}

bool Task_Application::parse_json(std::string content)
{
    bool ret = false;
    auto config = Config::get_config();
    config->set_satellite_config(Network_Id_Claro);
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    current_lineup->clear();
    cJSON *root = cJSON_Parse(content.c_str());

    if(root == nullptr)
    {
        DEBUG_MSG(TASK, ERROR, "Error parsing JSON\n");
        return false;
    }

    std::string title = cJSON_GetObjectItem(root, "title")->valuestring;
    DEBUG_MSG(TASK, DEBUG, "TITLE: " << title << "\n");
    uint8_t _volume = cJSON_GetObjectItem(root, "volume")->valueint;

    if(_volume > 100)
    {
        _volume =  100;
    }

    DEBUG_MSG(TASK, DEBUG, "Volume: " << _volume << "\n");
    m_sound->set_volume(_volume);
    cJSON *channels = cJSON_GetObjectItem(root, "channels");
    auto channel_count = cJSON_GetArraySize(channels);
    int i;

    for(i = 0; i < channel_count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(channels, i);
        auto frequency = cJSON_GetObjectItem(item, "frequency")->valueint;
        auto symbol_rate = cJSON_GetObjectItem(item, "symbol_rate")->valueint;
        auto polarity = static_cast<Polarity>(cJSON_GetObjectItem(item, "polarity")->valueint ? 0 : 1);
        auto band = static_cast<Band>(cJSON_GetObjectItem(item, "band_type")->valueint);
        auto lnbf_type = cJSON_GetObjectItem(item, "lnbf_type")->valueint;

        if(band == Band::Ku)
        {
            DEBUG_MSG(TASK, DEBUG, "Banda Ku\n");
            config->set_band(band);

            if(lnbf_type == 0)
            {
                config->set_lnbf_type(LNBF_Type::Universal);
                DEBUG_MSG(TASK, DEBUG, "LNBF_Type::Universal\n");
            }
            else
            {
                config->set_lnbf_type(LNBF_Type::Multi);
                DEBUG_MSG(TASK, DEBUG, "LNBF_Type::Multi\n");
            }
        }
        else //Banda C
        {
            config->set_band(band);

            if(lnbf_type == 0)
            {
                config->set_lnbf_type(LNBF_Type::Mono);
            }
            else
            {
                config->set_lnbf_type(LNBF_Type::Multi);
            }
        }

        //auto diseqc_type = cJSON_GetObjectItem(item, "diseqc_type")->valueint;
        //auto tuner_polarity = cJSON_GetObjectItem(item, "tuner_polarity")->valueint;
        Transponder tp(Transponder_Id{static_cast<Frequency_t>(frequency * 1000), polarity, 1}, symbol_rate, DVB_Mode::DVBS2, 0, 0, 0, true);
        current_lineup->transponders.emplace_back(std::move(tp));
        cJSON *streams = cJSON_GetObjectItem(item, "stream_params");
        int stream_count = cJSON_GetArraySize(streams);

        for(auto j = 0; j < stream_count; j++)
        {
            cJSON *sub_item = cJSON_GetArrayItem(streams, j);
            std::string channel_name = cJSON_GetObjectItem(sub_item, "name")->valuestring;
            auto service_id = cJSON_GetObjectItem(sub_item, "service_id")->valueint;
            Service s(tp.transponder_id, service_id);
            s.insert_zone(0);
            s.set_viewer_channel(i + 1);
            s.set_name(channel_name);
            s.set_regionalizacao(Regionalizacao::NaoRegionalizado);
            s.set_service_type(Service_Type::advanced_codec_hd_digital_television_service);
            current_lineup->services.emplace_back(std::move(s));
        }

        ret = true;
    }

    cJSON_Delete(root);
    return ret;
}

bool Task_Application::process_json_file(std::string file)
{
    std::ifstream ifs(file);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    DEBUG_MSG(TASK, DEBUG, "File content: \n" << content << endl);
    // Close file
    ifs.close();
    return parse_json(content);
}

void Task_Application::check_is_production_final_test()
{
    DEBUG_MSG(TASK, DEBUG, "USB_PATH: " << USB_PATH << "\n");
    std::error_code _;

    for(const auto &entry : std::filesystem::directory_iterator(std::string(USB_PATH), _))
    {
        if(std::filesystem::is_directory(entry.path(), _))
        {
            auto mount_point = entry.path().filename().string();
            std::string file = std::string(USB_PATH) + mount_point + "/" + std::string(CONFIG_FILE_FINAL_TEST);
            DEBUG_MSG(TASK, DEBUG, "PRODUCTION_FILE: " << file << "\n");

            if(std::filesystem::exists(file.c_str(), _))
            {
                DEBUG_MSG(TASK, DEBUG, "Config file " << CONFIG_FILE_FINAL_TEST << "found!\n");
                g_production_final_test = process_json_file(file);
                return;
            }
            else
            {
                DEBUG_MSG(TASK, ERROR, "Error: " << file << " NOT found!\n");
            }
        }
    }

    DEBUG_MSG(TASK, DEBUG, "Config file " << CONFIG_FILE_FINAL_TEST << " NOT found!\n");
}

void Task_Application::load_lineup_production_data()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    post_event_channel_change(POST_CALLER & current_lineup->services[0]);
    DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << current_lineup->services[0].viewer_channel() << " - " << current_lineup->services[0].name() << "\n");
    post_event_osd_production_info();
}


void Task_Application::check_updates()
{
}

size_t read_meminfo(std::string_view _field)
{
    std::ifstream meminfo("/proc/meminfo");

    if(!meminfo.is_open())
    {
        DEBUG_MSG(TASK, ERROR, "Unable to open /proc/meminfo\n");
        return 0;
    }

    std::string line;

    while(std::getline(meminfo, line))
    {
        auto pos = line.find(_field);

        if(pos != std::string::npos)
        {
            auto ptr = line.data() + _field.size() + 1;
            return strtoull(ptr, nullptr, 10);
        }
    }

    return 0;
}

void Task_Application::read_total_system_memory()
{
    m_system_memory_limit = read_meminfo("MemTotal:") * 0.05; // 5%
}

void Task_Application::check_free_system_memory()
{
    m_next_memory_check = decltype(m_next_memory_check)::clock::now() + std::chrono::seconds{3};
    auto cur = read_meminfo("MemFree:");

    if(cur <= m_system_memory_limit)
    {
        post_event_system_memory_is_low();
    }
}

void Task_Application::save_datetime_terms_of_use()
{
    // Faz a leitura da data e hora atual
    auto system_time = System::get_system_time();
    char buffer[18];
    auto string_length = snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d", system_time.day(), system_time.month(), system_time.year());
    DEBUG_MSG(OSD, DEBUG, "Data e hora atual: " << buffer << "\tArquivo: " << MBGUI_TERMS_CONDITIONS_DATE_FILE << "\n");

    write_file(MBGUI_TERMS_CONDITIONS_DATE_FILE, buffer, string_length);

}

} // namespace mb
