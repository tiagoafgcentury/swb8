#include "mb_osd_choose_home_satellite.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_osd.h"

#include <lvgl.h>


namespace mb {

OSD_Choose_Home_Satellite::OSD_Choose_Home_Satellite(OSD *_parent):
    OSD(_parent),
    m_sats(offset_x, offset_y, sat_w, sat_h, sat_s, sat_x, sat_y),
    m_keys(offset_x, offset_y, btn_w, btn_h, btn_s, btn_x, btn_y)
{
    for (const auto &name : m_names)
    {
        m_keys.add_label(name);
    }
}

OSD_Choose_Home_Satellite::~OSD_Choose_Home_Satellite()
{
    DELETE_OBJ(m_main);
    Osd_Breadcrumb::s_instance.remove_name();
}

bool OSD_Choose_Home_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(std::bind(m_callback, false));
            return true;

        case Remote_Control_Key::KEY_OK:
        {
            if (m_selected_area == Selected_Area::Button)
            {
                Task::post_event(std::bind(m_callback, false));
            }
            else
            {
                select_satellite();
            }

            return true;
        }

        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
            if (m_selected_area == Selected_Area::Satellites)
            {
                m_sats.next();
            }

            return true;

        case Remote_Control_Key::KEY_CHUP:
        case Remote_Control_Key::KEY_CHDOWN:
            if (m_selected_area == Selected_Area::Button)
            {
                m_keys.unselect();
                m_selected_area = Selected_Area::Satellites;
                m_sats.select();
            }
            else
            {
                m_sats.unselect();
                m_selected_area = Selected_Area::Button;
                m_keys.select();
            }

            return true;

        default:
            return true;
    }

    return true;
}

void OSD_Choose_Home_Satellite::show_menu_lnbf_detection_callback(bool _result)
{
    lv_obj_remove_flag(m_main, LV_OBJ_FLAG_HIDDEN);
    m_osd_lnbf_detection.reset();

    if (_result)
    {
        Task::post_event(std::bind(m_callback, true));
    }
}

void OSD_Choose_Home_Satellite::call_lbnf_detection(Satellite satellite)
{
    if (!m_osd_lnbf_detection)
    {
        m_osd_lnbf_detection = std::make_shared<OSD_Lnbf_Detection>(this);
        Task_OSD::set_lnbf_detection(m_osd_lnbf_detection);
    }

    m_osd_lnbf_detection->show_menu_lnbf_detection(std::bind(&OSD_Choose_Home_Satellite::show_menu_lnbf_detection_callback, this, std::placeholders::_1), satellite);
}

void OSD_Choose_Home_Satellite::show_menu_terms_of_use_callback(bool _result, int selected_satellite)
{
    if (_result)
    {
        auto satellite = m_satellites[selected_satellite];
        call_lbnf_detection(satellite);
    }

    lv_obj_remove_flag(m_main, LV_OBJ_FLAG_HIDDEN);
    m_osd_terms_of_use.reset();
}

void OSD_Choose_Home_Satellite::select_satellite()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Choose_Home_Satellite::select_satellite()");
    lv_obj_add_flag(m_main, LV_OBJ_FLAG_HIDDEN);
    auto selected_satellite = m_sats.get_selected();
    auto satellite = m_satellites[selected_satellite];
    Task::post_event_player_stop();

    if (selected_satellite == 1)
    {
        auto config = Config::get_config();
        config->set_satellite_config(Network_Id_Sky);

        if (!m_osd_terms_of_use)
        {
            m_osd_terms_of_use = std::make_unique<OSD_Terms_of_Use>(this);
        }

        m_osd_terms_of_use->show_menu_terms_of_use(std::bind(&OSD_Choose_Home_Satellite::show_menu_terms_of_use_callback, this, std::placeholders::_1, selected_satellite));
    }
    else
    {
        auto config = Config::get_config();
        config->set_satellite_config(Network_Id_Claro);
        call_lbnf_detection(satellite);
    }
}

void OSD_Choose_Home_Satellite::show_choose_home_satellite(OSD_Choose_Home_Satellite_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    //MainMenu
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.draw();
    // Título
    m_title = set_label_text_static(m_main, title, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, 0);
    set_label_text(m_main, "SATHD Regional", 448, 440 - offset_y, font_20, OSD_COLOR_WHITE);
    set_label_text(m_main, "Nova Parabólica", 691, 440 - offset_y, font_20, OSD_COLOR_WHITE);
    // Cria rodapé
    MB_OSD_Footer::draw(m_main, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -40);
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Escolha_um_satelite));
    // Carrega lista de satélites
    process_satellite_list();
}

void OSD_Choose_Home_Satellite::hide_menu()
{
    remove_focus();
    Task::post_event(std::bind(m_callback, false));
}

void OSD_Choose_Home_Satellite::got_focus()
{
}

void OSD_Choose_Home_Satellite::process_satellite_list()
{
    auto config = Config::get_config();
    auto local_sats = config->get_satellite_list();
    DEBUG_MSG(OSD, DEBUG, "Lista de satélites:\n");

    for (const auto &s : local_sats)
    {
        if (s.is_mandatory)
        {
            m_satellites.push_back(s);
            m_sats.add_label(s.name);
            DEBUG_MSG_NL(OSD, DEBUG, s.name << "\n");
        }
    }

    if (m_satellites.size() != 2)
    {
        DEBUG_MSG(OSD, ERROR, "Lista de satélites com erros!\n");
        return;
    }

    // Desenha teclado
    m_sats.set_background(m_main);
    m_sats.draw();
    m_sats.select();
}

} // namespace mb
