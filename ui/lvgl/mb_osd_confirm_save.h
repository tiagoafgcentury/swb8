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

#include <memory>
#include <functional>

namespace mb {

class OSD_Confirm_Save: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> Confirm_Save_CB_t;

private:
    lv_obj_t *m_label { nullptr };

    lv_timer_t *m_refresh_timer { nullptr };
    lv_obj_t *m_timer_label { nullptr };
    lv_obj_t *m_timer_label2 { nullptr };
    int m_timeout = 10;

    static constexpr auto offset_x = 715;
    static constexpr auto offset_y = 114;

    MB_OSD_Keys m_options;
    static constexpr auto option_x = 789;
    static constexpr auto option_y = 393;
    static constexpr auto option_w = 160;
    static constexpr auto option_h = 50;
    static constexpr auto option_s = 962 - 789;
    std::array<std::string_view, 2> m_option_names
    {
        tr(__Sim),
        tr(__Nao),
    };

    Confirm_Save_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    static void refresh_cb(lv_timer_t *tm);

public:
    OSD_Confirm_Save(OSD *_parent);
    virtual ~OSD_Confirm_Save();
    void show_confirm_save(Confirm_Save_CB_t _callback, lv_obj_t *_parent_screen, std::string_view _text, bool _timer_enable);
    void refresh_progress();
};

} // namespace mb
