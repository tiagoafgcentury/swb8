#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_keys.h"
#include "mb_osd_breadcrumb.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Set_Timezone: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(Timezone_Mode)> Set_Timezone_CB_t;

private:
    Timezone_Mode m_timezone_mode;

    static constexpr auto offset_x = 1000 - 23 - 3;
    static constexpr auto offset_y = 114;

    MB_OSD_Keys m_options;
    static constexpr auto option_x = 990;
    static constexpr auto option_y = 116;
    static constexpr auto option_w = 190;
    static constexpr auto option_h = 50;
    static constexpr auto option_s = 180 - 116;

    void process_timezone();
    Set_Timezone_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    static void refresh_cb(lv_timer_t *_tm);

public:
    OSD_Set_Timezone(OSD *_parent);
    virtual ~OSD_Set_Timezone();
    void show_set_timezone(Set_Timezone_CB_t _callback, lv_obj_t *_parent_screen, Timezone_Mode _timezone_mode);
    void refresh_progress();
};

} // namespace mb
