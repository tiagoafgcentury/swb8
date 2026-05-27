#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Menu_Support: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(void)> Menu_Support_CB_t;

private:
    std::unique_ptr<Osd_Breadcrumb> m_bread_crumb;

    static constexpr auto offset_x = 400;
    static constexpr auto offset_y = 200;
    static constexpr auto width = 700;
    static constexpr auto heigth = 60 * 6;

    static constexpr std::string_view whastapp_number = "(12) 3042-2700";
    static constexpr std::string_view email_address = "atendimento@centurybr.com.br";
    static constexpr std::string_view link_century = MBGUI_CENTURY_HOMEPAGE;
    static constexpr std::string_view link_qrcode = "https://api.whatsapp.com/send?phone=+551230422700&text=Ol%C3%A1,%20preciso%20de%20atendimento.";

    lv_obj_t *m_main_box { nullptr };
    lv_obj_t *m_qrcode { nullptr };
    lv_obj_t *m_footer { nullptr };

    Menu_Support_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Support(OSD *_parent);
    virtual ~OSD_Menu_Support();
    void show_menu_support(Menu_Support_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
