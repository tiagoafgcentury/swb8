#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_confirm_save.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_select_satellite.h"
#include "mb_osd_select_switch.h"
#include "mb_osd_password.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_instala_facil.h"
#include "mb_osd_keys.h"
//#include "mb_osd_channel_list_update.h"
#include "mb_osd_menu_update_usb_satellite.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Menu_Update: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(void)> Menu_Update_CB_t;

private:
    std::unique_ptr<OSD_Menu_Update_Usb_Satellite> m_update_usb_satellite;

    static constexpr auto offset_x = 400;
    static constexpr auto offset_y = 200;
    static constexpr auto width = 300;
    static constexpr auto heigth = 60 * 6;

    // Linha vertical
    lv_obj_t *m_line_left = nullptr;
    lv_obj_t *m_line_right = nullptr;

    //Menu_Options
    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_box { nullptr };
    lv_obj_t *m_bgd_main { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };
    lv_obj_t *m_passwd { nullptr };
    lv_obj_t *m_footer { nullptr };

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_x = 429;
    static constexpr auto button_y = offset_y;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 271 - offset_y;

    static constexpr auto footer_y =      -40;
    bool m_is_channel_list_screen = false;

    enum class Func_Active
    {
        Software,
        //List,
        COUNT
    };

    Menu_Update_CB_t m_callback;

    void show_menu_update_usb_satellite_callback();
    void create_satellite_list_unavailable();
    void hide_channel_list_screen();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Update(OSD *_parent);
    virtual ~OSD_Menu_Update();
    void show_menu_update(Menu_Update_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
