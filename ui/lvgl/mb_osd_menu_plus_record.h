#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_osd_sleep_timer.h"
#include "mb_osd_clock.h"
#include "mb_osd_keys.h"

#include <sys/statvfs.h>
#include <sys/stat.h>

#include <memory>
#include <functional>

namespace mb {

class OSD_Message_Box;

class OSD_Menu_Plus_Record: public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Plus_Record_CB_t;
    std::unique_ptr<OSD_Message_Box> m_message_box;

private:
    Plus_Record_CB_t m_callback;

    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;

    static constexpr auto info_w = DISPLAY_WIDTH / 1.3;
    static constexpr auto info_h = DISPLAY_HEIGHT / 1.6;

    static constexpr auto file_w = info_w / 2;
    static constexpr auto file_h = info_h / 1.1;

    bool m_rec_info_enable = false;
    bool m_rec_stop_recording = false;

    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_box_record { nullptr };
    lv_obj_t *m_rec_time { nullptr };

    lv_obj_t *m_box_confirm_stop { nullptr };

    lv_obj_t *m_info_box { nullptr };
    lv_obj_t *m_lbl_prog_name { nullptr };
    lv_obj_t *m_lbl_mode { nullptr };
    lv_obj_t *m_lbl_fs { nullptr };
    lv_obj_t *m_lbl_total { nullptr };
    lv_obj_t *m_lbl_available { nullptr };
    lv_obj_t *m_lbl_max_rec { nullptr };
    lv_obj_t *m_lbl_file_size { nullptr };
    lv_obj_t *m_lbl_record_rate { nullptr };

    lv_timer_t *m_hide_timer { nullptr };
    lv_timer_t *m_record_time_timer { nullptr };
    lv_timer_t *m_record_info_timer { nullptr };

    std::string m_mount_point;
    std::string m_filesystem_type;

    unsigned int m_pvr_current_time = 0;
    std::string m_pvr_filename;
    std::string m_pvr_file_path;
    uint64_t m_pvr_file_size = 0;
    uint64_t m_pvr_total_size = 0;
    uint64_t m_pvr_available_size = 0;
    double m_pvr_record_rate = 0.0;
    uint64_t m_pvr_max_rec_time = 0;
    struct statvfs m_pvr_statfs;
    struct stat m_pvr_info;

    bool m_hide_screen_enabled = false;
    // Teclado
    MB_OSD_Keys m_keys;
    MB_OSD_Keys m_confirm_keys;
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 0;
    static constexpr auto button_x = 590;
    static constexpr auto button_y = 100;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = button_w + 23;

    enum class FunctionOption
    {
        STOP,
        REC_INFO,
        COUNT
    };
    FunctionOption m_option = FunctionOption::STOP;

    struct FunctionOptionText
    {
        FunctionOption index;
        std::string_view text;
    };

    std::array<FunctionOptionText, (int)FunctionOption::COUNT> m_options =
    {
        {
            { FunctionOption::STOP, tr(__Parar) },
            { FunctionOption::REC_INFO, "Rec Info" }
        }
    };

    enum class FunctionYesNoOption
    {
        YES,
        NO,
        COUNT
    };
    FunctionYesNoOption m_yes_no_option = FunctionYesNoOption::NO;

    struct FunctionYesNoOptionText
    {
        FunctionYesNoOption index;
        std::string_view text;
    };

    std::array<FunctionYesNoOptionText, (int)FunctionYesNoOption::COUNT> m_yesnooptions =
    {
        {
            { FunctionYesNoOption::YES, tr(__Sim) },
            { FunctionYesNoOption::NO, tr(__Nao) }
        }
    };

    typedef std::chrono::system_clock::time_point Time_Point;
    Time_Point m_time_to_end;

    void start_hide_timer();
    static void hide_video_screen_cb(lv_timer_t *_tm);
    void show_video_screen();
    void get_usb_mount_point();
    void start_record_time_timer();
    static void record_time_cb(lv_timer_t *_tm);
    void del_record_time_timer();
    void update_record_time();
    void show_pvr_midia_informations();
    void create_pvr_midia_informations();
    void update_record_info();
    static void record_info_cb(lv_timer_t *_tm);
    void start_record_info_timer();
    void del_record_info_timer();
    void pvr_stop_record_callback();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    void message_box_callback(bool _ok);

public:
    OSD_Menu_Plus_Record(OSD *_parent);
    virtual ~OSD_Menu_Plus_Record();

    void show_menu_plus_record(lv_obj_t *_bgd, const Service *_srv, std::chrono::time_point<std::chrono::system_clock> _time_to_end, Plus_Record_CB_t _callback);
    void show_usb_plug_event(Event_USB_Plug _event);
};

} // namespace mb
