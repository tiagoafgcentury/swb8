#include "mb_osd_instala_facil.h"
#include "mb_osd_choose_home_satellite.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include "mb_zone_id.h"
#include "mb_terms_of_use.h"
#include "mb_osd_fonts.h"
#include "mb_osd_message_box.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "common/mb_state_file.h"
#include "mb_main.h"


#include <lvgl.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <filesystem>
#include <fstream>

namespace mb {

OSD_Instala_Facil::OSD_Instala_Facil(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    for (const auto &name : m_names)
    {
        m_keys.add_label(name);
    }

    m_breadcrumb_pos = Osd_Breadcrumb::s_instance.get_position();
}

OSD_Instala_Facil::~OSD_Instala_Facil()
{
    DELETE_OBJ(m_bgd_main);
    clear_screen();
    Osd_Breadcrumb::s_instance.clear_from_position(m_breadcrumb_pos);
    remove_focus();
}

bool OSD_Instala_Facil::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if (m_blocked)
    {
        return true;
    }

    switch (_event.key)
    {

        case Remote_Control_Key::KEY_OK:
        {
            auto pressed = m_keys.get_selected();
            if (pressed == 1)
            {
                DEBUG_MSG(OSD, DEBUG, "KEY_OK\n");
                process_ok_key();
            }
            else
            {
                goto EXIT_INSTALA_FACIL;
            }
        }
        break;

        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
        {
            m_keys.next();
        }
        break;

        case Remote_Control_Key::KEY_VOLTAR:
            goto EXIT_INSTALA_FACIL;

        default:
            return true;
    }

    return true;
EXIT_INSTALA_FACIL:
    Task::post_event(std::bind(m_callback, false));
    return true;
}

void OSD_Instala_Facil::show_menu_easy_install(OSD_Instala_Facil_CB_t _callback, bool restart)
{
    m_restart = restart;
    set_focus();
    m_callback = _callback;
    create_main_screen();
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.draw();
    m_keys.set_disabled(0);
    m_keys.set_disabled(1);
    // Cria rodapé
    MB_OSD_Footer::draw(m_main, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -40);
    // Caminho de migalhas
    if (Osd_Breadcrumb::s_instance.is_initialized() == false)
    {
        Osd_Breadcrumb::s_instance.init(m_bgd_main);
        Osd_Breadcrumb::s_instance.add_name(tr(__Instala_Facil));
    }

    // Inicia detecção do lnbf
    start_lnbf_detection();
}

void OSD_Instala_Facil::return_after_channel_list_screen(bool _result)
{
    m_osd_channel_list_update.reset();
    Task::post_event(std::bind(m_callback, _result));
}

void OSD_Instala_Facil::show_menu_activate_callback(bool _result)
{
    mb_osd_activate.reset();
    return_after_channel_list_screen(_result);
}

void OSD_Instala_Facil::create_main_screen()
{
    // Fundo preto
    if ( !m_bgd_main )
    {
        m_bgd_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_main);
        lv_obj_move_background(m_bgd_main);
    }
    // Tela principal
    if ( !m_main )
    {
        m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, height, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_main);
    }
    // Título
    if ( !m_title )
    {
        m_title = set_label_text(m_main, tr(__Bem_vindo_ao_instala_facil_century), 0, 0, font_semi_40, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_title);
        lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    }
    // Subtitulo
    if ( !m_subtitle )
    {
        m_subtitle = set_label_text(m_main, tr(__Deteccao_de_satelite_em_andamento_por_favor_aguarde), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_subtitle);
        lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, subtitle_y);
    }
#ifdef MBGUI_USE_RLOTTIE
    if (not m_sat_logo)
    {
        m_sat_logo = lv_rlottie_create_from_file(m_main, 200, 200, ANIM_SAT_SIGNAL);
        lv_obj_null_on_delete(&m_sat_logo);
        lv_rlottie_set_play_mode(m_sat_logo, LV_RLOTTIE_CTRL_LOOP);
        lv_obj_align(m_sat_logo, LV_ALIGN_TOP_MID, 0, subtitle_y + 25);
    }
#endif
}

