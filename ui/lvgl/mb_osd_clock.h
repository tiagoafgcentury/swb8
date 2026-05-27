#pragma once

#include <lvgl/lvgl.h>

namespace mb {

class OSD_Clock
{
private:
    lv_obj_t *m_clock { nullptr };
    lv_obj_t *m_date { nullptr };
    lv_timer_t *m_tmr_clock { nullptr };

    static void update_clock_cb(lv_timer_t *_timer);

protected:
    void add_clock(lv_obj_t *_bgd, int _x, int _y);
    void remove_clock();

public:
    virtual ~OSD_Clock();
};

} // namespace mb
