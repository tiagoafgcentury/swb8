#pragma once

#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "mb_events.h"
#include "mb_osd.h"

#include <memory>
#include <map>

namespace mb {

class OSD_Draw_Closed_Caption: public OSD
{
private:
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT;

    lv_obj_t *m_main_box { nullptr };
    lv_timer_t *m_hide_timer { nullptr };
    std::array<lv_obj_t *, MBGUI_CC_MAX_LINES> m_lbl_line;

    static void hide_timer_callback(lv_timer_t *);

public:
    OSD_Draw_Closed_Caption(OSD *_parent);
    virtual ~OSD_Draw_Closed_Caption();

    void create_closed_caption();
    void print_closed_caption(const Event_CC &_event);
};

} // namespace mb
