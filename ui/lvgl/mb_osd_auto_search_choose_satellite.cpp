#include "mb_osd_auto_search_choose_satellite.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_footer.h"
#include "mb_terms_of_use.h"
#include "mb_osd_fonts.h"

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
OSD_Auto_Search_Choose_Satellite::OSD_Auto_Search_Choose_Satellite(OSD *_parent):
    OSD(_parent)
{
    for (auto &label : m_satellite_label)
    {
        label = nullptr;
    }

    for (auto &box : m_satellite_box)
    {
        box = nullptr;
    }
}

OSD_Auto_Search_Choose_Satellite::~OSD_Auto_Search_Choose_Satellite()
{
    DELETE_OBJ(m_mainscreen);
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name();
}

// Processa tecla recebida
bool OSD_Auto_Search_Choose_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    static constexpr auto line_num = 4u;    // Total de linhas
    static constexpr auto column_num = 4u;  // Total de colunas
    auto len = m_satellites.size();
    auto line = m_selected_satellite / line_num;
    auto column = m_selected_satellite % column_num;

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        {
            if (m_selected_area == Selected_Area::Button)
            {
                Task::post_event(m_callback);
            }
            else
            {
                select_satellite();
            }
        }
        break;

        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event(m_callback);
        }
        break;

        case Remote_Control_Key::KEY_CHUP:
        {
            unselect();

            if (m_selected_area == Selected_Area::Button)
            {
                m_selected_area =  Selected_Area::Satellites;
                line = (line + 2) % 3;
                m_selected_satellite = line * line_num + column;

                while (m_selected_satellite >= len)
                {
                    --line;
                    m_selected_satellite = line * line_num + column;
                }
            }
            else if (m_selected_area == Selected_Area::Satellites && line == 0)
            {
                m_selected_area = Selected_Area::Button;
            }
            else
            {
                --line;
                m_selected_satellite = line * line_num + column;
            }

            select();
        }
        break;

        case Remote_Control_Key::KEY_CHDOWN:
        {
            unselect();

            if (m_selected_area == Selected_Area::Button)
            {
                m_selected_area =  Selected_Area::Satellites;
                line = (line + 1) % 3;
                m_selected_satellite = line * line_num + column;

                if (m_selected_satellite >= len)
                {
                    line = 0;
                    m_selected_satellite = column;
                }
            }
            else
            {
                auto limit = (line + 1) * (line_num + column);

                if (limit > len)
                {
                    m_selected_area = Selected_Area::Button;
                }
                else
                {
                    line = (line + 1) % 3;
                    m_selected_satellite = line * line_num + column;

                    if (m_selected_satellite >= len)
                    {
                        line = 0;
                        m_selected_satellite = column;
                    }
                }
            }

            select();
        }
        break;

        case Remote_Control_Key::KEY_VOLUP:
        {
            if (m_selected_area == Selected_Area::Button)
            {
                break;
            }

            unselect();
            column = (column + 1) % column_num;
            m_selected_satellite = line * line_num + column;

            if (m_selected_satellite >= len)
            {
                column = 0;
                m_selected_satellite = line * line_num + column;
            }

            select();
        }
        break;

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            if (m_selected_area == Selected_Area::Button)
            {
                break;
            }

            unselect();
            column = (column + 3) % column_num;
            m_selected_satellite = line * line_num + column;

            while (m_selected_satellite >= len)
            {
                --column;
                m_selected_satellite = line * line_num + column;
            }

            select();
        }

        default:
            break;
    }

    return true;
}

void OSD_Auto_Search_Choose_Satellite::show_menu_terms_of_use_callback(bool _result)
{
    if (_result)
    {
        if (!m_osd_as_channel_list)
        {
            m_osd_as_channel_list = std::make_unique<OSD_Auto_Search_Channel_List>(this);
        }

        auto satellite = m_satellites[m_selected_satellite];
        m_osd_as_channel_list->auto_search_channel_list(std::bind(&OSD_Auto_Search_Choose_Satellite::auto_search_channel_list_callback, this), satellite, false);
    }

    m_osd_terms_of_use.reset();
}

void OSD_Auto_Search_Choose_Satellite::auto_search_channel_list_callback()
{
    m_osd_as_channel_list.reset();
    Osd_Breadcrumb::s_instance.remove_name();
    Task::post_event(m_callback);
}

void OSD_Auto_Search_Choose_Satellite::select_satellite()
{
    auto config = Config::get_config();

    if (m_selected_satellite == 1 and not Terms_File::terms_has_been_accepted()) //SKY
    {
        config->set_satellite_config(Network_Id_Sky);

        if (!m_osd_terms_of_use)
        {
            m_osd_terms_of_use = std::make_unique<OSD_Terms_of_Use>(this);
        }

        m_osd_terms_of_use->show_menu_terms_of_use(std::bind(&OSD_Auto_Search_Choose_Satellite::show_menu_terms_of_use_callback, this, std::placeholders::_1));
    }
    else
    {
        if (m_selected_satellite == 0) //CLARO
        {
            config->set_satellite_config(Network_Id_Claro);
        }
        else
        {
            config->set_satellite_config(0);
        }

        if (!m_osd_as_channel_list)
        {
            m_osd_as_channel_list = std::make_unique<OSD_Auto_Search_Channel_List>(this);
        }

        auto satellite = m_satellites[m_selected_satellite];
        m_osd_as_channel_list->auto_search_channel_list(std::bind(&OSD_Auto_Search_Choose_Satellite::auto_search_channel_list_callback, this), satellite, false);
    }
}

