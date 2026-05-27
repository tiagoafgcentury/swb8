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

class OSD_Password: public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Password_CB_t;

private:
    lv_obj_t *m_main_screen     { nullptr };
    lv_obj_t *m_text_area       { nullptr };
    lv_obj_t *m_text_content    { nullptr };
    lv_obj_t *m_warning_content { nullptr };

    struct passwd_Info_t
    {
        lv_obj_t *rect { nullptr };
        lv_obj_t *circle { nullptr };
    };

    /*
     * Senha de acesso
     */
    static constexpr uint16_t MAX_CHAR_PASSWD = 4;
    passwd_Info_t m_passwd_info[MAX_CHAR_PASSWD];
    std::string m_pass_buffer = "";

    char m_password[5] = "";
    static constexpr auto width = DISPLAY_WIDTH / 3;
    static constexpr auto heigth = DISPLAY_HEIGHT / 8;
    static constexpr auto x_pos = (DISPLAY_WIDTH - width) / 2;
    static constexpr auto y_pos = (DISPLAY_HEIGHT - heigth) / 2;

    void clear_password_fields();
    Password_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    static void timer_callback(lv_timer_t *_timer);
    static void clear_password_fields_callback(lv_timer_t *_timer);

public:
    OSD_Password(OSD *_parent);
    virtual ~OSD_Password();
    void show_password(Password_CB_t _callback, lv_obj_t *_bgd, const char *_default_passwd);
};

} // namespace mb
