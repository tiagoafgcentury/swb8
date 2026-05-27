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

class OSD_Set_Clock: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void()> Set_Clock_CB_t;

private:
    lv_style_t  m_style;

    lv_obj_t *m_parent_screen { nullptr };
    lv_obj_t *m_textarea { nullptr };
    lv_obj_t *m_wrong_date { nullptr };
    lv_timer_t *m_refresh_timer { nullptr };
    int m_timeout = 3;

    static constexpr auto offset_x = 1000 - 23 - 3;
    static constexpr auto offset_y = 114;

    MB_OSD_Keys m_options;
    static constexpr auto option_x = 990;
    static constexpr auto option_y = 116;
    static constexpr auto option_w = 190;
    static constexpr auto option_h = 50;
    static constexpr auto option_s = 180 - 116;

    enum class Mode_Active
    {
        Browsing,
        Editing_Date,
        Editing_Time,
    };
    Mode_Active m_mode_active = Mode_Active::Browsing;

    void create_textarea(uint16_t _idx);
    void edit_date(lv_obj_t *_obj, uint16_t _idx);
    void edit_clock(lv_obj_t *_obj, uint16_t _idx);
    void process_date_time(Remote_Control_Key _key);

    Set_Clock_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    static void refresh_cb(lv_timer_t *tm);

public:
    OSD_Set_Clock(OSD *_parent);
    virtual ~OSD_Set_Clock();
    void show_set_clock(Set_Clock_CB_t _callback, lv_obj_t *m_parent_screen);
    void refresh_progress();

};

} // namespace mb
