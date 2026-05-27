#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_osd_sleep_timer.h"
#include "mb_osd_clock.h"
#include "mb_osd_keys.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Menu_Plus_Sleep: public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Plus_Sleep_CB_t;

private:
    Plus_Sleep_CB_t m_callback;

    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;

    static int s_sleep_timer_value;
    static lv_timer_t *s_sleep_timer;

    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_box_sleep { nullptr };
    lv_obj_t *m_button { nullptr };
    lv_obj_t *m_button_label { nullptr };
    static lv_obj_t *s_sleep_logo;

    lv_timer_t *m_exit_timer { nullptr };

    // Lista de possiveis tempos em minutos para desligar o receptor
    static constexpr int sleep_timer_value_array[6] = { 0, 15, 30, 45, 60, 120 };

    void sleep_timer_next();
    void sleep_timer_previous();
    void start_sleep_timer();
    void process_sleep_timer();
    void delete_sleep_timer();
    void update_sleep_timer_label();

    static void process_sleep_timer_cb(lv_timer_t *_tm);
    static void exit_sleep_screen_cb(lv_timer_t *_tm);
    void start_exit_timer();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Plus_Sleep(OSD *_parent);
    virtual ~OSD_Menu_Plus_Sleep();

    void show_menu_plus_sleep(lv_obj_t *_bgd, Plus_Sleep_CB_t _callback);
    void start_standby();
    void set_sleep_timer_value();
    std::pair<int, std::string> get_sleep_timer_pair();
    static OSD_Menu_Plus_Sleep s_instance;
};

} // namespace mb
