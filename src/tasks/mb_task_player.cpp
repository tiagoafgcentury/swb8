#include "common/mb_globals.h"
#include "mb_task_player.h"
#include "mb_task_demux.h"
#include "mb_task_tuner.h"
#include "mb_task_osd.h"
#include "mb_task_application.h"
#include "mb_events.h"
#include "hal/mb_display.h"
#include "hal/mb_system.h"
#include "mb_zone_id.h"


#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace mb {

std::string_view to_str(Task_Player::State _state)
{
    switch(_state)
    {
        case Task_Player::ST_IDLE:
            return "ST_IDLE";

        case Task_Player::ST_WAITING_FOR_LOCK:
            return "ST_WAITING_FOR_LOCK";

        case Task_Player::ST_WAITING_FOR_PMT:
            return "ST_WAITING_FOR_PMT";

        case Task_Player::ST_WAITING_FOR_PMT_UPDATE:
            return "ST_WAITING_FOR_PMT_UPDATE";

        case Task_Player::ST_WAITING_FOR_DESCRABLE_START:
            return "ST_WAITING_FOR_DESCRABLE_START";

        case Task_Player::ST_WAITING_FOR_DESCRABLE_STOP:
            return "ST_WAITING_FOR_DESCRABLE_STOP";
    }

    return "<UNDEFINED>";
}

Task_Player::Task_Player()
{
    mb_assert(s_task_player == nullptr);
    s_task_player = this;
}

Task_Player::~Task_Player()
{
    mb_assert(s_task_player == this);
    s_task_player = nullptr;
}

void Task_Player::change_state(FUNC_NAME State _new_state)
{
    DEBUG_MSG(PLAYER, DEBUG, "Player Change: " << _fun_name << " " << to_str(m_state) << " 🠦 " << to_str(_new_state) << "\n");
    m_state = _new_state;
}

void Task_Player::handle_event_lineup_changed()
{
    stop();
}

void Task_Player::handle_event_cas_request_descramble_stop_done()
{
    DEBUG_MSG(PLAYER, DEBUG, "ST_WAITING_FOR_DESCRABLE_STOP_DONE: " << to_str(state()) << "\n");

    switch(state())
    {
        case ST_WAITING_FOR_DESCRABLE_START:
        {
            change_state(FUNC_PARAM ST_IDLE);
            post_current_service_to_cas();
            break;
        }

        case ST_WAITING_FOR_DESCRABLE_STOP:
        {
            change_state(FUNC_PARAM ST_IDLE);
            if (m_change_srv)
            {
                Service* srv = m_change_srv;
                m_change_srv = nullptr;
                handle_event_channel_change(srv);
            }
            break;
        }

        default:
            break;
    }
}

void Task_Player::unlock_channel_change()
{
    m_channel_change_in_progress = false;

    if (m_pending_srv)
    {
        Service* srv = m_pending_srv;
        m_pending_srv = nullptr;
        DEBUG_MSG(PLAYER, ERROR, "Continuing channel change to: " << dec << srv->viewer_channel() << " - " << srv->name() << "\n");
        handle_event_channel_change(srv);
    }
}

