#pragma once

#include <memory>
#include <chrono>

#include "mb_task.h"
#include "mb_remote_control_handler.h"

typedef struct _lv_display_t lv_display_t;
typedef struct _lv_obj_t lv_obj_t;

namespace mb {

class Channel_Detail;
class OSD;
class OSD_Activate;
class OSD_Audio_LR;
class OSD_Change_Channel;
class OSD_Channel_List;
class OSD_Channel_List_Update;
class OSD_Fast_Install;
class OSD_Instala_Facil;
class OSD_Lnbf_Detection;
class OSD_Main_Menu;
class OSD_Media_Player;
class OSD_Menu_Player;
class OSD_Menu_Plus;
class OSD_Menu_Plus_Record;
class OSD_Message_Box;
class OSD_Popup_Message;
class OSD_Production_Info;
class OSD_Software_Update_Forced;
class OSD_Software_Update_finish;
class OSD_Tpm_Message;
class OSD_Volume;
class OSD_Waiting_Signal;
class OSD_Welcome_Banner;
class OSD_Radio_Icon;
class OSD_Closed_Caption;
class OSD_Draw_Closed_Caption;
class OSD_Draw_Subtitle;
class OSD_USB_Disk;
class Select_Satellite;
class OSD_Parental_Block_Screen;
class OSD_Program_Reminder;
class OSD_Message_System_Restart;
class OSD_Auto_Search_Channel_List;
class OSD_Software_Update;
class OSD_Message_Box;

class Task_OSD final: public Task, public Remote_Control_Handler
{
    friend class Task;
public:
    typedef std::function<void(Viewer_Channel_t)> Viewer_Channel_CB;
    typedef std::function<void(void)> Lineup_Ready_Event_CB;

protected:
    /*
     * This MUST be the first member because it will init LVGL and
     * DESTROY all LVGL's objects in deinit.  If there is ANY LVGL
     * object held by and OSD's classes, it will crash the app
     * due to a double-free on any LVGL object or timers.
     */
    struct Data;
    std::unique_ptr<Data> m_p;
    std::unique_ptr<Select_Satellite> m_osd_select_satellite;
    lv_obj_t *m_main_screen { nullptr };

private:
    std::unique_ptr<Channel_Detail> m_osd_ch_detail;
    std::unique_ptr<OSD_Activate> mb_osd_activate;
    std::unique_ptr<OSD_Audio_LR> m_audio_lr;
    std::unique_ptr<OSD_Change_Channel> m_osd_change_channel;
    std::unique_ptr<OSD_Channel_List> m_osd_ch_list;
    std::unique_ptr<OSD_Channel_List_Update> m_osd_channel_list_update;
    std::unique_ptr<OSD_Fast_Install> m_osd_fast_install;
    std::unique_ptr<OSD_Instala_Facil> m_osd_instala_facil;
    std::unique_ptr<OSD_Main_Menu> m_osd_main_menu;
    std::unique_ptr<OSD_Menu_Plus> m_plus;
    std::unique_ptr<OSD_Menu_Plus_Record> m_plus_record;
    std::unique_ptr<OSD_Message_Box> m_message_box;
    std::unique_ptr<OSD_Production_Info> m_osd_prod_info;
    std::unique_ptr<OSD_Software_Update_finish> m_osd_sw_updt_finish;
    std::unique_ptr<OSD_Volume> m_osd_volume;
    std::unique_ptr<OSD_Waiting_Signal> m_osd_waiting_signal;
    std::unique_ptr<OSD_Welcome_Banner> m_banner;
    std::unique_ptr<OSD_Radio_Icon> m_radio_icon;
    std::unique_ptr<OSD_Parental_Block_Screen> m_osd_parental_block_screen;
    std::unique_ptr<OSD_Closed_Caption> m_closed_caption;
    std::unique_ptr<OSD_Draw_Closed_Caption> m_draw_closed_caption;
    std::unique_ptr<OSD_Draw_Subtitle> m_draw_subtitle;
    std::unique_ptr<OSD_USB_Disk> m_usb_disk;
    std::unique_ptr<OSD_Program_Reminder> m_program_reminder;
    std::unique_ptr<OSD_Message_System_Restart> m_message_system_restart;
    std::unique_ptr<OSD_Auto_Search_Channel_List> m_osd_as_channel_list;
    std::unique_ptr<OSD_Software_Update> m_osd_software_update;

