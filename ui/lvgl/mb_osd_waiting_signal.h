#pragma once

#include "mb_menu_data.h"
#include "mb_remote_control_handler.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"

#include <memory>

namespace mb {

class OSD_Waiting_Signal : public OSD, public OSD_Clock
{
private:
    static constexpr uint16_t MAX_WIDTH_WAITING_SIGNAL = 480;
    static constexpr uint16_t MAX_HEIGHT_WAITING_SIGNAL = 240;
    static constexpr uint16_t OFFSET_X = 150;
    static constexpr uint16_t OFFSET_Y = 100;
    static constexpr uint16_t MAX_RANGE_WIDTH = 500;
    static constexpr uint16_t MAX_RANGE_HEIGHT = 300;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_bgd_wfs { nullptr };

    lv_timer_t *m_tmr_wait_signal { nullptr };

public:
    OSD_Waiting_Signal(OSD *_parent);
    virtual ~OSD_Waiting_Signal();

    void waiting_signal(lv_obj_t *_bgd_main);
    void cleanup_screen_waiting_signal();
    void update_waiting_signal();
};

}
