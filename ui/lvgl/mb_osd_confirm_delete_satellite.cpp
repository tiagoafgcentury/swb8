#include "mb_osd_confirm_delete_satellite.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_eit_events.h"

#include <lvgl.h>
#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <time.h>

namespace mb {

Confirm_Delete_Satellite::Confirm_Delete_Satellite(OSD *_parent):
    OSD(_parent)
{
}

Confirm_Delete_Satellite::~Confirm_Delete_Satellite()
{
    DELETE_OBJ(m_mainscreen);
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name(false);
    Osd_Breadcrumb::s_instance.remove_name(true);
}

// Processa tecla recebida
bool Confirm_Delete_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
            if (m_sat_mandatory == false)
            {
                auto result = m_btn_seletected == Btn_Active::Yes ? true : false;
                Task::post_event(std::bind(m_callback, result));
            }
            else
            {
                Task::post_event(std::bind(m_callback, false));
            }

            break;

        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event(std::bind(m_callback, false));
        }
        break;

        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
            if (m_sat_mandatory == false)
            {
                unselect();
                m_btn_seletected = static_cast<Btn_Active>((static_cast<int>(m_btn_seletected) + 1) % static_cast<int>(Btn_Active::COUNT));
                select();
            }

            break;

        default:
            break;
    }

    return true;
}

// Mostra a tela de seleção de satélite
void Confirm_Delete_Satellite::confirm_delete_satellite(confirm_delete_satellite_cb_t _callback, Satellite satellite)
{
    m_callback = _callback;
    set_focus();
    // Cria a tela principal
    m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 147, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_mainscreen);
    // Título da página
    m_title_box = create_rect(m_mainscreen, 0, 0, width, title_heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_title_box);
    m_title_label = set_label_text(m_title_box, tr(__Apagar_satelite), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title_label);
    lv_obj_align(m_title_label, LV_ALIGN_TOP_MID, 0, 0);
    m_sat_mandatory = satellite.is_mandatory;

    if (satellite.is_mandatory)
    {
        // Subtítulo da página
        m_subtitle_box = create_rect(m_mainscreen, 0, 130, width, subtitle_heigth, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_subtitle_box);
        char s[128];
        snprintf(s, sizeof(s), "%s %s ", tr(__Impossivel_remover_o_satelite).data(), satellite.name.data());
        m_subtitle_label = set_label_text(m_subtitle_box, s, 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_subtitle_label);
        lv_obj_align(m_subtitle_label, LV_ALIGN_TOP_MID, 0, 0);
        m_btn_voltar = create_rect(m_mainscreen, 530, 400, button_w, button_h, OSD_COLOR_ORANGE);
        lv_obj_null_on_delete(&m_btn_voltar);
        lv_obj_set_style_radius(m_btn_voltar, 25, DEFAULT_SELECTOR);
        m_lbl_voltar = set_label_text_static(m_btn_voltar, tr(__Voltar), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_voltar);
        lv_obj_align(m_lbl_voltar, LV_ALIGN_CENTER, 0, 0);
    }
    else
    {
        // Subtítulo da página
        m_subtitle_box = create_rect(m_mainscreen, 0, subtitle_y, width, subtitle_heigth, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_subtitle_box);
        char s[128];
        snprintf(s, sizeof(s), "%s %s ?",  tr(__Tem_certeza_que_deseja_apagar_o_satelite).data(), satellite.name.data());
        m_subtitle_label = set_label_text(m_subtitle_box, s, 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_subtitle_label);
        lv_obj_align(m_subtitle_label, LV_ALIGN_TOP_MID, 0, 0);
        // Desenha botões
        draw_buttons();
        select();
    }

    // Cria rodapé
    MB_OSD_Footer::draw(m_mainscreen, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -40);
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name({satellite.name.c_str(), tr(__Excluir)});
}

void Confirm_Delete_Satellite::draw_buttons()
{
    // Cria botões
    for (size_t i = 0; i < m_buttons.size(); i++)
    {
        auto x = button_x + i * button_s;
        m_buttons[i] = create_rect(m_mainscreen, x, button_y, button_w, button_h, OSD_COLOR_GREY);
        lv_obj_set_style_radius(m_buttons[i], button_h / 2, DEFAULT_SELECTOR);
        m_labels[i] = set_label_text(m_buttons[i], m_func_names[i], 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_labels[i], LV_ALIGN_CENTER, 0, 0);
    }
}

void Confirm_Delete_Satellite::select()
{
    lv_obj_t *obj = m_buttons[static_cast<int>(m_btn_seletected)];
    lv_obj_set_style_bg_color(obj, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
}

void Confirm_Delete_Satellite::unselect()
{
    lv_obj_t *obj = m_buttons[static_cast<int>(m_btn_seletected)];
    lv_obj_set_style_bg_color(obj, OSD_COLOR_GREY, DEFAULT_SELECTOR);
}

} // namespace mb
