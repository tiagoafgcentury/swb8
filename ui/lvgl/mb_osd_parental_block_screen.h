#pragma once

#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_osd.h"
#include "tasks/mb_task_cas.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task_eit_events.h"
#include "mb_osd_keys.h"
#include "mb_remote_control_handler.h"

#include "lvgl.h"
#include <functional>

namespace mb {

class OSD_Parental_Block_Screen: public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Banner_CB_t;

private:
    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;
    static constexpr auto name_width = 400;
    static constexpr auto name_height = 40;

    static constexpr auto _1_MINUTE = 60000;
    static constexpr auto _1_SECOND = 1000;
    int m_next_event = _1_SECOND;

    //Menu_Options
    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_front_box { nullptr };

    static constexpr int m_front_box_w = 600;
    static constexpr int m_front_box_h = 360;

    lv_obj_t *m_event_status_label { nullptr };
    lv_obj_t *m_event_name_label { nullptr };
    lv_obj_t *m_event_rating_label { nullptr };
    lv_style_t m_front_style = {};

    lv_timer_t *m_event_status_timer { nullptr };
    lv_timer_t *m_check_password_timer { nullptr };

    Volume_t m_previous_volume { 0 };
    bool m_mute_state { false };
    enum class Status
    {
        Unblocked,
        Blocked,
        Password_Pressed,
        COUNT
    };
    Status m_status = Status::Unblocked;

    struct passwd_Info_t
    {
        lv_obj_t *rect { nullptr };
        lv_obj_t *circle { nullptr };
    };

    struct Event_Info_t
    {
        UTC_MJD start_time;
        UTC_MJD end_time;
        uint32_t duration = 0;
        uint32_t seconds_to_end = 0;
        uint8_t age = 0;
    };

    static constexpr uint16_t MAX_CHAR_PASSWD = 4;
    passwd_Info_t m_passwd_info[MAX_CHAR_PASSWD];
    std::string m_pass_buffer = "";
    char m_password[5] = "";
    uint8_t m_parental_rating = 0;
    uint8_t m_age_limit = 0;
    uint32_t m_seconds_to_end = 0;

    static constexpr auto m_password_w = 83;
    static constexpr auto m_password_h = 83;
    static constexpr auto m_password_s = (m_front_box_w - 4 * m_password_w) / 5;
    static constexpr auto m_password_radius = 12;
    static constexpr auto m_password_gap = 20;
    static constexpr auto m_password_x = (m_front_box_w - 4 * m_password_w - 3 * m_password_gap) / 2;
    static constexpr auto m_password_y = m_front_box_h / 2 - m_password_h / 2;

#ifdef MBGUI_USE_RLOTTIE
    lv_obj_t *m_blocked_logo { nullptr };
#endif

    void hide_block_screen();
    void create_block_screen(std::string_view _age);
    void refresh_timer_callback();
    void password_timer_callback();
    void delete_hide_block_screen();
    void process_numeric_key(int num);
    std::pair<Status, uint8_t> get_event_information();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Parental_Block_Screen(OSD *_parent);
    virtual ~OSD_Parental_Block_Screen();

    void init_parental_control();
    void update_parental_control();
    void init_timer();
    void restart_timer();
};

} // namespace mb

