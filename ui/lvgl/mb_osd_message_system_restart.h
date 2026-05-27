#pragma once

#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"


#include <memory>
#include <map>

namespace mb {

class OSD_Message_Box;

class OSD_Message_System_Restart: public OSD
{

private:

    typedef std::function<void(void)> osd_message_system_restart_cb;

    std::unique_ptr<OSD_Message_Box> m_message_box;

    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT;
    static constexpr auto max_time_exit = 10;

    lv_timer_t *m_exit_timer{ nullptr };
    uint8_t m_count = max_time_exit;

    osd_message_system_restart_cb m_callback;
    void system_restart_callback();
    static void process_exit_cb(lv_timer_t *tm);

public:
    OSD_Message_System_Restart(OSD *_parent);
    virtual ~OSD_Message_System_Restart();

    void show_message_system_restart(osd_message_system_restart_cb _callback);
};

} // namespace mb