void OSD_Instala_Facil::process_ok_key()
{
    DEBUG_MSG(OSD, DEBUG, "status: " << static_cast<int>(m_status) << "\n");

    switch (m_status)
    {
        case Status::Detect:
        {
            if (m_satellite == Satellite::Sky_B1)
            {
                DEBUG_MSG(OSD, DEBUG, "Sky B1\n");
            }
        }
        break;

        case Status::Success:
        {
            clear_screen();
            DEBUG_MSG(OSD, DEBUG, "status_e::__scanning\n");

            if (m_satellite == Satellite::Sky_B1)
            {
                // Apaga arquivo com termos de uso
                {
                    namespace fs = std::filesystem;
                    fs::remove(MBGUI_TERMS_CONDITIONS_DATE_FILE);
                }
                terms_of_use();
            }
            else
            {
                looking_for_channel_list_update();
            }
        }
        break;

        default:
            break;
    }
}

void OSD_Instala_Facil::terms_of_use_callback(bool _result)
{
    DEBUG_MSG(OSD, DEBUG, (_result ? "Accepted" : "Declined") << "\n");
    m_osd_terms_of_use.reset();

    if (_result)
    {
        looking_for_channel_list_update();
    }
}

void OSD_Instala_Facil::terms_of_use()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Instala_Facil::terms_of_use\n");

    if (!m_osd_terms_of_use)
    {
        m_osd_terms_of_use = std::make_unique<OSD_Terms_of_Use>(this);
    }

    m_osd_terms_of_use->show_menu_terms_of_use(std::bind(&OSD_Instala_Facil::terms_of_use_callback, this, std::placeholders::_1));
}

void OSD_Instala_Facil::restart_timer_cb(lv_timer_t *tm)
{
    (void)tm;
    g_mbgui_reboot_after_exit.store(true, std::memory_order_release);
    g_mbgui_keep_running.store(false, std::memory_order_release);
    g_mbgui_restart_on_exit.store(false, std::memory_order_release);
}

void OSD_Instala_Facil::show_menu_channel_list_update_callback(bool result)
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Instala_Facil::show_menu_channel_list_update_callback: ");

    if (result)
    {
        auto stb_activated = false;
        auto config = Config::get_config();
        Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
        if(Zone_ID::get_zone_id(_oper) > 0)
        {
            stb_activated = true;
        }

        if (!mb_osd_activate)
        {
            mb_osd_activate = std::make_unique<OSD_Activate>(this);
        }

        mb_osd_activate->show_menu_activate(std::bind(&OSD_Instala_Facil::show_menu_activate_callback, this, std::placeholders::_1), stb_activated);
    }
    else
    {
        return_after_channel_list_screen(result);
    }
}

void OSD_Instala_Facil::looking_for_channel_list_update()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Instala_Facil::looking_for_channel_list_update\n");
    if (m_satellite == Satellite::Sky_B1)
    {
        DEBUG_MSG(OSD, DEBUG, "Sky B1\n");
        auto config = Config::get_config();
        config->set_satellite_config(Network_Id_Sky);
        Task::post_event_cas_switch_folder(true);
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "Star One D2\n");
        auto config = Config::get_config();
        config->set_satellite_config(Network_Id_Claro);
        Task::post_event_cas_switch_folder(false);
    }

    if (!m_osd_channel_list_update)
    {
        m_osd_channel_list_update = std::make_unique<OSD_Channel_List_Update>(this);
    }
    m_osd_channel_list_update->show_menu_channel_list_update(std::bind(&OSD_Instala_Facil::show_menu_channel_list_update_callback, this, std::placeholders::_1), m_restart);
}

void OSD_Instala_Facil::start_lnbf_detection()
{
    m_status = Status::Detect;
    DEBUG_MSG(OSD, DEBUG, "OSD_Instala_Facil::start_lnbf_detection\n");
    if (!m_detect_lnbf)
    {
        m_detect_lnbf = std::make_unique<MB_Detect_Lnbf>(this);
    }
    m_detect_lnbf->lnbf_detection_start(std::bind(&OSD_Instala_Facil::lnbf_detection_callback, this, std::placeholders::_1, std::placeholders::_2), mb::Satellite{});
}