void Task_Player::handle_event_channel_change(Service *_srv)
{
    if (!_srv)
    {
        DEBUG_MSG(PLAYER, ERROR, "No service to change channel\n");
        return;
    }

    Config::get_config()->select_satellite_by_id(_srv->satellite_id());
    DEBUG_MSG(PLAYER, INFO, "Channel change requested: " << dec << _srv->viewer_channel() << " - " << _srv->name() << "\n");

    Config::get_config()->select_satellite_by_id(_srv->satellite_id());
    DEBUG_MSG(PLAYER, INFO, "Channel change requested: " << dec << _srv->viewer_channel() << " - " << _srv->name() << "\n");

    State st = state(); 
    if (st != ST_IDLE)
    {
        DEBUG_MSG(PLAYER, INFO, "STATE: " << to_str(state()) << "\n");
        //return;
    }

    if (m_ts_state != TimeshiftState::Disabled)
    {
        pvr_timeshift_stop();
    }
    DEBUG_MSG(PLAYER, INFO, "CHANGE_CHANNEL: " << dec <<  _srv->viewer_channel() << " " << _srv->name() << " " << _srv->service_id() << " '" << to_str(_srv->regionalizacao()) << "'\n");

    // First, stop player if running
    if(m_current_srv or m_player)
    {
        stop();
    }

    // Don't check for the first 10s while still trying to change channel
    m_last_signal_check = decltype(m_last_signal_check)::clock::now() + 10s;

    // If we need to wait for CAS
    if(state() == ST_WAITING_FOR_DESCRABLE_STOP)
    {
        m_change_srv = _srv;
        return;
    }

    // Verify that we have a valid service
    {
        auto basic_service_type = to_basic_type(_srv->service_type());
        if(basic_service_type == Basic_Service_Type::Other)
        {
            DEBUG_MSG(PLAYER, WARN, "Invalid service type\n");
            reset_current_service();
            change_state(FUNC_PARAM ST_IDLE);
            change_channel_finish(false);
            return;
        }
    }
    // Initialize the current service
    {
        m_current_srv = _srv;
        m_current_srv_has_nagra_cas = m_current_srv->has_nagra_cas();
        DEBUG_MSG(PLAYER, DEBUG, "Current service updated, has_nagra_cas: " << (m_current_srv_has_nagra_cas ? "Yes" : "No") << "\n");
        if(not m_current_srv->check_pids_are_valid())
        {
            m_current_srv->reset_pmt_pids();
        }

        m_current_srv_is_updated = false;
        m_current_srv_pmt.reset();
        Lineup_Mutex_Ref::get_current_lineup()->set_current_service(_srv);
    }
    change_channel_finish(true);

    // Check Tuner
    auto [current_tp, is_locked] = Task_Tuner::get_current_transponder();
    auto change_tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(m_current_srv->transponder_id());

    if(not change_tp)  // If not valid TP
    {
        DEBUG_MSG(PLAYER, ERROR, "ABORT CHANNEL CHANGE - NULL TP\n");
        reset_current_service();
        change_state(FUNC_PARAM ST_IDLE);
        return;
    }

    if(current_tp != change_tp->transponder_id or !is_locked)
    {
        post_event_transponder_lock(POST_CALLER change_tp);
        change_state(FUNC_PARAM ST_WAITING_FOR_LOCK);
    }
    else
    {
        change_channel_start();
    }
}

void Task_Player::handle_event_transponder_lock(const Transponder *, bool /*_force*/)
{
    show_signal_failed(false);
}

void Task_Player::handle_event_transponder_locked(const Event_Tuner_Lock &_event)
{
    if(_event.success)
    {
        if(state() == ST_WAITING_FOR_LOCK)
        {
            change_channel_start();
        }
    }
    else
    {
        show_signal_failed(true);
    }
}

void Task_Player::change_channel_start()
{
    mb_assert(m_current_srv);

    // Check PMT data
    if((not m_current_srv_is_updated) and m_current_srv and m_current_srv->pmt_pid())
    {
        s_task_demux->pmt_table_require(m_current_srv->pmt_pid(), m_current_srv->service_id());
        s_task_demux->cat_table_require();
    }
    else
    {
        post_event_services_update();
    }

    change_state(FUNC_PARAM ST_WAITING_FOR_PMT);
}

void Task_Player::change_channel_start_player()
{
    post_event_osd_display_message({ .message = {}, .category = Message_Categories::Program_Access });
    m_player = std::make_unique<Player>();

    if(not m_current_srv)
    {
        DEBUG_MSG(PLAYER, ERROR, "New service disappeard before we finished.\n");
        change_state(FUNC_PARAM ST_IDLE);
        return;
    }

    m_player->open(*m_current_srv);
    m_player->start();
    m_player_started_at = decltype(m_player_started_at)::clock::now();
    DEBUG_MSG(PLAYER, INFO, "*** Play: " << dec << m_current_srv->transponder_id() << " - " << m_current_srv->name() << "\n");

    auto config = Config::get_config();
    const auto& sat_config = config->selected_satellite_config();

    if(m_current_srv_has_nagra_cas and (sat_config.network_policies != static_cast<uint32_t>(Network_Policies::Generic)))
    {
        post_current_service_to_cas();
        change_state(FUNC_PARAM ST_WAITING_FOR_DESCRABLE_START);
    }
    else
    {
        change_state(FUNC_PARAM ST_IDLE);
    }

    // Don't check for the first 3s
    m_last_signal_check = decltype(m_last_signal_check)::clock::now() + 3s;
    if (m_current_srv_will_be_recorded)
    {
        m_current_srv_will_be_recorded = false;
        //handle_event_pvr_start();
        post_event_osd_menu_plus(true, false, m_time_to_end);
        m_time_to_end = Time_Point::max();
    }
}

