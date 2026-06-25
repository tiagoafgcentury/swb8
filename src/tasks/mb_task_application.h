#pragma once

#include <chrono>
#include <vector>
#include <lvgl.h>

#include "mb_task.h"
#include "common/mb_config.h"

#include "mb_remote_control_handler.h"

#ifdef MBGUI_SAT_MONITOR
#include "mb_pg.h"
#endif // MBGUI_SAT_MONITOR

namespace mb {

class HDMI;
class Display;
class Sound;

class Task_Application final : public Task, public Remote_Control_Handler
{
    friend class Task;

public:
    enum State
    {
        ST_STARTING,
        ST_IDLE,
        ST_WAITING_FOR_LINEUP,
        ST_PROCESS_LINEUP_LOAD,
        ST_WAITING_FOR_APP_STATE,
        ST_EASY_INSTALL,
        ST_PROCESS_AUTOMATIC_SEARCH,
        ST_PRODUCTION_FINAL_TEST,
        ST_WAINTING_FOR_FACTORY_RESET_DONE,
        ST_STAND_BY_MODE,
#ifdef MBGUI_FORCED_UPDATE
        ST_FORCED_UPDATE,
#endif
    };

    Task_Application();
    virtual ~Task_Application();
    typedef std::function<void(Viewer_Channel_t)> ViewerChannel_CB;
    ViewerChannel_CB m_vc_callback;

private:
    bool m_started = false;
    State m_state { ST_STARTING };

    Config m_config;

    std::unique_ptr<HDMI> m_hdmi;
    std::unique_ptr<Display> m_display;
    std::unique_ptr<Sound> m_sound;

    Volume_t m_sound_increment { 5 };
    lv_timer_t *m_tmr_evt { nullptr };

    bool m_time_is_set { false };
    bool m_need_update_datetime_to_save_terms_of_use { false };

    std::optional<Viewer_Channel_t> m_channel_after_process;

    static constexpr std::chrono::milliseconds m_remote_control_key_press_timeout { 500 };
    std::chrono::steady_clock::time_point m_application_state_save_time;
    static constexpr std::chrono::milliseconds m_application_state_save_timeout { 5'000 };
    std::chrono::steady_clock::time_point m_wait_for_reset;

    int m_zap_channel_index {-1};
    bool m_zapping_active {false};
    std::chrono::steady_clock::time_point m_last_zap;
    
    size_t m_system_memory_limit { 0 };
    std::chrono::steady_clock::time_point m_next_memory_check {};

    bool m_checked_updates_in_standby { false };
    bool m_waiting_lineup_load_for_standby_scan { false };
    bool m_is_standby_scan { false };
    bool m_show_standby_changes { false };
    std::chrono::steady_clock::time_point m_standby_scan_time {};
    std::vector<Service> m_services_before_scan;

    void show_standby_changes();

    void change_state(State _state);
    State state() const
    {
        return m_state;
    }

    void write_file(std::string_view _file_name, const char *_data, size_t _data_size)
    {
    auto fp = fopen(_file_name.data(), "w+");

    if (fp)
    {
        fwrite(_data, 1, _data_size, fp);
        fclose(fp);
    }
    else
    {
        perror(__PRETTY_FUNCTION__);
    }
    }

    void application_state_reset_timer()
    {
        m_application_state_save_time = decltype(m_application_state_save_time)::clock::now() + m_application_state_save_timeout;
    }
    void application_state_save();

    static constexpr uint INVALID_CHANNEL = std::numeric_limits<uint>::max();

    void check_system_clock();
    bool change_channel_to(Viewer_Channel_t _viewer_channel);
    void save_datetime_terms_of_use();

public:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

protected:
    virtual void handle_event_application_state_loaded(const Event_Save_Application_State &_event) override;
    virtual void handle_event_channel_changed(Service *) override;
    virtual void handle_event_clock_set_time(const Event_Time &_event) override;
    virtual void handle_event_display_clear() override;
    virtual void handle_event_lineup_ready(const Event_Lineup_Ready &_event) override;
    virtual void handle_event_lineup_satellite_found() override;
    virtual void handle_event_transponder_locked(const Event_Tuner_Lock &_event) override;
    virtual void handle_event_process_automatic_search() override;
    virtual void handle_event_starting_application() override;
    virtual void handle_event_system_factory_reset() override;
    virtual void handle_event_system_factory_reset_done() override;
    virtual void handle_event_toggle_power() override;
    virtual void handle_event_zone_id_changed(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id) override;
    virtual void handle_event_program_access_denied() override;
    virtual void handle_event_save_terms_of_use() override;

    void check_is_production_final_test();
    void system_exit();
    void check_updates();
    bool process_json_file(std::string file);
    bool parse_json(std::string content);
    void load_lineup_production_data();
    bool verify_production_update_status();

    void read_total_system_memory();
    void check_free_system_memory();
public:
    void process() override;
    bool is_in_stand_by();    

#ifdef MBGUI_SAT_MONITOR
    void process_osd();
#endif
};

} // namespace mb
