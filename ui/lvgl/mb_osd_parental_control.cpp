#include "mb_osd_parental_control.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_osd.h" // Add this include for Task_OSD
#include "common/mb_state_file.h"

namespace mb {

OSD_Parental_Control::OSD_Parental_Control():
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    Osd_Breadcrumb::s_instance.add_name(tr(__Senha));
}

OSD_Parental_Control::~OSD_Parental_Control()
{
    DELETE_OBJ(m_main_screen);
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name();
}

// Processa tecla recebida
bool OSD_Parental_Control::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(std::bind(m_callback, false));
            return true;

        case Remote_Control_Key::KEY_OK:
            save_configuration();
            Task::post_event(std::bind(m_callback, true));
            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            m_keys.next();
            return true;

        case Remote_Control_Key::KEY_CHUP:
            m_keys.previous();
            return true;

        default:
            DEBUG_MSG(OSD, WARN, "Key 0x" << HEXBYTE(_event.key) << " not handled in password context\n");
            return true;
    }
}

void OSD_Parental_Control::config_parental_control(Parental_Callback_CB_t _callback, lv_obj_t *_bgd)
{
    m_callback = _callback;
    m_main_screen = _bgd;
    // Direciona recepção de tecla
    set_focus();
    m_keys.set_background(m_main_screen);
    m_keys.clear();
    m_keys.add_label(tr(__Desabilitado));
    m_keys.add_label(tr(__10_anos));
    m_keys.add_label(tr(__12_anos));
    m_keys.add_label(tr(__14_anos));
    m_keys.add_label(tr(__16_anos));
    m_keys.add_label(tr(__18_anos));
    m_keys.set_vertical();      // Posição do teclado
    m_keys.set_align_center();  // Deve ser chamado antes de draw()
    m_keys.draw();
    m_keys.select();
    State_File::App_State_File file;
    m_parental_rating = file.parental_rating;

    if (m_parental_rating < m_keys.get_size())
    {
        while (m_keys.get_selected() != m_parental_rating)
        {
            m_keys.next();
        }
    }
}

void OSD_Parental_Control::save_configuration()
{
    // Save the parental control configuration
    DEBUG_MSG(OSD, DEBUG, "Saving parental control configuration\n");
    auto parental_rating = m_keys.get_selected();
    DEBUG_MSG(OSD, DEBUG, "Parental rating selected: " << (int)parental_rating << "\n");

    if (m_parental_rating != parental_rating)
    {
        DEBUG_MSG(OSD, DEBUG, "Parental rating changed from " << (int)m_parental_rating << " to " << (int)parental_rating << "\n");
        State_File::App_State_File file;
        file.parental_rating = parental_rating;
        file.write();
        Task::post_event(&Task_OSD::update_parental_control);
    }
}

} // namespace mb