void Task_Player::change_channel_finish(bool _success)
{
    if (_success)
    {
        // Notify everyone we have a new current service
        post_event_channel_changed(POST_CALLER m_current_srv);

        // Wait at least this time to start signal check
        m_last_signal_check = decltype(m_last_signal_check)::clock::now() + 5s;
    }
    else
    {
        post_event_channel_changed(POST_CALLER nullptr);
        m_current_srv_will_be_recorded = false;
        m_time_to_end = Time_Point::max();
    }
}

void Task_Player::post_current_service_to_cas(bool _update_only)
{
    if(m_current_srv and not m_current_srv_pmt.empty())
    {
        const auto audio_pids = &m_current_srv->audio_pids();
        auto tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(m_current_srv->transponder_id());
        auto msg = Event_CAS_Request_Descramble
        {
            .original_network_id = tp->original_network_id,
            .transport_stream_id = tp->transport_stream_id,
            .pmt_section_data = std::move(m_current_srv_pmt),
            .video_pid = m_current_srv->video_pid(),
            .audio_pid = not audio_pids->empty() ? m_current_srv->current_audio().pid : PID_t{ 0 },
            .subtitle_pid = m_current_srv->subtitle_pid(),
        };

        if(_update_only)
        {
            post_event_cas_request_descramble_pmt_update(msg);
        }
        else
        {
            post_event_cas_request_descramble(msg);
        }
    }
}

void Task_Player::handle_event_service_pmt_section(PID_t _pid, Service_ID_t _service_id, DVB_Table_Section _pmt)
{
    auto pmt_version = (_pmt.data()[5] & 0b00111110) >> 1;

    switch(state())
    {
        case ST_IDLE:
        {
            if(m_current_srv and m_current_srv->service_id() == _service_id)
            {
                post_event_service_pmt_get_next_section(_pid, _service_id, pmt_version);

                auto config = Config::get_config();
                const auto& sat_config = config->selected_satellite_config();

                if(m_current_srv_has_nagra_cas and (sat_config.network_policies != static_cast<uint32_t>(Network_Policies::Generic)))
                {
                    bool update_only = true;
                    post_current_service_to_cas(update_only);
                }
            }

            break;
        }

        case ST_WAITING_FOR_PMT:
        case ST_WAITING_FOR_PMT_UPDATE:
        {
            if(m_current_srv and m_current_srv->service_id() == _service_id)
            {
                post_event_service_pmt_get_next_section(_pid, _service_id, pmt_version);
                m_current_srv_pmt = std::move(_pmt);
                change_channel_start_player();
                return;
            }

            break;
        }

        default:
            break;
    }
}

void Task_Player::handle_event_cas_request_descramble_done(bool _result)
{
    if(state() == ST_WAITING_FOR_DESCRABLE_START)
    {
        if(_result)
        {
            DEBUG_MSG(PLAYER, DEBUG, "Descrambling request OK - continue change\n");
        }
        else
        {
            DEBUG_MSG(PLAYER, ERROR, "Descrambling request Failed\n");
            stop();
        }

        change_state(FUNC_PARAM ST_IDLE);
    }
    else
    {
        DEBUG_MSG(PLAYER, DEBUG, "Ignore: handle_event_cas_request_descramble_done(" << (_result ? "T" : "F") << ") - " << to_str(state()));
    }
}

void Task_Player::handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &)
{
    DEBUG_MSG(PLAYER, INFO, "CAS RESTARTED - Re-sending state\n");

    if(m_current_srv)
    {
        post_current_service_to_cas();
    }

    if(m_ts_state != TimeshiftState::Disabled)
    {
        post_event_cas_pvr_timeshift_start(m_pvr_record_param);
        m_ts_state = TimeshiftState::Starting;
    }

    if(m_is_recording)
    {
        post_event_cas_pvr_record_start(m_pvr_record_param);
    }
}

