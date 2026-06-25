#pragma once

#include "mb_events.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <mqueue.h>

#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "common/static_string.h"

#ifndef NDEBUG
#define POST_CALLER __FILE__, __LINE__,
#define POST_CALLER_P const char* _file, int _line,
#define POST_DUMP DEBUG_MSG(TASK, DEBUG, "Caller " << basename(_file) << ":" << _line << "\t");
#else
#define POST_CALLER
#define POST_CALLER_P
#define POST_DUMP
#endif

namespace mb {

struct Event_Autodetect_LNBf;
struct Event_CAS_CAT_Table_Section;
struct Event_CAS_Fingerprint;
struct Event_CAS_Request_Descramble;
struct Event_Channel_Digits;
struct Event_Display_Message;
struct Event_System_Settings;
struct Event_Lineup_Ready;
struct Event_OTA_DSI;
struct Event_Remote_Control;
struct Event_Save_Application_State;
struct Event_Sound;
struct Event_Time;
struct Event_Tuner_Lock;
struct Event_LNBF_Params;

struct Lineup;
struct Satellite;
struct Transponder;

class Task_Application;
class Task_CAS;
class Task_Database;
class Task_Demux;
class Task_EIT_Events;
class Task_Easy_Install;
class Task_HTTP_Server;
class Task_OSD;
class Task_Player;
class Task_Remote_Control;
class Task_Sat_Monitor;
class Task_Tuner;

class Task
{
public:
    typedef std::chrono::steady_clock Clock;
    typedef std::chrono::time_point<Clock> Time_Point;

    typedef std::function<void()> Event_Callback;

    struct Event
    {
        explicit Event(Event_Callback &&_callback):
            callback(std::move(_callback))
        {}

        Event_Callback callback;
    };

    struct Timed_Event : public Event
    {
        Timed_Event(Time_Point _time_point,
                    std::chrono::milliseconds _interval,
                    Event_Callback &&_callback):
            Event(std::move(_callback)),
            time_point(std::move(_time_point)),
            interval(std::move(_interval))
        {}

        Time_Point time_point;
        std::chrono::milliseconds interval;
    };

private:
    static mqd_t s_queue_cas;
    static mqd_t s_queue_gui;

    static std::atomic<int> s_queue_use_count;

    static std::vector<Event> s_events;
    static std::vector<Timed_Event> s_timed_events;
    static std::vector<Timed_Event> s_timed_events_alt;
    static bool s_events_in_progress;

    static std::mutex s_events_async_mutex;
    static std::vector<Event> s_events_async;

