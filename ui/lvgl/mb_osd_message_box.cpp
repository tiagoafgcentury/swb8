#include "mb_osd_message_box.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include <lvgl.h>

namespace {
}

namespace mb {

OSD_Message_Box::OSD_Message_Box(OSD *_parent):
    OSD(_parent),
    //options(offset_x, offset_y, option_w, option_h, option_s, option_x, option_y)
    m_options(0, 0, option_w, option_h, option_s, option_x, option_y)
{
}

OSD_Message_Box::~OSD_Message_Box()
{
    DELETE_OBJ(m_main);
    remove_focus();
}

void OSD_Message_Box::draw_message_box(std::string_view _text)
{
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    lv_obj_set_style_bg_opa(m_main, LV_OPA_TRANSP, 0);
    m_box = create_rect(m_main, 0, 0, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box);
    lv_obj_set_style_radius(m_box, 25, DEFAULT_SELECTOR);
    lv_obj_set_style_bg_opa(m_box, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(m_box, 3, 0);
    lv_obj_set_style_border_color(m_box, OSD_COLOR_WHITE, 0);
    lv_obj_set_style_border_opa(m_box, LV_OPA_COVER, 0);
    lv_obj_align(m_box, LV_ALIGN_CENTER, 0, 0);
    // Cria área de confirmação
    m_label = set_label_text(m_box, _text, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_label);
    lv_obj_set_width(m_label, 580);
    lv_obj_set_height(m_label, 130);
    lv_obj_align(m_label, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_text_align(m_label, LV_TEXT_ALIGN_CENTER, 0);
}

void OSD_Message_Box::show_message_box(std::string_view _text)
{
    draw_message_box(_text);
}

void OSD_Message_Box::show_message_box_ok(Message_Box_CB_t _callback, std::string_view _text)
{
    // Direciona recepção de tecla
    set_focus();
    m_callback = _callback;
    m_message_type = Message_Type::OK;
    draw_message_box(_text);
    // Cria botões de confirmação
    m_options.add_label("Ok");
    m_options.set_x(240);
    m_options.set_background(m_box);
    m_options.set_horizontal();
    m_options.draw();
    m_options.select();
}

void OSD_Message_Box::show_message_box_yes_no(Message_Box_CB_t _callback, std::string_view _text, bool _yes_select)
{
    // Direciona recepção de tecla
    set_focus();
    m_callback = _callback;
    draw_message_box(_text.data());
    m_message_type = Message_Type::YES_NO;

    m_options.clear();
    m_options =
    {
        tr(__Sim),
        tr(__Nao),
    };

    // Cria botões de confirmação
    m_options.set_x(150);
    m_options.set_background(m_box);
    m_options.set_horizontal();
    m_options.draw();
    m_options.select();
    if( not _yes_select)
    {
        m_options.next();
    }
}

void OSD_Message_Box::update_message(std::string_view _text)
{
    if(m_label)
    {
        lv_label_set_text(m_label, _text.data());
    }
}


// Processa tecla recebida
bool OSD_Message_Box::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if (m_message_type == Message_Type::OK)
    {
        if (_event.key == Remote_Control_Key::KEY_OK)
        {
            Task::post_event(std::bind(m_callback, false));
            return true;
        }
    }
    else if (m_message_type == Message_Type::YES_NO)
    {
        if (_event.key == Remote_Control_Key::KEY_VOLTAR)
        {
            Task::post_event(std::bind(m_callback, false));
            return true;
        }

        // Pressionado "OK", retorna com a confirmação ou não para salvar
        if (_event.key == Remote_Control_Key::KEY_OK)
        {
            auto selected = m_options.get_selected() == 0 ? true : false;
            Task::post_event(std::bind(m_callback, selected));
            return true;
        }

        // Alternar entre Salvar e __Cancelar
        if (_event.key == Remote_Control_Key::KEY_VOLUP ||  _event.key == Remote_Control_Key::KEY_VOLDOWN)
        {
            m_options.next();
            return true;
        }
    }

    return true;
}

} // namespace mb