void Task_Player::handle_event_player_restart()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    if(!current_lineup->services.empty())
    {
        Service *service = current_lineup->get_current_service();
        post_event_channel_change(POST_CALLER service);
        DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << service->viewer_channel() << " - " << service->name() << "\n");
    }
}

void Task_Player::handle_event_player_change_audio(PID_t _new_pid)
{
    if(m_player)
    {
        const auto &current_audio = m_current_srv->current_audio();

        if(current_audio.pid == _new_pid)
        {
            m_player->change_audio(current_audio);
            m_current_srv->set_current_audio_index(m_current_srv->current_audio_index());
            return;
        }
        else
        {
            for(const auto &audio : m_current_srv->audio_pids())
            {
                if(audio.pid == _new_pid)
                {
                    m_player->change_audio(audio);
                    m_current_srv->set_current_audio_index(&audio - &m_current_srv->audio_pids()[0]);
                    return;
                }
            }
        }
    }

    DEBUG_MSG(PLAYER, WARN, "Could not change audio to: " << _new_pid << "\n");
}

void Task_Player::handle_event_player_stop()
{
    m_change_srv = nullptr;
    stop();
    post_event_player_stopped();
}

static bool g_show_signal_message = {false};
static int s_count_to_show = 0;

void Task_Player::restore_signal_state()
{
    if(g_show_signal_message)
    {
        Task::s_task_osd->show_waiting_signal();
    }
}

void Task_Player::show_signal_failed(bool _is_failed)
{
    if(_is_failed)
    {
        Task::s_task_osd->show_waiting_signal();
        g_show_signal_message = true;
        s_count_to_show = 0;
    }
    else
    {
        Task::s_task_osd->hide_waiting_signal();
        g_show_signal_message = false;
        s_count_to_show = 0;
    }
}

void Task_Player::verify_player_and_tuner_status()
{
    const bool is_in_error = (m_player and m_player->state() == Player::State::Error) or (not m_player);

    if(is_in_error and (not g_show_signal_message) and (not m_current_srv_is_blocked_by_cas))
    {
        s_count_to_show += 1;

        if(s_count_to_show == 5)
        {
            show_signal_failed(true);
        }
    }
    else if(g_show_signal_message and not is_in_error)
    {
        show_signal_failed(false);
    }
}

void Task_Player::process()
{
    switch(state())
    {
        case ST_IDLE:
        {
            if(m_player and m_player->state() == Player::State::Error)
            {
                auto now = decltype(m_last_restart)::clock::now();

                if(now - m_last_restart > MIN_RESTART_TIME)
                {
                    m_last_restart = now;

                    if(m_current_srv)
                    {
                        m_current_srv_is_updated = false;
                        m_current_srv->set_pmt_pid(0);
                        change_state(FUNC_PARAM ST_WAITING_FOR_PMT_UPDATE);
                        post_event_services_update();
                    }
                }
            }

            [[fallthrough]];
        }

        case ST_WAITING_FOR_LOCK:
        case ST_WAITING_FOR_PMT:
        case ST_WAITING_FOR_PMT_UPDATE:
        case ST_WAITING_FOR_DESCRABLE_START:
        case ST_WAITING_FOR_DESCRABLE_STOP:
            break;
    }

    if(m_player)
    {
        auto player_state = m_player->state();

        if(player_state != m_last_seen_state)
        {
            if(player_state == Player::State::Started)
            {
                unlock_channel_change();
                post_event_player_started();
            }
            else if(m_last_seen_state == Player::State::Started)
            {
                post_event_player_stopped();
            }

            m_last_seen_state = player_state;
        }
        else if(player_state == Player::State::Starting)
        {
            auto now = decltype(m_player_started_at)::clock::now();

            if(now - m_player_started_at > START_PLAY_TIMEOUT)
            {
                DEBUG_MSG(PLAYER, WARN, "Player FAILED TO START!\n");
                m_player_started_at = now;
                post_event_player_restart();
            }
        }

        auto now = decltype(m_last_signal_check)::clock::now();

        if(now - m_last_signal_check > SIGNAL_CHECK_INTERVAL)
        {
            m_last_signal_check = now;
            verify_player_and_tuner_status();
        }
    }

    if(m_media_player)
    {
        m_media_player->process_seek_tick();
    }
};

