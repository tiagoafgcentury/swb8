#include "mb_osd_ca_info.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "common/mb_version.h"
#include "mb_terms_of_use.h"

#include "mb_events.h"
#include "mb_zone_id.h"
#include "../../project_version.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace mb {

OSD_Ca_Info::OSD_Ca_Info(OSD *_parent):
    OSD(_parent)
{
    auto config = Config::get_config();
    m_is_sky = config->selected_satellite_config().network_id == Network_Id_Sky;
}

OSD_Ca_Info::~OSD_Ca_Info()
{
    remove_focus();
}

void OSD_Ca_Info::show_menu_terms_of_use_callback()
{
    m_osd_terms_of_use.reset();
    using namespace std::placeholders;
    m_process_scua_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Ca_Info::set_cas_fingerprint, this, _1, _2, _3, _4, _5, _6, _7));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_scua_cb);
}

bool OSD_Ca_Info::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        case Remote_Control_Key::KEY_MENU:
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        case Remote_Control_Key::KEY_CC:
        {
            if (is_sky())
            {
                if (!m_osd_terms_of_use)
                {
                    m_osd_terms_of_use = std::make_unique<OSD_Terms_of_Use>(this);
                }

                m_osd_terms_of_use->show_menu_terms_of_use(std::bind(&OSD_Ca_Info::show_menu_terms_of_use_callback, this));
            }

            return true;
        }

        default:
            return true;
    }

    return false;
}

void OSD_Ca_Info::show_box_info(lv_obj_t *m_main, CA_Info_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    m_main_screen = m_main;

    if (is_sky())
    {
        auto footer = MB_OSD_Footer::draw(m_main, tr(__Ver_termos_condicoes_pressione_cc), -40);
        lv_obj_align(footer, LV_ALIGN_DEFAULT, 20, 330);
    }

    // Só desenha tela final quando receber scua
    using namespace std::placeholders;
    m_process_scua_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Ca_Info::set_cas_fingerprint, this, _1, _2, _3, _4, _5, _6, _7));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_scua_cb);
}

void OSD_Ca_Info::set_cas_fingerprint(NAGRA_NUID_t _nuid, NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua, CAK_Version_t _cak_version,
                                      Project_Info_t _project_info, Chipset_Type_t _chipset_type, Chipset_Revision_t _chipset_revision)
{
    // Busca dados que serão exibidos na tela
    std::string manufacture_date = {};
    if (access(MBGUI_MANUFACTURE_DATE_FILE, F_OK) == 0)
    {
        manufacture_date = cat(MBGUI_MANUFACTURE_DATE_FILE);
        manufacture_date.erase(std::remove(manufacture_date.begin(), manufacture_date.end(), '\n'), manufacture_date.end());
    }
    std::string last_activation_data = {};
    if (access(MBGUI_LAST_ACTIVATION_DATE_FILE, F_OK) == 0)
    {
        last_activation_data = cat(MBGUI_LAST_ACTIVATION_DATE_FILE);
        last_activation_data.erase(std::remove(last_activation_data.begin(), last_activation_data.end(), '\n'), last_activation_data.end());
    }

    auto software_version = MB_OSD_Version::get_full_version();

    uint32_t zone_id = Zone_ID::get_zone_id(Satellite_Operator::Claro);
    std::string zone_id_str = "Star One D2 - Zone ID: ";
    if (zone_id == 0)
    {
        zone_id_str += tr(__Nao_ativado);
    }
    else 
    {
        zone_id_str += std::to_string(zone_id);
    }

    uint32_t bouquet_id = Zone_ID::get_zone_id(Satellite_Operator::Sky) + 25000;
    std::string bouquet_id_str = "SKY B1 - Seg. Bouquet: ";
    if (bouquet_id == 25000)
    {
        bouquet_id_str += tr(__Nao_ativado);
    }
    else
    {
        bouquet_id_str += std::to_string(bouquet_id);
    }

    std::stringstream data;
    data << "CAID: " << _caid << "\n"
            "SCUA: " << _scua << "\n"
            "NUID: " + _nuid + "\n"
         << zone_id_str << "\n"
         << bouquet_id_str << "\n"
         << tr(__Modelo) << ": " MBGUI_PRODUCT_NAME " " MBGUI_MODEL_NAME "\n"
         << tr(__CAK_Version) << ": " << _cak_version << "\n"
         << tr(__Project_Info) << ": " << _project_info << "\n"
         << tr(__Chipset) << ": " << _chipset_type << " Rev. " << _chipset_revision << "\n"
         << tr(__Data_de_fabricacao) << ": " << manufacture_date << "\n"
         << tr(__Versao_do_Software) << ": " << software_version << "\n"
         << tr(__Data_da_ultima_ativacao) << ": " << last_activation_data << "\n";

    if (is_sky())
    {
        Terms_File::App_Terms_of_Use terms;
        std::string terms_conditions_date = {};
        if (access(MBGUI_TERMS_CONDITIONS_DATE_FILE, F_OK) == 0)
        {
            terms_conditions_date = cat(MBGUI_TERMS_CONDITIONS_DATE_FILE);
        }
        data << tr(__Termos_e_condicoes_de_uso) << ": " << terms.version << "\n"
             << tr(__Aceite_dos_termos_de_uso) << ": " << terms_conditions_date << "\n";
    }

    DEBUG_MSG(OSD, DEBUG, data.str() << "\n");
    set_label_text(m_main_screen, data.str(), 20, 0, font_20, OSD_COLOR_WHITE);
}

void OSD_Ca_Info::got_focus()
{
}

} // namespace mb
