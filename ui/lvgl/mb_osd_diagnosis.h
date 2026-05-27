#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_footer.h"
#include "mb_osd_terms_of_use.h"

#include <memory>
#include <vector>

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "../../project_version.h"
#include "tasks/mb_task_cas.h"

namespace mb {

class OSD_Translate;

class OSD_Diagnosis : public OSD, public Remote_Control_Handler
{
private:
    typedef std::function<void(void)> Diagnostic_CB;
    Diagnostic_CB m_callback;

    lv_obj_t *m_main { nullptr };
    static constexpr uint16_t main_x = 0;
    static constexpr uint16_t main_y = 130;
    static constexpr uint16_t main_w = DISPLAY_WIDTH;
    static constexpr uint16_t main_h = DISPLAY_HEIGHT - main_y;

    lv_obj_t *m_box_1 { nullptr };
    static constexpr uint16_t box_1_x = 186;
    static constexpr uint16_t box_1_y = 0;
    static constexpr uint16_t box_1_w = 446;
    static constexpr uint16_t box_1_h = 76;

    lv_obj_t *m_box_2 { nullptr };
    static constexpr uint16_t box_2_x = 644;
    static constexpr uint16_t box_2_y = 0;
    static constexpr uint16_t box_2_w = 453;
    static constexpr uint16_t box_2_h = 113;

    lv_obj_t *m_box_3 { nullptr };
    static constexpr uint16_t box_3_x = 186;
    static constexpr uint16_t box_3_y = 217 - main_y;
    static constexpr uint16_t box_3_w = box_1_w;
    static constexpr uint16_t box_3_h = 250+20;

    lv_obj_t *m_box_4 { nullptr };
    static constexpr uint16_t box_4_x = 644;
    static constexpr uint16_t box_4_y = 254 - main_y;
    static constexpr uint16_t box_4_w = 453;
    static constexpr uint16_t box_4_h = 214+20;

    lv_obj_t *m_snr_data { nullptr };
    lv_timer_t *m_tmr_signal { nullptr };
    static constexpr uint16_t snr_x = 930;
    static constexpr uint16_t snr_y = 580 - main_y;

    lv_obj_t *m_svr_data { nullptr };

    // Barra de progresso
    lv_obj_t *m_slider_label;
    static constexpr auto slider_label_x = 507;
    static constexpr auto slider_label_y = snr_y;
    lv_obj_t *m_slider;
    static constexpr auto slider_y = 558 - main_y;
    static constexpr auto slider_w = 910;
    static constexpr auto slider_x = 186;
    static constexpr auto slider_h = 20;

    lv_obj_t *m_caid { nullptr };
    lv_obj_t *m_scua { nullptr };

    struct Satellite_Info_t
    {
        int frequency;
        int symbol_rate;
        char polarity[2]; // 'H' or 'V'
        std::string name;
        std::string switch_type;
        std::string switch_position;
    };

    Satellite_Operator m_oper = Satellite_Operator::Claro;

    Satellite_Info_t m_satellite_info;
    std::vector<Transponder> m_tps;
    Service_ID_t m_service_id;
    Transponder_Id m_transponder_id;
    bool m_stop_player = false;

    static void update_antenna_signal_cb(lv_timer_t *_timer);
    void update_antenna_signal();
    void create_quality_bar();
    void print_satellite();
    std::tuple<int, int> count_tvs_and_radios();

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_scua_cb;
    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Diagnosis *s_instance;

public:
    OSD_Diagnosis(OSD *_parent);
    virtual ~OSD_Diagnosis();

    virtual void show_diagnostics(Diagnostic_CB _callback, bool _stop_player = true);
};

} // namespace mb