    static std::mutex s_ipc_mutex;
    static std::chrono::steady_clock::time_point s_last_ipc_reopen;

public:
    static Task_Application *s_task_application;
    static Task_CAS *s_task_cas;
    static Task_Demux *s_task_demux;
    static Task_Database *s_task_database;
    static Task_OSD *s_task_osd;
    static Task_Player *s_task_player;
    static Task_Remote_Control *s_task_remote_control;
    static Task_Tuner *s_task_tuner;
    static Task_Easy_Install *s_task_easy_install;
#ifdef MB_USE_MICROHTTPD
    static Task_HTTP_Server *s_task_http_server;
#endif
#ifdef MBGUI_SAT_MONITOR
    static Task_Sat_Monitor *s_task_sat_monitor;
#else
    static Task_EIT_Events *s_task_eit_events;
#endif

protected:
    virtual void handle_event_application_ready();
    virtual void handle_event_application_state_load();
    virtual void handle_event_application_state_loaded(const Event_Save_Application_State &_event);
    virtual void handle_event_application_state_save(const Event_Save_Application_State &_event);
    virtual void handle_event_autodetect_lnbf_finished(bool _success);
    virtual void handle_event_blind_scan_progress(std::weak_ptr<Event_Blind_Scan_Progress> _event);
    virtual void handle_event_autodetect_lnbf_start(std::weak_ptr<Event_Autodetect_LNBf> _event);
    virtual void handle_event_set_default_lnbf();
    virtual void handle_event_set_lnbf_type(LNBF_Type);
    virtual void handle_event_change_lnbf_type(std::function<void(std::string &result)> _callback);
    virtual void handle_event_cas_start_emm_filtering(const Transponder *_tp);
    virtual void handle_event_cc(const Event_CC &_event);
    virtual void handle_event_subtitle(const Event_Subtitle_Image &_event);
    virtual void handle_event_cc_enable(CC_Type _type);
    virtual void handle_event_channel_change(Service *_srv);
    virtual void handle_event_channel_changed(Service *_srv);
    virtual void handle_event_channel_preview(Service *_srv);
    virtual void handle_event_clock_need_update();
    virtual void handle_event_clock_set_time(const Event_Time &_event);
    virtual void handle_event_clock_time_set(bool _success);
    virtual void handle_event_delete_satellite(const unsigned int id);
    virtual void handle_event_display_clear();
    virtual void handle_event_eit_update();
    virtual void handle_event_fast_install();
    virtual void handle_event_lineup_build(std::weak_ptr<Event_List_Update> _event, bool restart = true);
    virtual void handle_event_lineup_update(std::weak_ptr<Event_List_Update_Channel_List> _event);
    virtual void handle_event_lineup_changed();
    virtual void handle_event_lineup_load();
    virtual void handle_event_lineup_ready(const Event_Lineup_Ready &_event);
    virtual void handle_event_lineup_satellite_found();
    virtual void handle_event_lineup_save();
    virtual void handle_event_lineup_save_zone_id(Zone_ID_t _zone_id, Segment_ID_t _segment_id);
    virtual void handle_event_lineup_start();
    virtual void handle_event_lnbf_config_save(const Event_LNBF_Params &_event);
    virtual void handle_event_easy_install_save(bool _easy_install_finish);
    virtual void handle_event_osd_audio_lr();
    virtual void handle_event_osd_channel_list_show();
    virtual void handle_event_osd_display_message(const Event_Display_Message &_message);
    virtual void handle_event_osd_factory_reset();
    virtual void handle_event_osd_mainmenu_show();
    virtual void handle_event_osd_menu_plus(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
    virtual void handle_event_osd_closed_caption();
    virtual void handle_event_osd_production_info();
    virtual void handle_event_ota_update_get(PID_t _pid, std::weak_ptr<Event_OTA_DSI> _event);
    virtual void handle_event_dvb_subtitle_get(PID_t _pid);
    virtual void handle_event_dvb_subtitle_del(PID_t _pid);
    virtual void handle_event_player_change_audio(PID_t _new_pid);

    virtual void handle_event_media_player_start(std::string url, uint8_t _mode);
    virtual void handle_event_media_player_stop();
    virtual void handle_event_media_player_pause();
    virtual void handle_event_media_player_resume();
    virtual void handle_event_media_player_next(std::string url, uint8_t _mode);
    virtual void handle_event_media_player_previous(std::string url, uint8_t _mode);
    virtual void handle_event_media_player_forward(uint16_t speed, bool _is_video);
    virtual void handle_event_media_player_rewind(uint16_t speed, bool _is_video);
    virtual void handle_event_pvr_start();
    virtual void handle_event_pvr_playback_start(std::string url);
    virtual void handle_event_pvr_timeshift_start(std::string url);
    virtual void handle_event_pvr_timeshift_stop();
    virtual void handle_event_pvr_timeshift_play();
    virtual void handle_event_player_restart();
    virtual void handle_event_player_started();
    virtual void handle_event_player_stop();
    virtual void handle_event_player_stopped();
    virtual void handle_event_starting_application();
    virtual void handle_event_process_automatic_search();
    virtual void handle_event_satellite_list_load(std::function<void(std::vector<Satellite> &satellites)> _cb);
    virtual void handle_event_update_channel_list(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_data);
    virtual void handle_event_add_satellite(Satellite);
    virtual void handle_event_schedule_load(std::function<void(std::vector<ScheduleEntry> scheds)> _schedule);
    virtual void handle_event_insert_schedule(ScheduleEntry _entry);
    virtual void handle_event_delete_schedule(int _agenda_id);
    virtual void handle_event_update_schedule(ScheduleEntry _entry);
    virtual void handle_event_autodetect_progress(const Event_Transponder_data _progress);
    virtual void handle_event_service_favorite_changed(Service *_srv);
    virtual void handle_event_service_pmt_section(PID_t _pid, Service_ID_t _service_id, DVB_Table_Section _pmt);
    virtual void handle_event_service_pmt_get_next_section(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number);
    virtual void handle_event_services_update();
    virtual void handle_event_sound_changed(const Event_Sound &_event);
    virtual void handle_event_change_osd_media_player_state();
    virtual void handle_event_change_osd_home_state();
    virtual void handle_event_system_display_settings_load();
    virtual void handle_event_system_display_settings_loaded(const Event_System_Settings &_event);
    virtual void handle_event_system_display_settings_save(const Event_System_Settings &_event);
    virtual void handle_event_system_display_settings_saved();
    virtual void handle_event_system_factory_reset();
    virtual void handle_event_system_factory_reset_done();
    virtual void handle_event_system_memory_is_low();
    virtual void handle_event_system_need_reset();
    virtual void handle_event_ota_found();
    virtual void handle_event_transponder_lock(const Transponder *_tp, bool _force = false);
    virtual void handle_event_transponder_locked(const Event_Tuner_Lock &_event);
    virtual void handle_event_update_satellite(Satellite);
    virtual void handle_event_usb_plug_event(Event_USB_Plug);
    virtual void handle_event_zone_id_changed(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id);
    virtual void handle_event_toggle_power();
    virtual void handle_event_pin_reset();
    virtual void handle_event_delete_all_services();
    virtual void handle_event_start_recording(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
    virtual void handle_event_send_message_to_start_record(ScheduleEntry _sc_entry);
    virtual void handle_event_save_terms_of_use();

    // IPC Events
    virtual void handle_event_cas_fingerprint_get();
    virtual void handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event);
    //Este handler é redirecionado para handle_event_cas_popup_message -> handle_event_osd_display_message
    virtual void handle_event_cas_request_descramble(const Event_CAS_Request_Descramble &_event);
    virtual void handle_event_cas_request_descramble_done(bool _result);
    virtual void handle_event_cas_request_descramble_pmt_update(const Event_CAS_Request_Descramble &_event);
    virtual void handle_event_cas_request_descramble_stop();
    virtual void handle_event_cas_request_descramble_stop_done();
    virtual void handle_event_cas_send_cat_table_section(const Event_CAS_CAT_Table_Section &_event);
    virtual void handle_event_program_access_denied();
    virtual void handle_event_cas_pvr_timeshift_start(Event_PVR_Record_Param _param);
    virtual void handle_event_cas_pvr_timeshift_play();
    virtual void handle_event_cas_pvr_timeshift_stop();
    virtual void handle_event_cas_pvr_record_start(Event_PVR_Record_Param _param);
    virtual void handle_event_cas_pvr_record_stop();
    virtual void handle_event_cas_pvr_record_pause();
    virtual void handle_event_cas_pvr_record_resume();
    virtual void handle_event_cas_pvr_play_start(std::string url);
    virtual void handle_event_cas_pvr_play_stop();
    virtual void handle_event_cas_pvr_play_pause();
    virtual void handle_event_cas_pvr_play_resume();
    virtual void handle_event_cas_pvr_play_forward(uint16_t _mp_speed);
    virtual void handle_event_cas_pvr_play_rewind(uint16_t _mp_speed);
    virtual void handle_event_cas_pvr_get_status(const Event_PVR_Status &_status);
    virtual void handle_event_cas_pvr_play_next(std::string url);
    virtual void handle_event_cas_switch_folder(bool _is_sky);
    virtual void handle_event_cas_exit();


#ifdef MBGUI_PERIODIC_DUMP
    virtual void handle_event_debbug_dump_status();
#endif // MBGUI_PERIODIC_DUMP

#ifdef MBGUI_SAT_MONITOR
    virtual void handle_event_terminal_text(const StaticString &_text);
#endif

    static void process_ipc_messages();
    static void process_events();
    virtual void process() = 0;

private:
    static void send_ipc_message(mqd_t _mqdes, const uint8_t *_msg_ptr, size_t _msg_len);

public:
    Task();
    virtual ~Task();

    static bool run_processes();

    static void post_event(Event_Callback _callback);
    static void post_event(Time_Point _tp, Event_Callback _callback);
    static void post_timer(std::chrono::milliseconds _tp, Event_Callback _callback);

    template <typename T>
    static void post_event(T &_tp, Event_Callback _callback)
    {
        post_event(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() + _tp), std::move(_callback));
    }

