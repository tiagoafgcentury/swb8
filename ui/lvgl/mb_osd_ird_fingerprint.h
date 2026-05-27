#pragma once

#include "mb_events.h"
#include "mb_osd.h"

#include <lvgl.h>

#include <string>

namespace mb {

class OSD_Ird_Fingerprint final: public OSD
{
private:
    Event_CAS_Ird_Fingerprint m_event;
    lv_obj_t *m_main { nullptr };
    lv_timer_t *m_hide_timer { nullptr };
    lv_timer_t *m_repeat_timer { nullptr };
    uint8_t m_remaining_repetitions { 0 };

    static void hide_timer_cb(lv_timer_t *_timer);
    static void repeat_timer_cb(lv_timer_t *_timer);

    static lv_color_t map_color(uint8_t _color);
    static bool is_transparent(uint8_t _color);
    static int percent_to_x(uint8_t _percent, int _width);
    static int percent_to_y(uint8_t _percent, int _height);

    std::string build_message() const;
    void show_once();
    void hide_current();
    void schedule_next();

public:
    explicit OSD_Ird_Fingerprint(Event_CAS_Ird_Fingerprint _event);
    ~OSD_Ird_Fingerprint();

    void start();
};

} // namespace mb
