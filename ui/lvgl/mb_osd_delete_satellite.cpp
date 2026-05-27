#include "mb_osd_delete_satellite.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include <lvgl.h>

#include "mb_menu_resources.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "tasks/mb_task_eit_events.h"
#include "mb_events.h"

#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <src/misc/lv_color.h>
#include <src/misc/lv_types.h>
#include <src/misc/lv_color.h>
#include <time.h>

namespace mb {

OSD_Delete_Satellite *OSD_Delete_Satellite::s_instance { nullptr };

OSD_Delete_Satellite::OSD_Delete_Satellite(OSD *_parent):
    OSD(_parent)
{
    s_instance = this;
}

OSD_Delete_Satellite::~OSD_Delete_Satellite()
{
    DELETE_OBJ(m_mainscreen);
    remove_focus();
    s_instance = nullptr;
}

// Processa tecla recebida
bool OSD_Delete_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event(std::bind(m_callback, false));
        }
        break;

        case Remote_Control_Key::KEY_OK:
        {
            if (m_selected_button == Func_Active::Yes)
            {
                // Apaga satélite
                Task::post_event_delete_satellite(m_satellite.id);
                Task::post_event(std::bind(m_callback, true));
            }
            else
            {
                Task::post_event(std::bind(m_callback, false));
            }
        }
        break;

        case Remote_Control_Key::KEY_CHDOWN:
        {
        }
        break;

        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
        {
            m_selected_button = m_selected_button == Func_Active::Yes ? Func_Active::No : Func_Active::Yes;
            select();
        }
        break;

        default:
            break;
    }

    return true;
}

// Mostra a tela de seleção de satélite
void OSD_Delete_Satellite::delete_satellite(OSD_Delete_Satellite_CB_t _callback, Satellite _sat)
{
    m_callback = _callback;
    set_focus();
    // Carrega satélite recebido como argumento em variável da classe
    m_satellite = _sat;
    // Cria a tela principal
    m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_mainscreen);
    lv_obj_set_style_bg_opa(m_mainscreen, LV_OPA_90, 0);
    // Desenha logo
    m_logo = load_image(m_mainscreen, LOGO_MENU_CENTURY_C_CINZA, logo_x, logo_y, logo_width, logo_heigth);
    lv_obj_null_on_delete(&m_logo);
    // Desenha caminho de migalhas
    m_breadcrumb_box = create_rect(m_mainscreen, breadcrumb_x, breadcrumb_y, breadcrumb_width, breadcrumb_heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_breadcrumb_box);
    // Texto caminho de migalhas
    char breadcrumb[128];
    snprintf(breadcrumb, sizeof(breadcrumb), "%s > %s > %s > %s",  tr(__Ajustes).data(), tr(__Satelite).data(), tr(__Escolha_um_satelite).data(), m_satellite.name.data());
    m_breadcrumb_label = set_label_text(m_breadcrumb_box, breadcrumb, 0, 0, font_25, OSD_COLOR_GREY_DARK);
    lv_obj_null_on_delete(&m_breadcrumb_label);
    lv_obj_align(m_breadcrumb_label, LV_ALIGN_LEFT_MID, 0, 0);
    // Posição do texto complementar
    lv_point_t pos;
    lv_text_get_size(&pos, breadcrumb, font_25, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    // Desenha caixa de texto complementar
    m_item_box = create_rect(m_mainscreen, breadcrumb_x + pos.x + 25, breadcrumb_y, breadcrumb_width / 2, breadcrumb_heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_item_box);
    // Texto complementar
    char item[128];
    snprintf(item, sizeof(item), "> %s", tr(__Excluir).data());
    m_item_label = set_label_text(m_item_box, item, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_item_label);
    lv_obj_align(m_item_label, LV_ALIGN_LEFT_MID, -20, 0);
    // Título da página
    m_title_box = create_rect(m_mainscreen, title_x, title_y, title_width, title_heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_title_box);
    m_title_label = set_label_text(m_title_box, m_satellite.name.c_str(), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title_label);
    lv_obj_align(m_title_label, LV_ALIGN_CENTER, 0, 0);
    // Subtítulo da página
    m_subtitle_box = create_rect(m_mainscreen, subtitle_x, subtitle_y, subtitle_width, subtitle_heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_subtitle_box);
    m_subtitle_label = set_label_text(m_subtitle_box, tr(__Selecione_uma_das_opcoes_abaixo_para_prosseguir), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle_label);
    lv_obj_align(m_subtitle_label, LV_ALIGN_CENTER, 0, 0);
    // Botão sim
    m_yes_btn = create_rect(m_mainscreen, yes_x, yes_y, yes_width, yes_heigth, OSD_COLOR_GREY);
    lv_obj_null_on_delete(&m_yes_btn);
    lv_obj_set_style_radius(m_yes_btn, yes_heigth / 2, 0);
    m_yes_label = set_label_text(m_yes_btn, tr(__Sim), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_yes_label);
    lv_obj_align(m_yes_label, LV_ALIGN_CENTER, 0, 0);
    // Botão não
    m_no_btn = create_rect(m_mainscreen, no_x, no_y, no_width, no_heigth, OSD_COLOR_GREY);
    lv_obj_null_on_delete(&m_no_btn);
    lv_obj_set_style_radius(m_no_btn, no_heigth / 2, 0);
    m_no_label = set_label_text(m_no_btn, tr(__Nao), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_no_label);
    lv_obj_align(m_no_label, LV_ALIGN_CENTER, 0, 0);
    // Seleciona botão Editar
    select();
}

// Seleciona botão ativo
void OSD_Delete_Satellite::select()
{
    switch (m_selected_button)
    {
        case Func_Active::Yes:
        {
            lv_obj_set_style_bg_color(m_yes_btn, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
            lv_obj_set_style_bg_color(m_no_btn, OSD_COLOR_GREY, DEFAULT_SELECTOR);
        }
        break;

        case Func_Active::No:
        {
            lv_obj_set_style_bg_color(m_no_btn, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
            lv_obj_set_style_bg_color(m_yes_btn, OSD_COLOR_GREY, DEFAULT_SELECTOR);
        }
        break;

        default:
            break;
    }
}

} // namespace mb