    static void post_event_application_ready();
    static void post_event_application_state_load();
    static void post_event_application_state_loaded(const Event_Save_Application_State &_event);
    static void post_event_application_state_save(const Event_Save_Application_State &_event);
    static void post_event_autodetect_lnbf_finished(bool _success);
    static void post_event_blind_scan_progress(std::weak_ptr<Event_Blind_Scan_Progress> _event);
    static void post_event_autodetect_lnbf_start(std::weak_ptr<Event_Autodetect_LNBf> _event);
    static void post_event_autodetect_progress(const Event_Transponder_data _progress);
    static void post_event_cas_start_emm_filtering(const Transponder *_tp);
    static void post_event_cc(Event_CC _event);
    static void post_event_subtitle(Event_Subtitle_Image _event);
    static void post_event_cc_enable(CC_Type _type);
    static void post_event_change_osd_home_state();
    static void post_event_change_osd_media_player_state();
    static void post_event_channel_change(POST_CALLER_P Service *_srv);
    static void post_event_channel_changed(POST_CALLER_P Service *_srv);
    static void post_event_channel_preview(POST_CALLER_P Service*_srv);
    static void post_event_clock_need_update();
    static void post_event_clock_set_time(const Event_Time &_event);
    static void post_event_clock_time_set(bool _success);
    static void post_event_delete_satellite(const unsigned int id);
    static void post_event_display_clear();
    static void post_event_easy_install_save(bool _easy_install_finish);
    static void post_event_eit_update();
    static void post_event_fast_install();
    static void post_event_lineup_build(std::weak_ptr<Event_List_Update> _event, bool restart = true);
    static void post_event_lineup_update(std::weak_ptr<Event_List_Update_Channel_List> _event);
    static void post_event_lineup_changed();
    static void post_event_lineup_load();
    static void post_event_lineup_ready(const Event_Lineup_Ready &_event);
    static void post_event_lineup_satellite_found();
    static void post_event_lineup_save();
    static void post_event_lineup_save_zone_id(Zone_ID_t _zone_id, Segment_ID_t _segment_id);
    static void post_event_lineup_start();
    static void post_event_lnbf_config_save(const Event_LNBF_Params &_event);