// Mostra a tela de seleção de satélite
void OSD_Auto_Search_Choose_Satellite::show_select_satellite(Auto_Search_Choose_Satellite_CB_t _callback)
{
    m_callback = _callback;
    set_focus();
    // Cria a tela principal
    m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 147, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_mainscreen);
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Escolha_um_satelite));
    // Título da página
    m_title_box = create_rect(m_mainscreen, title_x, title_y, title_width, title_heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_title_box);
    m_title_label = set_label_text(m_title_box, tr(__Escolha_um_satelite), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title_label);
    lv_obj_align(m_title_label, LV_ALIGN_CENTER, 0, 0);
    // Desenha botão de sair
    m_button_box = create_rect(m_mainscreen, button_x, button_y, button_width, button_heigth, OSD_COLOR_GREY_MEDIUM);
    lv_obj_null_on_delete(&m_button_box);
    lv_obj_set_style_radius(m_button_box, button_heigth / 2, DEFAULT_SELECTOR);
    m_button_label = set_label_text(m_button_box, tr(__Voltar), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_button_label);
    lv_obj_align(m_button_label, LV_ALIGN_CENTER, 0, 0);
    // Cria rodapé
    MB_OSD_Footer::draw(m_mainscreen, tr(__Selecione_o_satelite_desejado_e_pressione_ok_para_continuar), -40);
    // Carrega lista de satélites
    process_satellite_list();
}

void OSD_Auto_Search_Choose_Satellite::select()
{
    if (m_selected_area == Selected_Area::Button)
    {
        auto label = lv_obj_get_child(m_button_box, 0);
        lv_obj_set_style_text_font(label, font_semi_25, 0);
        lv_obj_set_style_bg_color(m_button_box, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    }
    else if (m_satellite_box[m_selected_satellite])
    {
        auto label = lv_obj_get_child(m_satellite_box[m_selected_satellite], 0);
        lv_obj_set_style_text_font(label, font_semi_25, 0);
        lv_obj_set_style_bg_color(m_satellite_box[m_selected_satellite], OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    }
}

void OSD_Auto_Search_Choose_Satellite::unselect()
{
    if (m_selected_area == Selected_Area::Button)
    {
        auto label = lv_obj_get_child(m_button_box, 0);
        lv_obj_set_style_text_font(label, font_semi_25, 0);
        lv_obj_set_style_bg_color(m_button_box, OSD_COLOR_GREY_MEDIUM, DEFAULT_SELECTOR);
    }
    else if (m_satellite_box[m_selected_satellite])
    {
        auto label = lv_obj_get_child(m_satellite_box[m_selected_satellite], 0);
        lv_obj_set_style_text_font(label, font_25, 0);
        lv_obj_set_style_bg_color(m_satellite_box[m_selected_satellite], OSD_COLOR_GREY_MEDIUM, DEFAULT_SELECTOR);
    }
}

void OSD_Auto_Search_Choose_Satellite::process_satellite_list()
{
    DEBUG_MSG(OSD, DEBUG, "Loading satellite list...\n");
    m_satellites = Config::get_config()->get_satellite_list();
    // Apaga os 2 satélites obrigatórios da Claro e SKY, caso existam, para não exibi-los na tela de seleção
    m_satellites.erase(std::remove_if(m_satellites.begin(), m_satellites.end(), [](const Satellite& sat)
    {        
        return sat.is_mandatory;
    }), m_satellites.end());
    if (m_satellites.size() == 0)
    {
        DEBUG_MSG(OSD, WARN, "Lista de satélites vazia, ignorar\n");
        return;
    }

    for (auto &label : m_satellite_label)
    {
        DELETE_OBJ(label);
    }

    for (auto &box : m_satellite_box)
    {
        DELETE_OBJ(box);
    }

    int it = 0 ;
    DEBUG_MSG(OSD, DEBUG, "Total de satélites: " << m_satellites.size() << "\n");
    for ( auto &satellite : m_satellites )
    {
        DEBUG_MSG(OSD, DEBUG, "ID: " << satellite.id << " Nome: " << satellite.name << " Mandatory: " << (satellite.is_mandatory ? "true" : "false") << "\n");
        auto line = it / 4;
        auto column = it % 4;
        // Valores obtidos na tela de seleção de satélite
        auto x = 166 + column * (409 - 166);
        auto y = 308 - start_y + line * (381 - 308);
        m_satellite_box[it] = create_rect(m_mainscreen, x, y, satellite_width, satellite_heigth, OSD_COLOR_GREY_MEDIUM);
        lv_obj_null_on_delete(&m_satellite_box[it]);
        lv_obj_set_style_radius(m_satellite_box[it], satellite_heigth / 2, DEFAULT_SELECTOR);
        m_satellite_label[it] = set_label_text(m_satellite_box[it], m_satellites[it].name.c_str(), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_satellite_label[it]);
        lv_obj_align(m_satellite_label[it], LV_ALIGN_CENTER, 0, 0);
        // Incrementa o índice
        ++it;
    }
    // Exibe o primeiro satélite selecionado
    select();
}

} // namespace mb
