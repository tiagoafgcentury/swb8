#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Change_Password: public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Password_Callback_CB_t;

private:
    lv_style_t m_passwd_style;
    lv_obj_t *m_main_screen     { nullptr };
    lv_obj_t *m_warning_content { nullptr };

    struct passwd_Info_t
    {
        lv_obj_t *rect { nullptr };
        lv_obj_t *circle { nullptr };
    };

    /*
     * Senha de acesso
     */
    static constexpr uint16_t MAX_CHAR_PASSWD = 8;
    passwd_Info_t m_passwd_info[MAX_CHAR_PASSWD];
    std::string m_pass_buffer = "";

    static constexpr auto width = DISPLAY_WIDTH / 3;
    static constexpr auto height = DISPLAY_HEIGHT / 8;
    static constexpr auto x_pos = (DISPLAY_WIDTH - width) / 2;
    static constexpr auto y_pos = (DISPLAY_HEIGHT - height) / 2;

    static constexpr auto box_x = 0;
    static constexpr auto box_y = 40;
    static constexpr auto box_width = 80;
    static constexpr auto box_height = 80;
    static constexpr auto box_x_spacing = box_width + 20;
    static constexpr auto box_y_spacing = box_height + 80;

    void clear_password_fields();
    Password_Callback_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    void process_number_key(Event_Remote_Control _event);

    static void timer_callback(lv_timer_t *timer);
    static void clear_password_fields_callback(lv_timer_t *timer);

public:
    OSD_Change_Password(OSD *_parent);
    virtual ~OSD_Change_Password();
    void change_parental_password(Password_Callback_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
