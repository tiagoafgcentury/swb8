#include "mb_osd_ird_fingerprint.h"

#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <utility>

namespace mb {

OSD_Ird_Fingerprint::OSD_Ird_Fingerprint(Event_CAS_Ird_Fingerprint _event):
    OSD(nullptr),
    m_event(std::move(_event))
{
    m_remaining_repetitions = m_event.repetition_nb == 0 ? 1 : m_event.repetition_nb;
}

OSD_Ird_Fingerprint::~OSD_Ird_Fingerprint()
{
    DELETE_TIMER(m_hide_timer);
    DELETE_TIMER(m_repeat_timer);
    hide_current();
}

void OSD_Ird_Fingerprint::start()
{
    show_once();
}

lv_color_t OSD_Ird_Fingerprint::map_color(uint8_t _color)
{
    switch(_color)
    {
        case 0: return lv_color_make(0, 0, 0);
        case 1: return lv_color_make(255, 255, 255);
        case 2: return lv_color_make(255, 0, 0);
        case 3: return lv_color_make(0, 0, 255);
        case 4: return lv_color_make(0, 160, 0);
        case 5: return lv_color_make(255, 255, 0);
        default: return lv_color_make(0, 0, 0);
    }
}

bool OSD_Ird_Fingerprint::is_transparent(uint8_t _color)
{
    return _color == 7;
}

int OSD_Ird_Fingerprint::percent_to_x(uint8_t _percent, int _width)
{
    auto percent = _percent == 0 ? static_cast<uint8_t>(std::rand() % 101) : _percent;
    return std::clamp((DISPLAY_WIDTH - _width) * static_cast<int>(percent) / 100, 0, DISPLAY_WIDTH - _width);
}

int OSD_Ird_Fingerprint::percent_to_y(uint8_t _percent, int _height)
{
    auto percent = _percent == 0 ? static_cast<uint8_t>(std::rand() % 101) : _percent;
    return std::clamp((DISPLAY_HEIGHT - _height) * static_cast<int>(percent) / 100, 0, DISPLAY_HEIGHT - _height);
}

std::string OSD_Ird_Fingerprint::build_message() const
{
    std::ostringstream out;

    if(m_event.show_time)
    {
        auto now = std::time(nullptr);
        auto local = std::localtime(&now);

        if(local)
        {
            char buffer[16];
            std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
            out << buffer << ' ';
        }
    }

    if(m_event.show_stb_sn and not m_event.stb_sn.empty())
    {
        out << m_event.stb_sn << ' ';
    }

    out << m_event.ua;
    return out.str();
}

void OSD_Ird_Fingerprint::show_once()
{
    hide_current();

    if(m_event.ua.empty())
    {
        return;
    }

    constexpr int box_width = 520;
    constexpr int box_height = 64;
    auto x = percent_to_x(m_event.location_x, box_width);
    auto y = percent_to_y(m_event.location_y, box_height);

    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), x, y, box_width, box_height, map_color(m_event.bg_color));
    lv_obj_set_style_bg_opa(m_main, is_transparent(m_event.bg_color) ? LV_OPA_TRANSP : LV_OPA_90, DEFAULT_SELECTOR);
    lv_obj_set_style_radius(m_main, 0, DEFAULT_SELECTOR);

    auto label = set_label_text(m_main, build_message(), 0, 0, font_25, map_color(m_event.fg_color));
    lv_obj_set_style_text_opa(label, is_transparent(m_event.fg_color) ? LV_OPA_TRANSP : LV_OPA_COVER, DEFAULT_SELECTOR);
    lv_obj_set_width(label, box_width - 20);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    auto display_time = m_event.display_time_ms == 0 ? 1000 : m_event.display_time_ms;
    auto repetition_time = static_cast<uint32_t>(m_event.repetition_time_s) * 1000;

    if(repetition_time > 0 and display_time > repetition_time)
    {
        display_time = repetition_time;
    }

    m_remaining_repetitions--;
    m_hide_timer = lv_timer_create(hide_timer_cb, display_time, this);
    lv_timer_set_repeat_count(m_hide_timer, 1);
    lv_timer_set_auto_delete(m_hide_timer, true);
}

void OSD_Ird_Fingerprint::hide_current()
{
    if(m_main)
    {
        lv_obj_delete(m_main);
        m_main = nullptr;
    }
}

void OSD_Ird_Fingerprint::schedule_next()
{
    if(m_remaining_repetitions == 0)
    {
        return;
    }

    auto display_time = m_event.display_time_ms == 0 ? 1000 : m_event.display_time_ms;
    auto repetition_time = static_cast<uint32_t>(m_event.repetition_time_s) * 1000;
    auto delay = repetition_time > display_time ? repetition_time - display_time : 1;

    m_repeat_timer = lv_timer_create(repeat_timer_cb, delay, this);
    lv_timer_set_repeat_count(m_repeat_timer, 1);
    lv_timer_set_auto_delete(m_repeat_timer, true);
}

void OSD_Ird_Fingerprint::hide_timer_cb(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Ird_Fingerprint *>(lv_timer_get_user_data(_timer));
    thiz->m_hide_timer = nullptr;
    thiz->hide_current();
    thiz->schedule_next();
}

void OSD_Ird_Fingerprint::repeat_timer_cb(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Ird_Fingerprint *>(lv_timer_get_user_data(_timer));
    thiz->m_repeat_timer = nullptr;
    thiz->show_once();
}

} // namespace mb
