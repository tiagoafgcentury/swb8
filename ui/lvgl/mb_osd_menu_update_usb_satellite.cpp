#include "mb_osd_menu_update_usb_satellite.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_password.h"
#include <lvgl.h>

namespace {
}

namespace mb {

OSD_Menu_Update_Usb_Satellite::OSD_Menu_Update_Usb_Satellite(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        "USB",
        tr(__Satelite),
    };
}

OSD_Menu_Update_Usb_Satellite::~OSD_Menu_Update_Usb_Satellite()
{
    DELETE_OBJ(m_main);
    // Retira item do caminho de migalhas
    Osd_Breadcrumb::s_instance.remove_name();
    // Remove foco do controle remoto
    remove_focus();
}

void OSD_Menu_Update_Usb_Satellite::show_menu_software_update_callback()
{
    lv_obj_remove_flag(m_main, LV_OBJ_FLAG_HIDDEN);
    m_software_update.reset();
    //Task::post_event_player_restart();
}

// Processa tecla recebida
bool OSD_Menu_Update_Usb_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Tecla OK: exibe lista de satélites
    if (_event.key == Remote_Control_Key::KEY_OK)
    {
        auto selected = static_cast<Func_Active>(m_keys.get_selected());
        lv_obj_add_flag(m_main, LV_OBJ_FLAG_HIDDEN);

        if (!m_software_update)
        {
            m_software_update = std::make_unique<OSD_Software_Update>(this);
        }

        m_software_update->show_menu_software_update(std::bind(&OSD_Menu_Update_Usb_Satellite::show_menu_software_update_callback, this), selected == Func_Active::Satellite, false);
        return true;
    }

    // Pressionado "Voltar", retorna com false
    if (_event.key == Remote_Control_Key::KEY_VOLTAR)
    {
        Task::post_event(m_callback);
        return true;
    }

    // Seleciona próximo botão
    if (_event.key == Remote_Control_Key::KEY_CHDOWN ||
            _event.key == Remote_Control_Key::KEY_CHUP)
    {
        m_keys.next();
        auto active = static_cast<Func_Active>(m_keys.get_selected());
        // Substitui item do caminho de migalhas
        if (active == Func_Active::Satellite)
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Satelite));
        }
        else
        {
            Osd_Breadcrumb::s_instance.replace_last_name("USB");
        }
    }

    return true;
}

void OSD_Menu_Update_Usb_Satellite::show_menu_update_usb_satellite(Menu_Update_USB_Satellite_CB_t _callback, lv_obj_t *_bgd)
{
    // Direciona recepção de tecla
    DEBUG_MSG(OSD, DEBUG, "show_menu_satellite()\n");
    set_focus();
    m_callback = _callback;
    // Cria área do menu
    m_main = create_rect(_bgd, 300, 80, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    // Desenha linha vertical
    m_line_left = create_rect(m_main, 0, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_left);
    m_line_right = create_rect(m_main, width - 3, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_right);
    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    // Cria cortina sobe o menu que fica inicialmente apagada
    m_bgd_fade = create_rect(m_main, 0, 0, width - 3, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_fade);
    lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    // Preenche caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name("USB");
}

} // namespace mb
