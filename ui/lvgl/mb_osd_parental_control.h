#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Parental_Control: public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Parental_Callback_CB_t;

private:
    Parental_Callback_CB_t m_callback;

    lv_obj_t *m_main_screen     { nullptr };
    lv_obj_t *m_warning_content { nullptr };

    static constexpr auto width = DISPLAY_WIDTH / 3;
    static constexpr auto height = DISPLAY_HEIGHT / 8;
    static constexpr auto x_pos = (DISPLAY_WIDTH - width) / 2;
    static constexpr auto y_pos = (DISPLAY_HEIGHT - height) / 2;
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 0;

    uint8_t m_parental_rating = 0;

    // Botão da tela de detecção de LNBF
    MB_OSD_Keys m_keys;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto button_x = 20;
    static constexpr auto button_y = 0;
    static constexpr auto spacing = 180 - 114;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    void save_configuration();

public:
    OSD_Parental_Control();
    virtual ~OSD_Parental_Control();
    void config_parental_control(Parental_Callback_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