    std::weak_ptr<OSD_Lnbf_Detection> m_osd_lnbf_detection;
    std::weak_ptr<OSD_Popup_Message> m_osd_popup_message;

#ifdef MBGUI_FORCED_UPDATE
    std::unique_ptr<OSD_Software_Update_Forced> m_sw_updt_forced;
#endif

    enum class State
    {
        IDLE,
        HOME,
        INFO,
        CONFIG,
        VOLUME,
        MEDIA_PLAYER,
        RECORD,

    };

    State m_state { State::IDLE };

    Viewer_Channel_CB m_vc_callback;
    Lineup_Ready_Event_CB m_lineup_ready_callback;
    std::chrono::steady_clock::time_point m_busy_until;

    void fast_install_callback();
    void sound_changed_callback(bool _is_muted);
    void factory_reset_callback(bool _result);
    void software_updated_finish_callback(bool _result);
    void show_menu_channel_list_update_callback();
    void show_menu_activate_callback();
    void auto_search_channel_list_callback();
    void show_hide_radio_screen(const Service *_service);
    void hide_radio_screen();
    void check_parental_control();
    void channel_list_update_callback(bool _result);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    virtual void handle_event_autodetect_progress(Event_Transponder_data _progress) override;
    virtual void handle_event_change_osd_home_state() override;
    virtual void handle_event_change_osd_media_player_state() override;
    virtual void handle_event_channel_change(Service *_service) override;
    virtual void handle_event_channel_changed(Service *_service) override;
    virtual void handle_event_channel_preview(Service *_srv) override;
    virtual void handle_event_fast_install() override;
    virtual void handle_event_lineup_ready(const Event_Lineup_Ready &_event) override;
    virtual void handle_event_osd_audio_lr() override;
    virtual void handle_event_osd_channel_list_show() override;
    virtual void handle_event_osd_display_message(const Event_Display_Message &_message) override;
    virtual void handle_event_osd_factory_reset() override;
    virtual void handle_event_osd_mainmenu_show() override;
    virtual void handle_event_osd_menu_plus(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end) override;
    virtual void handle_event_osd_production_info() override;
    virtual void handle_event_player_started() override;
    virtual void handle_event_player_stop() override;
    virtual void handle_event_service_favorite_changed(Service *_service) override;
    virtual void handle_event_sound_changed(const Event_Sound &_event) override;
    virtual void handle_event_usb_plug_event(Event_USB_Plug _event) override;
    virtual void handle_event_zone_id_changed(Zone_ID_t, Zone_ID_t _to_zone_id) override;
    virtual void handle_event_cc_enable(CC_Type _type) override;
    virtual void handle_event_osd_closed_caption() override;
    virtual void handle_event_cc(const Event_CC &_event) override;
    virtual void handle_event_subtitle(const Event_Subtitle_Image &_event) override;
    virtual void handle_event_send_message_to_start_record(ScheduleEntry _sc_entry) override;
    virtual void handle_event_system_need_reset() override;
    virtual void handle_event_ota_found() override;

    virtual void got_focus() override;

    void tvro_banner_hide();
    void tvro_banner_show();
public:
    Task_OSD();
    virtual ~Task_OSD();

    virtual void process() override;

    void show_menu();
    bool has_menu_info();
    void close_menu_info();
    void show_menu_info(Service *_service = nullptr);
    void show_menu_detail();
    void hide_waiting_signal();
    void show_waiting_signal();
    void show_tv_channel_list();
    void show_menu_digit(char _digit, Viewer_Channel_CB _callback);
    void set_force_refresh(bool _value);
    void show_instala_facil();
    void software_updated_finish();
    void set_event_lineup_ready(Lineup_Ready_Event_CB _callback);
    static void set_lnbf_detection(std::shared_ptr<OSD_Lnbf_Detection> _osd_lnbf_detection);
    static void update_parental_control();
    void message_box_callback(bool _ok);
    void software_update_callback(bool _result);


#ifdef MBGUI_FORCED_UPDATE
    void software_updated_forced();
#endif
};

} // namespace mb