    static void post_event_media_player_forward(uint16_t speed, bool _is_video);
    static void post_event_media_player_next(std::string url, uint8_t _mode);
    static void post_event_media_player_pause();
    static void post_event_media_player_previous(std::string url, uint8_t _mode);
    static void post_event_media_player_resume();
    static void post_event_media_player_rewind(uint16_t speed, bool _is_video);
    static void post_event_media_player_start(std::string url, uint8_t _mode);
    static void post_event_media_player_stop();

    static void post_event_osd_audio_lr();
    static void post_event_osd_channel_list_show();
    static void post_event_osd_display_message(Event_Display_Message _message);
    static void post_event_osd_factory_reset();
    static void post_event_osd_mainmenu_show();
    static void post_event_osd_menu_plus(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
    static void post_event_osd_closed_caption();
    static void post_event_osd_production_info();
    static void post_event_ota_update_get(PID_t _pid, std::weak_ptr<Event_OTA_DSI> _event);
    static void post_event_dvb_subtitle_get(PID_t _pid);
    static void post_event_dvb_subtitle_del(PID_t _pid);
    static void post_event_pin_reset();
    static void post_event_delete_all_services();
    static void post_event_start_recording(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
    static void post_event_player_change_audio(PID_t _new_pid);
    static void post_event_player_restart();
    static void post_event_player_started();
    static void post_event_player_stop();
    static void post_event_player_stopped();
    static void post_event_process_automatic_search();
    static void post_event_pvr_start();
    static void post_event_pvr_playback_start(std::string url);
    static void post_event_pvr_record_get_time_duration();
    static void post_event_pvr_timeshift_start(std::string url);
    static void post_event_pvr_timeshift_stop();
    static void post_event_pvr_timeshift_play();
    static void post_event_satellite_list_load(std::function<void(std::vector<Satellite>)>);
    static void post_event_add_satellite(Satellite);
    static void post_event_schedule_load(std::function<void(std::vector<ScheduleEntry>)>);
    static void post_event_insert_schedule(ScheduleEntry _entry);
    static void post_event_delete_schedule(int _agenda_id);
    static void post_event_update_schedule(ScheduleEntry _entry);
    static void post_event_service_favorite_changed(Service *_srv);
    static void post_event_service_pmt_get_next_section(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number);
    static void post_event_service_pmt_section(PID_t _pid, Service_ID_t _service_id, DVB_Table_Section _pmt);
    static void post_event_services_update();
    static void post_event_set_default_lnbf();
    static void post_event_set_lnbf_type(LNBF_Type _lnbf_type);
    static void post_event_change_lnbf_type(std::function<void(const std::string)>);
    static void post_event_sound_changed(const Event_Sound &_event);
    static void post_event_starting_application();
    static void post_event_system_display_settings_load();
    static void post_event_system_display_settings_loaded(const Event_System_Settings &_event);
    static void post_event_system_display_settings_save(const Event_System_Settings &_event);
    static void post_event_system_display_settings_saved();
    static void post_event_system_factory_reset();
    static void post_event_system_factory_reset_done();
    static void post_event_system_memory_is_low();
    static void post_event_system_need_reset();
    static void post_event_ota_found();
    static void post_event_toggle_power();
    static void post_event_transaction_commit();
    static void post_event_transaction_start();
    static void post_event_transponder_lock(POST_CALLER_P const Transponder *_tp, bool _force = false);
    static void post_event_transponder_locked(const Event_Tuner_Lock &_event);
    static void post_event_update_channel_list(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_data);
    static void post_event_update_satellite(Satellite);
    static void post_event_usb_plug_event(Event_USB_Plug);
    static void post_event_zone_id_changed(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id);
    static void post_event_send_message_to_start_record(ScheduleEntry _sc_entry);
    static void post_event_program_access_denied();
    static void post_event_save_terms_of_use();

    // IPC Events
    static void post_event_cas_fingerprint_get();
    static void post_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event);
    static void post_event_cas_popup_message(Event_Display_Message _event);
    static void post_event_cas_request_descramble(Event_CAS_Request_Descramble _event);
    static void post_event_cas_request_descramble_done(bool _result);
    static void post_event_cas_request_descramble_pmt_update(Event_CAS_Request_Descramble _event);
    static void post_event_cas_request_descramble_stop();
    static void post_event_cas_request_descramble_stop_done();
    static void post_event_cas_send_cat_table_section(Event_CAS_CAT_Table_Section _event);
    static void post_event_cas_pvr_timeshift_start(Event_PVR_Record_Param _param);
    static void post_event_cas_pvr_timeshift_play();
    static void post_event_cas_pvr_timeshift_stop();
    static void post_event_cas_pvr_record_start(Event_PVR_Record_Param _param);
    static void post_event_cas_pvr_record_stop();
    static void post_event_cas_pvr_record_pause();
    static void post_event_cas_pvr_record_resume();
    static void post_event_cas_pvr_play_start(std::string url);
    static void post_event_cas_pvr_play_stop();
    static void post_event_cas_pvr_play_pause();
    static void post_event_cas_pvr_play_resume();
    static void post_event_cas_pvr_play_forward(uint16_t _mp_speed);
    static void post_event_cas_pvr_play_rewind(uint16_t _mp_speed);
    static void post_event_cas_pvr_play_next(std::string url);
    static void post_event_cas_pvr_get_status(Event_PVR_Status _status);
    static void post_event_cas_switch_folder(bool _is_sky);
    static void post_event_cas_exit();

#ifdef MBGUI_SAT_MONITOR
    static void post_event_terminal_text(const StaticString &_text);
#endif

    static Task_Tuner *get_tuner_task()
    {
        return s_task_tuner;
    }

    static Task_Demux *get_demux_task()
    {
        return s_task_demux;
    }
};

}