void Task_Player::stop()
{
    auto config = Config::get_config();
    const auto& sat_config = config->selected_satellite_config();

    if(m_current_srv_has_nagra_cas and (sat_config.network_policies != static_cast<uint32_t>(Network_Policies::Generic)))
    {
        DEBUG_MSG(PLAYER, DEBUG, "Task_Player::stop - Sending request_descramble_stop (cached has_nagra_cas is true)\n");
        post_event_cas_request_descramble_stop();
        change_state(FUNC_PARAM ST_WAITING_FOR_DESCRABLE_STOP);
    }

    m_player.reset();
    reset_current_service();
    Display::clear();

    switch(state())
    {
        case ST_IDLE:
        case ST_WAITING_FOR_DESCRABLE_STOP:
            break;

        default:
            change_state(FUNC_PARAM ST_IDLE);
            break;
    }
}

void Task_Player::restart()
{
    if(m_current_srv)
    {
        m_last_restart = decltype(m_last_restart)::clock::now();
        // back this up, it might be reset
        auto srv = m_current_srv;
        auto pmt = m_current_srv_pmt;
        auto upd = m_current_srv_is_updated;
        stop();
        m_current_srv = srv;
        m_current_srv_pmt = std::move(pmt);
        m_current_srv_is_updated = upd;
        change_channel_start();
    }
    else
    {
        change_state(FUNC_PARAM ST_IDLE);
        reset_current_service();
    }
}

Player::State Task_Player::get_player_state() const
{
    return m_player ? m_player->state() : Player::State::Stopped;
}

Service *Task_Player::current_srv()
{
    return s_task_player->m_current_srv;
}

void Task_Player::reset_current_service()
{
    m_current_srv = nullptr;
    m_current_srv_has_nagra_cas = false;
    m_current_srv_pmt.reset();
    m_current_srv_is_updated = false;
    m_current_srv_is_blocked_by_cas = false;
}

void Task_Player::handle_event_osd_display_message(const Event_Display_Message &_message)
{
    switch(_message.category)
    {
        case Message_Categories::Program_Access:
        {
            m_current_srv_is_blocked_by_cas = false;
            break;
        }

        case Message_Categories::Program_Access_Denied:
        {
            DEBUG_MSG(PLAYER, WARN, "Program Access is DENIED by CAK\n");
            // Disable restart
            m_player_started_at = decltype(m_player_started_at)::max();
            m_current_srv_is_blocked_by_cas = true;
            post_event_program_access_denied();
            break;
        }

        default: // Ignore all others
            break;
    }
}

#ifdef MBGUI_PERIODIC_DUMP
void Task_Player::handle_event_debbug_dump_status()
{
    DEBUG_MSG(PLAYER, DEBUG,
        "Task player state: " << to_str(state()) << "\n"
        "Player state: " << (m_player ? to_str(m_player->state()) : std::string_view{}) << "\n"
        "Service: " << (m_current_srv ? m_current_srv->name() : std::string_view{}) << "\n"
        "Current service is blocked by cas: " << (m_current_srv_is_blocked_by_cas ? "Yes" : "No") << "\n"
    );

    if(m_player)
    {
        m_player->get_player_status();
    }
}

#endif // #ifdef MBGUI_PERIODIC_DUMP


void Task_Player::handle_event_start_recording(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    Service *service = nullptr;
    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

        if(not current_lineup->services.empty())
        {
            for(auto &srv : current_lineup->services)
            {
                if(srv.service_id() == _service_id)
                {
                    service = &srv;
                    goto FOUND_SERVICE;
                }
            }
        }
    }
    // Service not found??
    mb_assert(false);
    m_time_to_end = Time_Point::max();
    return;

FOUND_SERVICE:
    m_current_srv_will_be_recorded = true;
    m_time_to_end = _time_to_end;
    handle_event_channel_change(service);
}