void OSD_Instala_Facil::lnbf_detection_callback(bool _result, Transponder_Id _tp)
{
    DEBUG_MSG(OSD, DEBUG, (_result ? "Success" : "Fail") << ", Frequency: " << _tp.frequency() << "\n");
    if (_result)
    {
        // Salva satélite detectado no arquivo de estado
        State_File::App_State_File file;
        NID_t detected_network_id;
        if (_tp.frequency() == 12120000)
        {
            m_satellite = Satellite::Starone_D2;
            detected_network_id = Network_Id_Claro;
        }
        else
        {
            m_satellite = Satellite::Sky_B1;
            detected_network_id = Network_Id_Sky;
        }
        file.network_id = detected_network_id;
        file.write();

        // Check for satellite change
        NID_t last_network_id = 0;
        std::ifstream last_nid_file(MBGUI_LAST_NID_FILE, std::ios::binary);
        if (last_nid_file.is_open())
        {
            last_nid_file.read(reinterpret_cast<char*>(&last_network_id), sizeof(last_network_id));
            last_nid_file.close();

            if (last_network_id != 0 && last_network_id != detected_network_id)
            {
                DEBUG_MSG(OSD, INFO, "Satellite change detected: " << last_network_id << " -> " << detected_network_id << "\n");
                m_blocked = true;
                if (!m_message_box)
                {
                    m_message_box = std::make_unique<OSD_Message_Box>(this);
                }
                m_message_box->show_message_box(tr(__Mudanca_de_satelite_detectada_reiniciar));
                lv_timer_create(restart_timer_cb, 5000, nullptr);
            }
        }

        // Save detected network_id
        std::ofstream out_nid_file(MBGUI_LAST_NID_FILE, std::ios::binary);
        if (out_nid_file.is_open())
        {
            out_nid_file.write(reinterpret_cast<const char*>(&detected_network_id), sizeof(detected_network_id));
            out_nid_file.close();
        }

        auto config = Config::get_config();
        auto type = OSD_Translate::translate(config->lnbf_type());
        //config->load_satellite_list();

        auto satellite_name = m_satellites[static_cast<size_t>(m_satellite)];
        auto text = std::string(tr(__Satelite_detectado_com_sucesso));
        text += "\n";
        text += std::string(satellite_name) + " - " + std::string(type);
        lv_label_set_text(m_subtitle, text.data());
        m_keys.set_enabled(0);
        m_keys.set_enabled(1);

        while (m_keys.get_selected() != 1)
        {
            m_keys.next();
        }

        m_status = Status::Success;
        DEBUG_MSG(OSD, DEBUG, " - status_e::__success\n");
    }
    else
    {
        m_status = Status::Fail;
        DEBUG_MSG(OSD, DEBUG, " - status_e::__fail\n");
        lv_label_set_text(m_subtitle, tr(__Nenhum_satelite_encontrado).data());

        if (m_detect_lnbf)
        {
            m_detect_lnbf.reset();
        }
        start_osd_lnbf_snr();
    }

    if (m_detect_lnbf)
    {
        m_detect_lnbf.reset();
    }

#ifdef MBGUI_USE_RLOTTIE
    if (m_sat_logo)
    {
        lv_rlottie_set_play_mode(m_sat_logo, LV_RLOTTIE_CTRL_PAUSE);
        lv_obj_add_flag(m_sat_logo, LV_OBJ_FLAG_HIDDEN);
    }
#endif
}

void OSD_Instala_Facil::start_osd_lnbf_snr()
{
    if (!m_osd_lnbf_snr)
    {
        m_osd_lnbf_snr = std::make_shared<OSD_Lnbf_Snr>(this);
    }
    m_osd_lnbf_snr->show_lnbf_snr(std::bind(&OSD_Instala_Facil::osd_lnbf_snr_callback, this, std::placeholders::_1));
}

void OSD_Instala_Facil::osd_lnbf_snr_callback(bool _result)
{
    // Caso tenha encontrado sinal, refazer a detecção do lnbf
    if (_result)
    {
        DEBUG_MSG(OSD, DEBUG, "Success" << "\n");
        lv_label_set_text(m_subtitle, tr(__Deteccao_de_satelite_em_andamento_por_favor_aguarde).data());
        start_lnbf_detection();
#ifdef MBGUI_USE_RLOTTIE
        if (m_sat_logo)
        {
            lv_rlottie_set_play_mode(m_sat_logo, LV_RLOTTIE_CTRL_LOOP);
            lv_obj_remove_flag(m_sat_logo, LV_OBJ_FLAG_HIDDEN);
        }
#endif
    }
    else 
    {
        DEBUG_MSG(OSD, DEBUG, "Fail" << "\n");
        Task::post_event(std::bind(m_callback, false));
    }
    m_osd_lnbf_snr.reset();
}  

void OSD_Instala_Facil::got_focus()
{
}

void OSD_Instala_Facil::clear_screen()
{
#ifdef MBGUI_USE_RLOTTIE
    DELETE_OBJ(m_sat_logo);
#endif
    DELETE_OBJ(m_main);
    //DELETE_OBJ(m_bgd_main);
}

} // namespace mb

