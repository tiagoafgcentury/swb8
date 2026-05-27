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
class OSD_Menu_Plus;

class OSD_Program_Reminder: public OSD
{

private:

    typedef std::function<void(void)> osd_program_reminder_cb;

    std::unique_ptr<OSD_Message_Box> m_message_box;
    std::unique_ptr<OSD_Menu_Plus> m_plus;

    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT;
    static constexpr auto max_time_exit = 10;

    lv_timer_t *m_exit_timer{ nullptr };
    uint8_t m_count = max_time_exit;
    lv_timer_t *m_change_channel_exit_timer{ nullptr };


    ScheduleEntry m_s_entry;

    osd_program_reminder_cb m_callback;
    void record_schedule_start_callback(bool _result);
    void remember_schedule_start_callback();
    void change_channel();
    static void process_exit_cb(lv_timer_t *tm);
    static void process_change_channel_cb(lv_timer_t *tm);

public:
    OSD_Program_Reminder(OSD *_parent);
    virtual ~OSD_Program_Reminder();

    void show_program_reminder(osd_program_reminder_cb _callback, ScheduleEntry _s_entry);
};

} // namespace mb

