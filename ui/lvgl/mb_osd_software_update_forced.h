
#pragma once

#include "mb_osd.h"
#include "mb_osd_software_update.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_lnbf_detection.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"

#include "../../project_version.h"
#include "tasks/mb_task_cas.h"

namespace mb {

class OSD_Translate;
class OSD_Choose_Home_Satellite;

class OSD_Software_Update_Forced : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> OSD_Software_Update_Forced_CB_t;

private:
    std::unique_ptr<OSD_Choose_Home_Satellite> m_osd_home_sat;
    std::shared_ptr<OSD_Lnbf_Detection> m_osd_lnbf_detection;
    std::unique_ptr<OSD_Software_Update> m_osd_software_update;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_bgd_main { nullptr };
    lv_obj_t *m_bgd_info { nullptr };
    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_title { nullptr };

    std::string_view title = tr(__Atualizacao_obrigatoria_de_software);
    static constexpr auto title_y = 0;
    static constexpr auto title_h = 53;

    lv_obj_t *m_subtitle { nullptr };
    std::string_view subtitle = tr(__Para_uma_experiencia_continua_atualize_software);
    static constexpr auto subtitle_y = 247 - offset_y;
    static constexpr auto subtitle_h = 33;

    static constexpr auto footer_y =      -40;
    lv_obj_t *m_caid_number { nullptr };
    lv_obj_t *m_scua_number { nullptr };
    lv_obj_t *m_nuid_number { nullptr };


    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_x = 429;
    static constexpr auto button_y = 382;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 652 - 409;

    enum class Func_Active
    {
        Satelite,
        USB,
        COUNT
    };

    void looking_for_software_update(bool update_mode);
    void call_lbnf_detection(Satellite satellite);
    void create_informations_screen();
    bool m_is_informations = false;

    lv_timer_t *m_timer_get_cas_fingerprint = nullptr;
    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_fingerprint_cb;
    void get_cas_fingerprint();
    static void get_cas_fingerprint_callback(lv_timer_t *_timer);
    void set_cas_fingerprint(NAGRA_NUID_t _nuid, NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

    OSD_Software_Update_Forced_CB_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Software_Update_Forced(OSD *_parent);
    virtual ~OSD_Software_Update_Forced();

    void show_menu_updt_forced(OSD_Software_Update_Forced_CB_t _callback);

};

} // namespace mb
