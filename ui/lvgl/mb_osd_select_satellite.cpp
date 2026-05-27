#include "mb_osd_select_satellite.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"
#include <lvgl.h>

#include "mb_menu_resources.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task_eit_events.h"

#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <time.h>

namespace mb {

// Inicializa objetos no construtor
Select_Satellite::Select_Satellite(OSD *_parent):
    OSD(_parent),
    m_keys(0, 0, keys_w, keys_h, 0, keys_x, keys_y)
{
}

// Destrutor
Select_Satellite::~Select_Satellite()
{
    DELETE_OBJ(m_mainscreen);
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name();
}

void Select_Satellite::confirm_delete_satellite_callback(bool _confirm_ok)
{
    if (_confirm_ok)
    {
        auto s = m_satellites[m_selected_satellite];
        if (s.is_mandatory == false)
        {
            Task::post_event_delete_satellite(s.id);
            DEBUG("Apagar satélite\n");
            // Apaga satélite selecionado da lista
            m_satellites.erase(m_satellites.begin() + m_selected_satellite);
            process_satellite_list();
        }
    }
    else
    {
        DEBUG("Não apagar satélite\n");
    }

    m_confirm_delete.reset(); 
}

// Processa tecla recebida
bool Select_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_PLUS:
        {
            DEBUG("Add satellite\n");
            auto len = m_satellites.size();
            if (len <  MAX_SATELLITE)
            {
                DEBUG("Adding new satellite\n");
                auto new_satellite = m_dummy_satellite;
                new_satellite.name = "Novo satélite " + std::to_string(len + 1);
                Task::post_event(std::bind(m_callback, new_satellite, true));
            }
            break;
        }

        case Remote_Control_Key::KEY_5:
        {
            m_selected_satellite = m_keys.get_selected();
            if (m_selected_satellite < m_satellites.size())
            {
                if (!m_confirm_delete)
                {
                    m_confirm_delete = std::make_unique<Confirm_Delete_Satellite>(this);
                }
                auto sat = m_satellites[m_selected_satellite];
                m_confirm_delete->confirm_delete_satellite(std::bind(&Select_Satellite::confirm_delete_satellite_callback, this, std::placeholders::_1), sat);
            }
        }
        break;

        case Remote_Control_Key::KEY_OK:
        {
            m_pressed_key = _event.key;
            auto exec_func = m_keys.get_selected_callback();
            if (exec_func) exec_func();
            break;
        }

        case Remote_Control_Key::KEY_VOLTAR:
        {
            Satellite sat = {0, "", Band::Ku, LNBF_Type::Multi, LNBF_Position::Normal, DiseqC_Type::None, 0, 0, true };
            Task::post_event(std::bind(m_callback, sat, false));
        }
        break;

        case Remote_Control_Key::KEY_CHUP:
        {
            m_keys.previous_line();
            break;
        }
        
        case Remote_Control_Key::KEY_CHDOWN:
        {
            m_keys.next_line();
            break;
        }
        
        case Remote_Control_Key::KEY_VOLUP:
        {
            m_keys.next();
            break;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            m_keys.previous();
            break;
        }    

        default:
            break;
    }
    return true;
}

// Mostra a tela de seleção de satélite
void Select_Satellite::show_select_satellite(select_satellite_callback_t _callback)
{
    m_callback = _callback;
    set_focus();
    // Cria a tela principal
    m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 147, width, heigth, OSD_COLOR_BLACK);
    auto m_title_label = set_label_text(m_mainscreen, tr(__Escolha_um_satelite), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(m_title_label, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_null_on_delete(&m_mainscreen);
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Escolha_um_satelite));
    // Cria rodapé
    m_footer = MB_OSD_Footer::draw(m_mainscreen, tr(__Selecione_o_satelite_desejado_e_pressione_ok_para_editar_5_para_remover_ou_mais_para_incluir_um_novo), -40);
    lv_obj_null_on_delete(&m_footer);
    // Carrega lista de satélites
    load_satellite_list();
}

void Select_Satellite::load_satellite_list()
{
    DEBUG_MSG(OSD, DEBUG, TERM_YELLOW_BOLD "Select_Satellite::load_satellite_list()\n" TERM_RESET);
    auto callback = [this](const std::vector<Satellite> &sats)
    {
        if(sats.size() > 0)
        {
            this->m_satellites.clear();
            this->m_satellites = sats;
            this->process_satellite_list();
        }
    };    
    Task::post_event_satellite_list_load(std::bind(callback, std::placeholders::_1));
}

void Select_Satellite::process_satellite_list()
{
    DEBUG("Drawing satellite list...\n");
    // Limpa teclas anteriores
    m_keys.clear();
    // Desenha teclas dos satélites
    m_keys.set_background(m_mainscreen);
    m_keys.set_horizontal();
    m_keys.set_spacing(keys_x_spacing,keys_y_spacing);
    for (const auto &sat : m_satellites)
    {
        m_keys.add_label(sat.name, std::bind(&Select_Satellite::on_satellite_callback, this), 0);
        DEBUG("Satellite: " << sat.name << "\n");
    }
    m_keys.set_columns(4);
    m_keys.set_lines(3);
    // Tecla fora do grid
    m_keys.add_group();
    m_keys.set_x(voltar_keys_x);
    m_keys.set_y(voltar_keys_y);
    m_keys.set_lines(1);
    m_keys.set_columns(1);
    m_keys.add_label(tr(__Voltar), std::bind(&Select_Satellite::on_voltar_callback, this), 0);
    m_keys.draw();
    m_keys.select(0);
}

void Select_Satellite::on_satellite_callback()
{
    m_selected_satellite = m_keys.get_selected();
    auto sat = m_satellites[m_selected_satellite];
    DEBUG("Satellite selected: " << sat.name << ", LNBF_Type: " << to_str(sat.type) << "\n");
    Task::post_event(std::bind(m_callback, sat, true));
}

void Select_Satellite::on_voltar_callback()
{
    Satellite s = {0, "", Band::Ku, LNBF_Type::Multi, LNBF_Position::Normal, DiseqC_Type::None, 0, 0, true };
    Task::post_event(std::bind(m_callback, s, false));
}

void Select_Satellite::got_focus()
{
}

} // namespace mb
