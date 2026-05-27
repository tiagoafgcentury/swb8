#include "mb_osd_confirm_default.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_fonts.h"

#include <lvgl.h>

namespace {
}

namespace mb {

OSD_Confirm_Default::OSD_Confirm_Default(OSD *_parent):
    OSD(_parent)
{
}

OSD_Confirm_Default::~OSD_Confirm_Default()
{
    DELETE_OBJ(m_main);
    // Remove foco do controle remoto
    remove_focus();
}

void OSD_Confirm_Default::show_confim_default(Confirm_CB_t _callback, lv_obj_t *m_parent_screen)
{
    // Direciona recepção de tecla
    DEBUG_MSG(OSD, DEBUG, "show_confim_default()\n");
    set_focus();
    m_callback = _callback;

    // Verifica se existe uma tela anterior
    if (m_parent_screen)
    {
        m_main = lv_canvas_create(m_parent_screen);
    }
    else
    {
        m_main = lv_canvas_create(get_main_screen(OSD_Layer::MAIN_MENU));
    }

    // Cria área de confirmação
    m_main_box = create_rect(m_main, x_pos, y_pos, width, heigth, OSD_COLOR_BLACK);
    // Mensagem para executar padrão de fábrica
    std::string text;
    text.append(tr(__Deseje_executar_padrao_de_fabrica));
    text.append("\n");
    text.append(tr(__Esta_acao_vai_apagar_todos_canais));
    m_confirm_text = set_label_text(m_main_box, text.data(), 60, 60, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_set_style_width(m_confirm_text, width - 120, 0);
    // Botão salvar desabilitado
    m_save_btn = lv_button_create(m_main_box);
    lv_obj_set_pos(m_save_btn, save_x, save_y);
    lv_obj_set_size(m_save_btn, btn_width, btn_heigth);
    m_save_label = lv_label_create(m_save_btn);
    lv_obj_set_style_text_font(m_save_label, font_semi_25, 0);
    lv_label_set_text_static(m_save_label, tr(__Salvar).data());
    lv_obj_center(m_save_label);
    // Botão salvar habilitado
    m_cancel_btn = lv_button_create(m_main_box);
    lv_obj_set_pos(m_cancel_btn, cancel_x, cancel_y);
    lv_obj_set_size(m_cancel_btn, btn_width, btn_heigth);
    m_cancel_label = lv_label_create(m_cancel_btn);
    lv_obj_set_style_text_font(m_cancel_label, font_semi_25, 0);
    lv_label_set_text_static(m_cancel_label, tr(__Cancelar).data());
    lv_obj_center(m_cancel_label);
    // Indica botão ativo
    set_btn_active();
}

// Processa tecla recebida
bool OSD_Confirm_Default::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Pressionado "Voltar", retorna com false
    if (_event.key == Remote_Control_Key::KEY_VOLTAR)
    {
        Task::post_event(std::bind(m_callback, false));
        return true;
    }

    // Pressionado "OK", retorna com a confirmação ou não para salvar
    if (_event.key == Remote_Control_Key::KEY_OK)
    {
        // Resposta da função
        bool res = false;

        if (m_status == Func_Status::Save)
        {
            res = true;
            Task::post_event_system_factory_reset();
        }

        Task::post_event(std::bind(m_callback, res));
        return true;
    }

    // Alternar entre Salvar e __Cancelar
    if (_event.key == Remote_Control_Key::KEY_VOLUP ||  _event.key == Remote_Control_Key::KEY_VOLDOWN)
    {
        // Inverte o status
        m_status = m_status == Func_Status::Cancel ? Func_Status::Save : Func_Status::Cancel;
        set_btn_active();
    }

    return true;
}

void OSD_Confirm_Default::set_btn_active()
{
    if (m_status == Func_Status::Cancel)
    {
        lv_obj_set_style_text_color(m_cancel_label, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_color(m_cancel_btn, OSD_COLOR_BLUE, DEFAULT_SELECTOR);
        lv_obj_set_style_text_color(m_save_label, OSD_COLOR_BLACK, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_color(m_save_btn, OSD_COLOR_BLACK_LIGHT, DEFAULT_SELECTOR);
    }
    else
    {
        lv_obj_set_style_text_color(m_save_label, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_color(m_save_btn, OSD_COLOR_BLUE, DEFAULT_SELECTOR);
        lv_obj_set_style_text_color(m_cancel_label, OSD_COLOR_BLACK, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_color(m_cancel_btn, OSD_COLOR_BLACK_LIGHT, DEFAULT_SELECTOR);
    }
}

} // namespace mb
