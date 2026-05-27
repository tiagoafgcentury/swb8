#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_breadcrumb.h"

#include <memory>
#include <chrono>
#include <vector>

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "../../project_version.h"
#include "tasks/mb_task_cas.h"

namespace mb {

class OSD_Translate;

class OSD_Box_Info : public OSD, public Remote_Control_Handler
{
private:
    typedef std::function<void(void)> Box_Info_CB_t;

    lv_obj_t *m_bgd_info { nullptr };
    lv_obj_t *m_bgd_receiver_info { nullptr };
    lv_obj_t *m_bgd_cas_info { nullptr };

    lv_obj_t *m_link { nullptr };
    lv_obj_t *m_caid_number { nullptr };

    lv_obj_t *m_scua_number { nullptr };

    lv_obj_t *m_instruction { nullptr };
    lv_obj_t *m_instruction_box { nullptr };

    static constexpr auto instruction_x =  140;
    static constexpr auto instruction_y =  200;
    static constexpr auto instruction_w =  220;
    static constexpr auto instruction_h =  90;

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_fingerprint_cb;
    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

    lv_obj_t *m_qrcode { nullptr };

    Box_Info_CB_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

public:
    OSD_Box_Info(OSD *_parent);
    virtual ~OSD_Box_Info();

    virtual void show_box_info(lv_obj_t *_bgd, Box_Info_CB_t _callback);

    void process();
};

} // namespace mb