void Task_Player::handle_event_media_player_start(std::string url, uint8_t _mode)
{
    if(m_player)
    {
        stop();
    }

    if(!m_media_player)
    {
        m_media_player = std::make_unique<Media_Player>();
    }

    if(m_media_player and (m_media_player->state() == Media_Player::State::Idle or
                           m_media_player->state() == Media_Player::State::Idle))
    {
        auto _p_mode = _mode == 0 ? Media_Player::Player_Mode::Audio : Media_Player::Player_Mode::Video;
        m_media_player->open(url, _p_mode);
        m_media_player->start();
    }
}

void Task_Player::handle_event_media_player_stop()
{
    if(m_media_player)
    {
        m_media_player->stop_seek();
        switch(m_media_player->state())
        {
            case Media_Player::State::Started:
            case Media_Player::State::Starting:
            case Media_Player::State::Paused:
            case Media_Player::State::Pausing:
            case Media_Player::State::Finish:
                m_media_player->stop();
                m_media_player->close();
                break;

            default:
                break;
        }
    }
}

void Task_Player::handle_event_media_player_pause()
{
    if(m_media_player)
    {
        m_media_player->stop_seek();
        switch(m_media_player->state())
        {
            case Media_Player::State::Started:
            case Media_Player::State::Starting:
                m_media_player->pause();
                break;

            default:
                break;
        }
    }
}

void Task_Player::handle_event_media_player_resume()
{
    if(m_media_player)
    {
        m_media_player->stop_seek();
        switch(m_media_player->state())
        {
            case Media_Player::State::Paused:
            case Media_Player::State::Pausing:
                m_media_player->resume();
                break;

            default:
                break;
        }
    }
}

void Task_Player::handle_event_media_player_forward(uint16_t _mp_speed, bool _is_video)
{
    if(m_media_player)
    {
        switch(m_media_player->state())
        {
            case Media_Player::State::Started:
            case Media_Player::State::Starting:
                if(_is_video)
                {
                    m_media_player->video_forward(static_cast<Media_Player::MP_Speed_Forward>(_mp_speed));
                }
                else
                {
                    m_media_player->audio_forward(static_cast<Media_Player::MP_Speed_Forward>(_mp_speed));
                }
                break;

            default:
                break;
        }
    }
}

void Task_Player::handle_event_media_player_rewind(uint16_t _mp_speed, bool _is_video)
{
    if(m_media_player)
    {
        switch(m_media_player->state())
        {
            case Media_Player::State::Started:
            case Media_Player::State::Starting:
                if(_is_video)
                {
                    m_media_player->video_rewind(static_cast<Media_Player::MP_Speed_Rewind>(_mp_speed));
                }
                else
                {
                    m_media_player->audio_rewind(static_cast<Media_Player::MP_Speed_Rewind>(_mp_speed));
                }
                break;

            default:
                break;
        }
    }
}

void Task_Player::handle_event_media_player_next(std::string url, uint8_t _mode)
{
    handle_event_media_player_stop();
    handle_event_media_player_start(url, _mode);
}

void Task_Player::handle_event_media_player_previous(std::string url, uint8_t _mode)
{
    handle_event_media_player_stop();
    handle_event_media_player_start(url, _mode);
}


unsigned int Task_Player::get_mp_curr_time()
{
    if(m_media_player)
    {
        return m_media_player->get_current_time();
    }

    return 0;
}

unsigned int Task_Player::get_mp_total_time()
{
    if(m_media_player)
    {
        return m_media_player->get_total_time();
    }

    return 0;
}

Media_Player::State Task_Player::get_media_player_state() const
{
    return m_media_player ? m_media_player->state() : Media_Player::State::Idle;
}

