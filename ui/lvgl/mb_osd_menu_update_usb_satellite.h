#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_software_update.h"
#include "mb_osd_keys.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Menu_Update_Usb_Satellite: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(void)> Menu_Update_USB_Satellite_CB_t;

private:
    std::unique_ptr<OSD_Software_Update> m_software_update;

    static constexpr auto offset_x = 662;
    static constexpr auto offset_y = 200;
    static constexpr auto width = 300;
    static constexpr auto heigth = 360;

    // Linha vertical
    lv_obj_t *m_line_left = nullptr;
    lv_obj_t *m_line_right = nullptr;

    //Menu_Options
    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };
    lv_obj_t *m_passwd { nullptr };

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_x = 695;
    static constexpr auto button_y = offset_y;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 271 - offset_y;

    enum class Func_Active
    {
        USB,
        Satellite,
        COUNT
    };

    Menu_Update_USB_Satellite_CB_t m_callback;

    void show_menu_software_update_callback();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Update_Usb_Satellite(OSD *_parent);
    virtual ~OSD_Menu_Update_Usb_Satellite();
    void show_menu_update_usb_satellite(Menu_Update_USB_Satellite_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
