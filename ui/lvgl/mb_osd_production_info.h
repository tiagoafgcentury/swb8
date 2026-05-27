#pragma once
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "tasks/mb_task_cas.h"
#include <memory>

namespace mb {

class OSD_Production_Info: public OSD//, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Production_Info_CB_t;

private:
    lv_obj_t *m_bgd_main { nullptr };
    lv_obj_t *m_caid { nullptr };
    lv_obj_t *m_scua { nullptr };

    static constexpr auto start_x = 90;
    static constexpr auto start_y = 360; //150;
    static constexpr auto start_w = 450;
    static constexpr auto start_h = 275;

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_fingerprint_cb;
    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

    Production_Info_CB_t m_callback;

    //protected:
    //virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Production_Info(OSD *_parent);
    virtual ~OSD_Production_Info();
    void show_production_info(Production_Info_CB_t _callback);
};

}
