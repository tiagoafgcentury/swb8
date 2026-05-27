#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_remote_control_handler.h"

#include <memory>
#include <array>

namespace mb {

class Fade_Canvas;

class OSD_Keyboard:  public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(Satellite)> Osd_Keyboard_cb_t;

private:
    lv_obj_t *m_mainscreen { nullptr };
    lv_obj_t *m_line { nullptr };
    lv_obj_t *m_textarea { nullptr };
    static const int s_kb_ctrl[];

    static constexpr auto s_keyboard_size = 40;
    static constexpr auto s_key_width = 40;
    static constexpr auto s_key_heigth = 40;

    typedef std::array<std::string_view, s_keyboard_size> Keyboard_t;
    Keyboard_t m_kb_map_upper;
    Keyboard_t m_kb_map_lower;
    Keyboard_t m_kb_map_num;
    Keyboard_t m_kb_map_special;

    std::array<lv_obj_t *, s_keyboard_size> m_key_box;
    std::array<lv_obj_t *, s_keyboard_size> m_key_label;

    typedef std::pair<int, int> Nav_Keyboard_t;
    static const std::pair<int, int> s_kb_nav_map[];
    int m_active_key = 0;

    Satellite m_satellite;

    enum class Active_Kbd
    {
        Lower,
        Upper,
        Num,
        Special,
        COUNT
    };
    Active_Kbd m_active_kbd = Active_Kbd::Lower;

    Osd_Keyboard_cb_t m_callback;

    lv_timer_t *m_timer { nullptr };
    static constexpr auto s_timer_period = 1000;
    bool m_next_char = true;
    std::string m_last;

    void create_textarea();
    void create_keyboard();
    void select();
    void unselect();
    const Keyboard_t *get_active_kbd();
    void process_ok();
    void process(std::string_view _c);

    static void timer_cb(lv_timer_t *_timer);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Keyboard(OSD *_parent);
    virtual ~OSD_Keyboard();
    void osd_keyboard(Osd_Keyboard_cb_t _keyboard_cb, lv_area_t _area, Satellite _satellite);
    void process_timer();
};

} // namespace mb
