#pragma once

#include "mb_osd.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "tasks/mb_task_database.h"
#include "mb_osd_lnbf_detection.h"
#include "mb_osd_terms_of_use.h"
#include "mb_osd_keys.h"

namespace mb {

class OSD_Translate;

class OSD_Choose_Home_Satellite : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> OSD_Choose_Home_Satellite_CB_t;

private:
    std::shared_ptr<OSD_Lnbf_Detection> m_osd_lnbf_detection;
    std::unique_ptr<OSD_Terms_of_Use> m_osd_terms_of_use;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main               { nullptr };
    lv_obj_t *m_title               { nullptr };
    std::string_view title = tr(__Escolha_um_satelite);
    static constexpr auto title_y = 180 - offset_y;
    static constexpr auto title_h = 53;

    // Teclado
    MB_OSD_Keys m_sats;
    static constexpr auto sat_x = 409;
    static constexpr auto sat_y = 382;
    static constexpr auto sat_w = 220;
    static constexpr auto sat_h = 50;
    static constexpr auto sat_s = 652 - sat_x;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto btn_x = 530;
    static constexpr auto btn_y = 530;//570;
    static constexpr auto btn_w = 220;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_s = 0;
    std::array<std::string_view, 1> m_names
    {
        tr(__Voltar),
    };

    /*
     * Estrutura padrão de botões alinhados horizontalmente
     */
    enum class Selected_Area
    {
        Satellites,
        Button,
        COUNT
    };
    Selected_Area m_selected_area = Selected_Area::Satellites;

    static constexpr auto Satellite_Num = 2;
    std::array<std::string, Satellite_Num> m_func_names =
    {
        "SATHD Regional",
        "Nova Parabólica"
    };
    static constexpr auto func_name_y = 443 - offset_y;

    std::vector<Satellite> m_satellites;

    OSD_Choose_Home_Satellite_CB_t m_callback;

    void select_satellite();
    void call_lbnf_detection(Satellite satellite);

    void show_menu_lnbf_detection_callback(bool _result);
    void show_menu_terms_of_use_callback(bool _result, int selected_satellite);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Choose_Home_Satellite *s_instance;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Choose_Home_Satellite(OSD *_parent);
    virtual ~OSD_Choose_Home_Satellite();

    virtual void show_choose_home_satellite(OSD_Choose_Home_Satellite_CB_t _callback);
    virtual void hide_menu();
    void process_satellite_list();

    void process();
};

} // namespace mb
