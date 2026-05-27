#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_footer.h"
#include "mb_osd_terms_of_use.h"

#include <memory>
#include <chrono>
#include <vector>

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "../../project_version.h"
#include "tasks/mb_task_cas.h"

namespace mb {

class OSD_Translate;

class OSD_Ca_Info : public OSD, public Remote_Control_Handler
{
private:
    std::unique_ptr<OSD_Terms_of_Use> m_osd_terms_of_use;
    typedef std::function<void(void)> CA_Info_CB_t;

    lv_obj_t *m_main_screen { nullptr };

    lv_style_t *line_spacing { nullptr };
    CA_Info_CB_t m_callback;

    bool m_is_sky { false };

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_scua_cb;
    void set_cas_fingerprint(NAGRA_NUID_t _nuid, NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua, CAK_Version_t _cak_version,
                             Project_Info_t _project_info, Chipset_Type_t _chipset_type, Chipset_Revision_t _chipset_revision);

    void show_menu_terms_of_use_callback();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

    bool is_sky()
    {
        return m_is_sky;
    }

public:
    OSD_Ca_Info(OSD *_parent);
    virtual ~OSD_Ca_Info();

    virtual void show_box_info(lv_obj_t *m_bgd, CA_Info_CB_t _callback);

};

} // namespace mb