void Task_Player::handle_event_pvr_start()
{
    if(m_player)
    {
        auto system_time = System::get_system_time().to_local_time();
        std::string channel_name = mb::sanitize_filename(current_srv()->name());

        char buffer[255];
        snprintf(buffer, 255, "%s_%4d-%02d-%02d.%02d.%02d.%02d",
                 channel_name.c_str(),
                 system_time.year(),
                 system_time.month(),
                 system_time.day(),
                 system_time.hour(),
                 system_time.minute(),
                 system_time.second());
        std::string filename = buffer;

        auto video_type = 0;

        switch(m_current_srv->video_codec())
        {
            case Video_Codec::H264:
                video_type = 1;
                break;

            case Video_Codec::MPEG2:
            case Video_Codec::HEVC:
            case Video_Codec::MPEG4:
            case Video_Codec::UNDEFINED:
            case Video_Codec::None:
                video_type = 0;
                break;
        }

        m_pvr_record_param.video_pid       = m_current_srv->video_pid();
        m_pvr_record_param.video_type      = video_type;

        if(not m_current_srv->audio_pids().empty())
        {
            m_pvr_record_param.audio_pid_count = m_current_srv->audio_pids().size();

            for(size_t it = 0 ; it < m_current_srv->audio_pids().size(); ++it)
            {
                const auto &audio = m_current_srv->audio_pids()[it];
                m_pvr_record_param.audio_pid[it]       = audio.pid;
                m_pvr_record_param.audio_desc_pid      = NULL;
                auto audio_type = 0;

                switch(audio.codec)
                {
                    case Audio_Codec::MP1:
                        audio_type = 0;
                        break;

                    case Audio_Codec::MP2:
                        audio_type = 1;
                        break;

                    case Audio_Codec::AC3:
                        audio_type = 3;
                        break;

                    case Audio_Codec::AAC:
                    default:
                        audio_type = 2;
                        break;
                }

                m_pvr_record_param.audio_type[it]      = audio_type;
            }
        }

        m_pvr_record_param.pcr_pid         = m_current_srv->pcr_pid();
        m_pvr_record_param.filename        = filename;
        m_pvr_record_param.url             = {};
        post_event_cas_pvr_record_start(m_pvr_record_param);
        m_is_recording = true;
        m_pvr_status = {};
        m_last_pvr_seq = 0;
    }

}

void Task_Player::pvr_record_stop()
{
    m_is_recording = false;
    post_event_cas_pvr_record_stop();
    m_last_pvr_seq = 0;
    m_pvr_status = {};
}

void Task_Player::handle_event_pvr_playback_start(std::string url)
{
    if(m_player)
    {
        stop();
    }

    post_event_cas_pvr_play_start(url);
    m_pvr_status = {};
    m_last_pvr_seq = 0;
}

void Task_Player::pvr_playback_stop()
{
    post_event_cas_pvr_play_stop();
    m_last_pvr_seq = 0;
}

void Task_Player::handle_event_pvr_timeshift_start(std::string url)
{
    if(m_player)
    {

        if(m_ts_state != TimeshiftState::Disabled)
        {
            return;
        }

        auto system_time = System::get_system_time().to_local_time();
        std::string channel_name = mb::sanitize_filename(current_srv()->name());

        char buffer[255];
        snprintf(buffer, 255, "%s_%4d-%02d-%02d.%02d.%02d.%02d",
                 channel_name.c_str(),
                 system_time.year(),
                 system_time.month(),
                 system_time.day(),
                 system_time.hour(),
                 system_time.minute(),
                 system_time.second());
        std::string filename = buffer;

        auto video_type = 0;

        switch(m_current_srv->video_codec())
        {
            case Video_Codec::H264:
                video_type = 1;
                break;

            case Video_Codec::MPEG2:
            case Video_Codec::HEVC:
            case Video_Codec::MPEG4:
            case Video_Codec::UNDEFINED:
            case Video_Codec::None:
                video_type = 0;
                break;
        }

        m_pvr_record_param.video_pid       = m_current_srv->video_pid();
        m_pvr_record_param.video_type      = video_type;

        if(not m_current_srv->audio_pids().empty())
        {
            m_pvr_record_param.audio_pid_count = m_current_srv->audio_pids().size();

            for(size_t it = 0 ; it < m_current_srv->audio_pids().size(); ++it)
            {
                const auto &audio = m_current_srv->audio_pids()[it];
                m_pvr_record_param.audio_pid[it]       = audio.pid;
                m_pvr_record_param.audio_desc_pid      = NULL;
                auto audio_type = 0;

                switch(audio.codec)
                {
                    case Audio_Codec::MP1:
                        audio_type = 0;
                        break;

                    case Audio_Codec::MP2:
                        audio_type = 1;
                        break;

                    case Audio_Codec::AC3:
                        audio_type = 3;
                        break;

                    case Audio_Codec::AAC:
                    default:
                        audio_type = 2;
                        break;
                }

                m_pvr_record_param.audio_type[it]      = audio_type;
            }
        }

        m_pvr_record_param.pcr_pid         = m_current_srv->pcr_pid();
        m_pvr_record_param.filename        = filename;
        m_pvr_record_param.url             = url;
        post_event_cas_pvr_timeshift_start(m_pvr_record_param);
        m_pvr_status = {};
        m_last_pvr_seq = 0;
        m_ts_state = TimeshiftState::Starting;
    }
}

