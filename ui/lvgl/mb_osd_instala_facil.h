#pragma once

#include "mb_osd.h"
#include "mb_osd_software_update.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"
#include "mb_remote_control_handler.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_lnbf_detection.h"
#include "mb_osd_terms_of_use.h"
#include "mb_osd_lnbf_detection.h"
#include "mb_osd_lnbf_snr.h"

namespace mb {

class OSD_Translate;
class OSD_Choose_Home_Satellite;

class OSD_Instala_Facil : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> OSD_Instala_Facil_CB_t;
    std::unique_ptr<MB_Detect_Lnbf> m_detect_lnbf;
    std::shared_ptr<OSD_Lnbf_Snr> m_osd_lnbf_snr;

private:
    std::unique_ptr<OSD_Choose_Home_Satellite> m_osd_home_sat;
    std::unique_ptr<OSD_Software_Update> m_osd_software_update;
    std::unique_ptr<OSD_Activate> mb_osd_activate;
    std::unique_ptr<OSD_Channel_List_Update> m_osd_channel_list_update;
    std::unique_ptr<OSD_Terms_of_Use> m_osd_terms_of_use;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_bgd_main { nullptr };
    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_title { nullptr };
#ifdef MBGUI_USE_RLOTTIE
    lv_obj_t *m_sat_logo { nullptr };
#endif

    static constexpr auto title_y = 283 - offset_y;
    static constexpr auto title_h = 53;

    lv_obj_t *m_subtitle { nullptr };
    static constexpr auto subtitle_y = 350 - offset_y;
    static constexpr auto subtitle_h = 33;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_x = 429;
    static constexpr auto button_y = 570;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 652 - 409;
    std::array<std::string_view, 2> m_names
    {
        tr(__Voltar),
        tr(__Proximo),
    };

    enum class Status
    {
        Detect,
        Fail,
        Success,
        Terms,
        COUNT
    };
    Status m_status = Status::Detect;

    enum class Satellite
    {
        Starone_D2,
        Sky_B1,
        None,
        COUNT
    };
    Satellite m_satellite = Satellite::None;
    std::array<std::string_view, 2> m_satellites
    {
        "Star One D2",
        "Sky B1",
    };

    Osd_Breadcrumb::Position m_breadcrumb_pos;

    void looking_for_channel_list_update();

    void show_menu_channel_list_update_callback(bool _result);
    void show_menu_activate_callback(bool _result);
    void return_after_channel_list_screen(bool _result);

    bool m_restart = true;
    OSD_Instala_Facil_CB_t m_callback;
    void lnbf_detection_callback(bool _result, Transponder_Id _tp);
    void start_lnbf_detection();
    void osd_lnbf_snr_callback(bool _result);
    void start_osd_lnbf_snr();
    void process_ok_key();
    void terms_of_use();
    void terms_of_use_callback(bool _result);
    void clear_screen();
    void create_main_screen();

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
    OSD_Instala_Facil(OSD *_parent);
    virtual ~OSD_Instala_Facil();

    virtual void show_menu_easy_install(OSD_Instala_Facil_CB_t _callback, bool restart = true);
};

} // namespace mb
