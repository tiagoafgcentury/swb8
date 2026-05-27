#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_keys.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_footer.h"
#include "mb_osd_diagnosis.h"

#include <memory>
#include <chrono>
#include <vector>

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "../../project_version.h"
#include "tasks/mb_task_cas.h"
#include "mb_osd_box_info.h"
#include "mb_osd_ca_info.h"

namespace mb {

class OSD_Translate;

class OSD_Menu_Info : public OSD, public Remote_Control_Handler
{
private:
    std::unique_ptr<OSD_Box_Info> mb_osd_box_info;
    std::unique_ptr<OSD_Ca_Info> mb_osd_ca_info;
    std::unique_ptr<OSD_Diagnosis> mb_osd_diagnostics;
    typedef std::function<void(void)> Menu_Info_CB_t;

    Service *m_service;
    Viewer_Channel_t m_viewer_channel = 0;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_bgd_info { nullptr };
    lv_obj_t *m_left_line { nullptr };
    lv_obj_t *m_rigth_line { nullptr };
    lv_obj_t *m_bckg_fade { nullptr };
    lv_obj_t *m_footer_main { nullptr };
    lv_obj_t *m_footer_opt { nullptr };

    lv_obj_t *m_info_box { nullptr };
    static constexpr auto width = 300;
    static constexpr auto heigth = 360;
    static constexpr uint16_t info_heigth = 360;
    static constexpr uint16_t info_width = 460;
    static constexpr uint16_t info_x = 735;
    static constexpr uint16_t info_y = 200;

    lv_obj_t *m_ca_info { nullptr };
    static constexpr uint16_t ca_info_h = 450;
    static constexpr uint16_t ca_info_w = 500;//550;
    static constexpr uint16_t ca_info_x = 735;
    static constexpr uint16_t ca_info_y = 116;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto offset_x = 430;
    static constexpr auto offset_y = 200;
    static constexpr auto button_x = 460;
    static constexpr auto button_y = offset_y;
    static constexpr auto button_w = 270;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 271 - offset_y;

    // Função ativada
    enum class Func_Active
    {
        Receiver,
        CA,
        Diagnostic,
    };
    Func_Active m_func_active = Func_Active::Receiver;

    void update_breadcrumb();
    void process_receiver();
    void process_receiver_callback();

    void process_ca();
    void process_ca_callback();

    void process_diagnostic();
    void process_diagnostic_callback();

    Menu_Info_CB_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

public:
    OSD_Menu_Info(OSD *_parent);
    virtual ~OSD_Menu_Info();

    virtual void show_menu_info(Menu_Info_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
