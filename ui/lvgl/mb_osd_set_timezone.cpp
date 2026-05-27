#include "mb_osd_set_timezone.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"

#include <lvgl.h>

namespace mb {

OSD_Set_Timezone::OSD_Set_Timezone(OSD *_parent):
    OSD(_parent),
    m_options(offset_x, offset_y, option_w, option_h, option_s, option_x, option_y)
{
    m_options =
    {
        tr(__Automatico),
        "F. Noronha UTC-2",
        "Brasília UTC-3",
        "Amazonas UTC-4",
        "Acre UTC-5",
    };
}

OSD_Set_Timezone::~OSD_Set_Timezone()
{
    Osd_Breadcrumb::s_instance.remove_name();
    remove_focus();
}

// Processa tecla recebida
bool OSD_Set_Timezone::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(std::bind(m_callback, m_timezone_mode));
            break;

        case Remote_Control_Key::KEY_OK:
            process_timezone();
            Task::post_event(std::bind(m_callback, m_timezone_mode));
            break;

        case Remote_Control_Key::KEY_CHUP:
            m_options.previous();
            break;

        case Remote_Control_Key::KEY_CHDOWN:
            m_options.next();

        default:
            break;
    }

    return true;
}

void OSD_Set_Timezone::show_set_timezone(Set_Timezone_CB_t _callback, lv_obj_t *_parent_screen, Timezone_Mode _timezone_mode)
{
    // Direciona recepção de tecla
    set_focus();
    m_callback = _callback;
    // Cria botões de confirmação
    m_options.set_background(_parent_screen);
    m_options.set_vertical();
    m_options.set_fonts(font_20, font_semi_20);
    m_options.draw();
    m_options.select();
    m_timezone_mode = _timezone_mode;
    m_options.set_previously_selected(static_cast<int>(m_timezone_mode));
    Osd_Breadcrumb::s_instance.add_name(tr(__Fuso_horario));
}

void OSD_Set_Timezone::process_timezone()
{
    m_timezone_mode = (static_cast<Timezone_Mode>(m_options.get_selected()));
}

} // namespace mb
