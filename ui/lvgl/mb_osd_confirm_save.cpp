#include "mb_osd_confirm_save.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_fonts.h"

#include <lvgl.h>

namespace {
}

namespace mb {

OSD_Confirm_Save::OSD_Confirm_Save(OSD *_parent):
    OSD(_parent),
    m_options(offset_x, offset_y, option_w, option_h, option_s, option_x, option_y)
{
    for (auto name : m_option_names)
    {
        m_options.add_label(name);
    }
}

OSD_Confirm_Save::~OSD_Confirm_Save()
{
    DELETE_TIMER(m_refresh_timer);
    remove_focus();
}

void OSD_Confirm_Save::show_confirm_save(Confirm_Save_CB_t _callback, lv_obj_t *_parent_screen, std::string_view _text, bool _timer_enable)
{
    // Direciona recepção de tecla
    set_focus();
    m_callback = _callback;
    // Cria área de confirmação
    m_label = set_label_text(_parent_screen, _text, 0, 0, font_25, OSD_COLOR_WHITE);
    //lv_obj_align(m_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(m_label, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_width(m_label, 380);
    lv_label_set_long_mode(m_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(m_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_null_on_delete(&m_label);
    m_timer_label = set_label_text(_parent_screen, "", 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_timer_label);
    lv_obj_align(m_timer_label, LV_ALIGN_TOP_MID, 0, 335 - 114);
    lv_obj_set_style_text_align(m_timer_label, LV_TEXT_ALIGN_CENTER, 0);
    // Cria botões de confirmação
    m_options.set_background(_parent_screen);
    m_options.set_horizontal();
    m_options.draw();
    m_options.select();
    m_options.next();

    if (_timer_enable)
    {
        m_timer_label2 = set_label_text(_parent_screen, "", 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_timer_label2);
        lv_obj_align(m_timer_label2, LV_ALIGN_CENTER, 0, -10);
        lv_obj_set_style_text_align(m_timer_label2, LV_TEXT_ALIGN_CENTER, 0);
        // Cria timer para atualização de tela
        m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
    }
}

void OSD_Confirm_Save::refresh_cb(lv_timer_t *tm)
{
    OSD_Confirm_Save *thiz = static_cast<OSD_Confirm_Save *>(lv_timer_get_user_data(tm));
    thiz->refresh_progress();
}

void OSD_Confirm_Save::refresh_progress()
{
    if (m_timeout--)
    {
        lv_label_set_text_fmt(m_timer_label2, "%ds", m_timeout);
    }
    else
    {
        Task::post_event(std::bind(m_callback, false));
    }
}

// Processa tecla recebida
bool OSD_Confirm_Save::handle_event_remote_control(const Event_Remote_Control &_event)
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
        auto selected = m_options.get_selected() == 0 ? true : false;
        Task::post_event(std::bind(m_callback, selected));
        return true;
    }

    // Alternar entre Salvar e __Cancelar
    if (_event.key == Remote_Control_Key::KEY_VOLUP ||  _event.key == Remote_Control_Key::KEY_VOLDOWN)
    {
        m_options.next();
    }

    return true;
}

} // namespace mb
