#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_menu_satellite.h"
#include "mb_osd_menu_config.h"
#include "mb_osd_menu_update.h"
#include "mb_osd_menu_suporte.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"
#include "mb_osd_menu_info.h"
#include "mb_osd_box_info.h"
#include "mb_osd_terms_of_use.h"

#include <memory>
#include <chrono>
#include <vector>

namespace mb {

class OSD_Translate;

class OSD_Menu_Ajustes : public OSD, public Remote_Control_Handler
{
private:
    std::unique_ptr<OSD_Menu_Config> m_config;
    std::unique_ptr<OSD_Menu_Satellite> m_menu_satellite;
    std::unique_ptr<OSD_Menu_Update> m_update;
    std::unique_ptr<OSD_Menu_Info> mb_osd_menu_info;
    std::unique_ptr<OSD_Menu_Support> m_support;
    std::unique_ptr<OSD_Terms_of_Use> m_osd_terms_of_use;

    typedef std::function<void(bool)> OSD_Menu_Ajustes_CB_t;

    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bdg_box { nullptr };
    static constexpr auto main_x = 150;
    static constexpr auto main_y = 0;
    static constexpr auto main_w = DISPLAY_WIDTH - main_x;
    static constexpr auto main_h = DISPLAY_HEIGHT - main_y;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto offset_x = main_x;
    static constexpr auto offset_y = 0;
    static constexpr auto button_x = 163;
    static constexpr auto button_y = 198;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 271 - 198;

    // Função ativada
    enum class Func_Active
    {
        Satellite,
        Adjust,
        Updates,
        Info,
        Support,
        COUNT
    };
    Func_Active m_function_active = Func_Active::Satellite;

    std::array<std::string_view, static_cast<size_t>(Func_Active::COUNT)> m_icons =
    {
        LOGO_MENU_INICIO,
        LOGO_MENU_CANAIS_DE_TV,
        LOGO_MENU_CANAIS_DE_RADIOS,
        LOGO_MENU_MULTIMIDIA,
        LOGO_MENU_AJUSTES
    };

    lv_obj_t *m_bgd_fade { nullptr };
    OSD_Menu_Ajustes_CB_t m_callback;

    void init_breadcrumb();
    void update_breadcrumb();

    void osd_menu_satellite_callback(bool _result);
    void osd_menu_info_callback();
    void osd_menu_config_callback(bool _ok);
    void osd_menu_support_callback();
    void osd_menu_updates_callback();
    void init_names();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

public:
    OSD_Menu_Ajustes(OSD *_parent);
    virtual ~OSD_Menu_Ajustes();

    virtual void show_menu_ajustes(OSD_Menu_Ajustes_CB_t _callback);

    void process();
};

} // namespace mb
