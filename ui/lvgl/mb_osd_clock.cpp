#include "mb_osd_clock.h"

#include <hal/mb_system.h>

#include "mb_osd.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

namespace mb {

OSD_Clock::~OSD_Clock()
{
    remove_clock();
}

void OSD_Clock::update_clock_cb(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Clock *>(lv_timer_get_user_data(_timer));
    auto system_time = System::get_system_time().to_local_time();

    if (thiz->m_clock)
    {
        lv_label_set_text_fmt(thiz->m_clock, "%.2d:%.2d", system_time.hour(), system_time.minute());
    }

    if (thiz->m_date)
    {
        lv_label_set_text_fmt(thiz->m_date, "%02d/%02d/%04d", system_time.day(), system_time.month(), system_time.year());
    }
}

void OSD_Clock::add_clock(lv_obj_t *_bgd, int _x, int _y)
{
    remove_clock();
    m_clock = OSD::set_label_text_static(_bgd, "", _x, _y, font_bold_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_clock);
    m_date = OSD::set_label_text_static(_bgd, "", _x, (_y + 45), font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_date);
    m_tmr_clock = lv_timer_create(update_clock_cb, 1000,  this);
    update_clock_cb(m_tmr_clock);
}

void OSD_Clock::remove_clock()
{
    DELETE_TIMER(m_tmr_clock);
    DELETE_OBJ(m_clock);
    DELETE_OBJ(m_date);
}

} // namespace mb
