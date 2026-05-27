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

#include <memory>
#include <functional>

namespace mb {

class OSD_Confirm_Default: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> Confirm_CB_t;

private:
    static constexpr auto width = DISPLAY_WIDTH / 2;
    static constexpr auto heigth = DISPLAY_HEIGHT / 2;
    static constexpr auto x_pos = (DISPLAY_WIDTH - width) / 2;
    static constexpr auto y_pos = (DISPLAY_HEIGHT - heigth) / 2;

    //Menu_Options
    lv_obj_t *m_main_box { nullptr };
    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_save_btn { nullptr };
    lv_obj_t *m_save_label { nullptr };
    lv_obj_t *m_cancel_btn { nullptr };
    lv_obj_t *m_cancel_label { nullptr };
    lv_obj_t *m_confirm_text { nullptr };

    static constexpr auto btn_width = width / 3;
    static constexpr auto save_x = (width - 2 * btn_width) / 3;
    static constexpr auto cancel_x = (2 * save_x) + btn_width;

    static constexpr auto btn_heigth = 40;
    static constexpr auto save_y = heigth - 100;
    static constexpr auto cancel_y = heigth - 100;

    void set_btn_active();
    Confirm_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    // Função ativada
    enum class Func_Status
    {
        Save,
        Cancel,
        COUNT
    };
    Func_Status m_status = Func_Status::Cancel;

public:
    OSD_Confirm_Default(OSD *_parent);
    virtual ~OSD_Confirm_Default();
    void show_confim_default(Confirm_CB_t _callback, lv_obj_t *_parent_screen);
};

} // namespace mb

