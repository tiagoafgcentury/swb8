#pragma once

#include "hal/mb_media_player.h"
#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_osd_clock.h"
#include "mb_osd_keys.h"
#include "mb_osd_message_box.h"


#include <memory>
#include <functional>

namespace mb {

class OSD_Message_Box;

class OSD_Menu_Plus_Timeshift: public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Plus_Timeshift_CB_t;
    std::unique_ptr<OSD_Message_Box> m_message_box;


private:
    Plus_Timeshift_CB_t m_callback;

    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;

    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_box_timeshift { nullptr };
    lv_obj_t *m_rec_time { nullptr };
    lv_obj_t *m_play_time { nullptr };
    lv_obj_t *m_box_confirm_stop { nullptr };
    lv_obj_t *m_button { nullptr };
    lv_obj_t *m_label_speed = nullptr;

    lv_timer_t *m_hide_timer { nullptr };
    lv_timer_t *m_record_time_timer { nullptr };

    bool m_hide_screen_enabled = false;
    bool m_rec_stop_recording = false;

    bool m_timeshift_initializing {false};

    enum class Tms_Functions_t
    {
        TMS_PLAY = 0,
        TMS_PAUSED,
        TMS_REWIND,
        TMS_FORWARD,
        TMS_CANCEL,
        MAX_TMS_FUNCTIONS,
    };

    uint16_t m_selected_item = static_cast<uint16_t>(Tms_Functions_t::TMS_PLAY);
    std::array<lv_obj_t *, static_cast<uint8_t>(Tms_Functions_t::MAX_TMS_FUNCTIONS)> m_mp_functions;
    std::array<lv_obj_t *, static_cast<uint8_t>(Tms_Functions_t::MAX_TMS_FUNCTIONS)> m_mp_functions_sel;
    Media_Player::MP_Speed_Forward m_current_forward_speed {Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL};
    Media_Player::MP_Speed_Rewind m_current_rewind_speed {Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL};

    MB_OSD_Keys m_confirm_keys;
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 0;
    static constexpr auto button_x = 750;
    static constexpr auto button_y = 100;
    static constexpr auto button_w = 110;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = button_w + 23;

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

    std::string m_mount_point = "";

    void create_tms_frame();
    void get_usb_mount_point();
    void start_hide_timer();
    static void hide_video_screen_cb(lv_timer_t *_tm);
    void show_video_screen();
    void start_record_time_timer();
    static void record_time_cb(lv_timer_t *_tm);
    void del_record_time_timer();
    void update_record_time();
    void update_record_info();
    static void record_info_cb(lv_timer_t *_tm);
    void start_record_info_timer();
    void del_record_info_timer();
    void set_menu_selection(uint8_t _menu, bool _selected);
    void move_selection(uint8_t _from, uint8_t _to);
    void move_left();
    void move_right();
    void next_forward_speed();
    void next_rewind_speed();
    void update_rewind_label_speed();
    void update_forward_label_speed();

    void show_message_timeshift_initializing();
    void hide_message_timeshift_initializing();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Plus_Timeshift(OSD *_parent);
    virtual ~OSD_Menu_Plus_Timeshift();

    void show_menu_plus_timeshift(lv_obj_t *_bgd, const Service *_srv, Plus_Timeshift_CB_t _callback);
};

} // namespace mb

