#pragma once

#include "mb_task.h"
#include "hal/mb_player.h"
#include "hal/mb_media_player.h"

#ifndef NDEBUG
#define FUNC_NAME std::string_view _fun_name,
#define FUNC_PARAM __FUNCTION__,
#else
#define FUNC_NAME
#define FUNC_PARAM
#endif

namespace mb {

class Task_Player final: public Task
{
    friend class Task;

public:
    enum State
    {
        ST_IDLE,
        ST_WAITING_FOR_LOCK,
        ST_WAITING_FOR_PMT,
        ST_WAITING_FOR_PMT_UPDATE,
        ST_WAITING_FOR_DESCRABLE_START,
        ST_WAITING_FOR_DESCRABLE_STOP,
    };

    State state() const
    {
        return m_state;
    }

    void post_current_service_to_cas(bool _update_only = false);

    enum class PVR_State
    {
        Idle,
        Opened,
        Closing,
        Starting,
        Started,
        Stopping,
        Stopped,
        Paused,
        Pausing,
        Finish,
        Error,
    };

    enum class TimeshiftState
    {
        Disabled,
        Starting,
        Recording,
        Playing,
        Paused
    };

private:
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Media_Player> m_media_player;

    bool     m_channel_change_in_progress = false;
    Service* m_pending_srv = nullptr;
    void unlock_channel_change();

    Player::State m_last_seen_state { Player::State::Idle };

    Service *m_current_srv { nullptr };
    bool m_current_srv_has_nagra_cas { false };
    Service *m_change_srv { nullptr };
    DVB_Table_Section m_current_srv_pmt;
    bool m_current_srv_is_updated { false };
    bool m_current_srv_is_blocked_by_cas { false };
    bool m_current_srv_will_be_recorded { false };
    bool m_is_recording { false };
    typedef std::chrono::system_clock::time_point Time_Point;
    Time_Point m_time_to_end = Time_Point::max();

    Event_PVR_Record_Param m_pvr_record_param = {};
    Event_PVR_Status m_pvr_status = {};
    uint32_t m_last_pvr_seq = 0;
    PVR_State m_pvr_state { PVR_State::Idle };
    TimeshiftState m_ts_state { TimeshiftState::Disabled };

    State m_state { ST_IDLE };
    std::atomic<int> m_change_channel_timer{0};

    // Timer to avoid infinite restarts
    std::chrono::steady_clock::time_point m_last_signal_check;
    std::chrono::steady_clock::time_point m_last_restart;
    std::chrono::steady_clock::time_point m_player_started_at;

    void change_state(FUNC_NAME State _new_state);
    void change_channel_start();
    void change_channel_start_player();
    void change_channel_finish(bool _success);
    void reset_current_service();
    void show_signal_failed(bool _is_failed);
    void verify_player_and_tuner_status();

protected:
    virtual void handle_event_channel_change(Service *_srv) override;
    virtual void handle_event_lineup_changed() override;
    virtual void handle_event_player_change_audio(PID_t _new_pid) override;
    virtual void handle_event_player_restart() override;
    virtual void handle_event_player_stop() override;
    virtual void handle_event_transponder_lock(const Transponder *_tp, bool _force = false) override;
    virtual void handle_event_transponder_locked(const Event_Tuner_Lock &_event) override;
    virtual void handle_event_cas_request_descramble_done(bool _result) override;
    virtual void handle_event_cas_request_descramble_stop_done() override;
    virtual void handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event) override;
    virtual void handle_event_service_pmt_section(PID_t _pid, Service_ID_t _service_id, DVB_Table_Section _pmt) override;
    virtual void handle_event_osd_display_message(const Event_Display_Message &_message) override;
    virtual void handle_event_start_recording(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end) override;

    virtual void handle_event_media_player_start(std::string url, uint8_t _mode) override;
    virtual void handle_event_media_player_stop() override;
    virtual void handle_event_media_player_pause() override;
    virtual void handle_event_media_player_resume() override;
    virtual void handle_event_media_player_previous(std::string url, uint8_t _mode) override;
    virtual void handle_event_media_player_next(std::string url, uint8_t _mode) override;
    virtual void handle_event_media_player_forward(uint16_t speed, bool _is_video) override;
    virtual void handle_event_media_player_rewind(uint16_t speed, bool _is_video) override;

    virtual void handle_event_pvr_start() override;
    virtual void handle_event_pvr_playback_start(std::string url) override;
    virtual void handle_event_cas_pvr_get_status(const Event_PVR_Status &_status) override;
    virtual void handle_event_pvr_timeshift_start(std::string url) override;
    virtual void handle_event_cc_enable(CC_Type _type) override;

public:
    Task_Player();
    virtual ~Task_Player();

    virtual void process() override;

    static Service *current_srv();

    void stop();
    void restart();
    void restore_signal_state();
    Player::State get_player_state() const;

    unsigned int get_mp_curr_time();
    unsigned int get_mp_total_time();
    Media_Player::State get_media_player_state() const;

    PVR_State get_pvr_player_state() const;

    unsigned int get_pvr_playback_curr_time();
    unsigned int get_pvr_playback_total_time();

    unsigned int get_pvr_record_curr_time();
    std::string get_pvr_record_filename();

    std::string get_pvr_mount_point();
    std::string get_pvr_filesystem_type();

    unsigned int get_pvr_timeshift_play_curr_time();
    unsigned int get_pvr_timeshift_rec_curr_time();
    uint64_t get_stc();

    void pvr_timeshift_start();
    void pvr_timeshift_stop();
    void pvr_timeshift_resume();
    void pvr_timeshift_pause();

    void pvr_record_stop();
    void pvr_playback_stop();

#ifdef MBGUI_PERIODIC_DUMP
    virtual void handle_event_debbug_dump_status() override;
#endif // MBGUI_PERIODIC_DUMP

    void set_cc_enabled(CC_Type _type);
};

std::string_view to_str(Task_Player::State _state);

}