void Task_Player::pvr_timeshift_stop()
{
    if(m_ts_state == TimeshiftState::Disabled)
    {
        return;
    }

    post_event_cas_pvr_timeshift_stop();
    m_ts_state = TimeshiftState::Disabled;
    m_last_pvr_seq = 0;
    restart();
}

void Task_Player::pvr_timeshift_pause()
{
    if(m_ts_state == TimeshiftState::Playing)
    {
        post_event_cas_pvr_play_pause();
        m_ts_state = TimeshiftState::Paused;
    }
}

void Task_Player::pvr_timeshift_resume()
{
    post_event_cas_pvr_play_resume();
    m_ts_state = TimeshiftState::Playing;

}

void Task_Player::handle_event_cas_pvr_get_status(const Event_PVR_Status& _status)
{
    if (_status.seq <= m_last_pvr_seq)
    {
        return;
    }

    m_last_pvr_seq = _status.seq;
    m_pvr_status = _status;
    if (m_ts_state == TimeshiftState::Starting && _status.timeshift_record_curr_time >= 1)
    {
        DEBUG_MSG(PLAYER, INFO, "Timeshift REC ready, starting PLAY\n");
        if(m_player)
        {
            m_player->stop(); //live_player
        }
        post_event_cas_pvr_timeshift_play();
        m_ts_state = TimeshiftState::Playing;
    }
}

unsigned int Task_Player::get_pvr_record_curr_time()
{
    return m_pvr_status.record_current_time;
}

std::string Task_Player::get_pvr_record_filename()
{
    return m_pvr_record_param.filename;
}

std::string Task_Player::get_pvr_mount_point()
{
    return m_pvr_status.mount_point;
}

std::string Task_Player::get_pvr_filesystem_type()
{
    return m_pvr_status.filesystem_type;
}

unsigned int Task_Player::get_pvr_playback_curr_time()
{
    return m_pvr_status.player_current_time;
}

unsigned int Task_Player::get_pvr_playback_total_time()
{
    return m_pvr_status.player_total_time;
}

Task_Player::PVR_State Task_Player::get_pvr_player_state() const
{
    return static_cast<PVR_State>(m_pvr_status.state);
}

unsigned int Task_Player::get_pvr_timeshift_rec_curr_time()
{
    return m_pvr_status.timeshift_record_curr_time;
}

unsigned int Task_Player::get_pvr_timeshift_play_curr_time()
{
    return m_pvr_status.timeshift_play_current_time;
}


void Task_Player::handle_event_cc_enable(CC_Type _type)
{
    set_cc_enabled(_type);
}

void Task_Player::set_cc_enabled(CC_Type _type)
{
    if(m_player)
    {
        switch (_type)
        {
            case CC_Type::Closed_Caption:
            {
                using namespace std::placeholders;
                m_player->set_cc_callback(std::bind(&Task::post_event_cc, _1));
                m_player->start_closed_caption();
                break;
            }

            case CC_Type::Subtitle:
            {
                if(m_current_srv)
                {
                    Task::post_event_dvb_subtitle_get(m_current_srv->subtitle_pid());
                }
                break;
            }

            case CC_Type::Disabled:
            default:
            {
                m_player->stop_closed_caption();
                m_player->set_cc_callback({});
                if(m_current_srv)
                {
                    if ( m_current_srv->subtitle_pid() != PID_t{0} and m_current_srv->subtitle_pid() != PID_t{8191})
                    {
                        Task::post_event_dvb_subtitle_del(m_current_srv->subtitle_pid());
                    }
                }
                break;
            }
        }
    }
}

uint64_t Task_Player::get_stc()
{
    if(m_player)
    {
        return m_player->get_player_stc();
    }
    return 0;
}

}
