
#include "mb_osd_menu_ajustes.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_menu_satellite.h"

#include "tasks/mb_task_database.h"

#include "common/mb_globals.h"
#include "mb_events.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "mb_osd_keys.h"

namespace mb {

OSD_Menu_Ajustes::OSD_Menu_Ajustes(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    init_names();
}

OSD_Menu_Ajustes::~OSD_Menu_Ajustes()
{
    m_callback = nullptr;
    DELETE_OBJ(m_bdg_box);
    DELETE_OBJ(m_main_menu);
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name();
    Osd_Breadcrumb::s_instance.clear();
}

void OSD_Menu_Ajustes::osd_menu_satellite_callback(bool _result)
{
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_menu_satellite.reset();
    if(_result)
    {
        Task::post_event(std::bind(m_callback, true));
    }
}

void OSD_Menu_Ajustes::osd_menu_info_callback()
{
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    mb_osd_menu_info.reset();
}

void OSD_Menu_Ajustes::osd_menu_config_callback(bool _ok)
{
    // Atualiza texto em caso de alteração de idioma
    Osd_Breadcrumb::s_instance.clear();
    init_breadcrumb();
    Osd_Breadcrumb::s_instance.add_name(tr(__Ajustes_gerais));
    // Redesenha teclado
    init_names();
    m_keys.draw();
    m_keys.select();
    m_keys.next();
    lv_obj_move_foreground(m_bgd_fade);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_config.reset();

    if (_ok)
    {
        if (m_callback)
        {
            Task::post_event(std::bind(m_callback, true));
        }
    }
}

void OSD_Menu_Ajustes::osd_menu_support_callback()
{
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_support.reset();
}

void OSD_Menu_Ajustes::osd_menu_updates_callback()
{
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_update.reset();
}

bool OSD_Menu_Ajustes::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_MENU:
        case Remote_Control_Key::KEY_VOLTAR:
            if (not Lineup_Mutex_Ref::is_empty())
            {
                if (m_callback)
                {
                    Task::post_event(std::bind(m_callback, false));
                }
            }

            return true;

        case Remote_Control_Key::KEY_OK:
        {
            lv_obj_remove_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
            m_function_active = static_cast<Func_Active>(m_keys.get_selected());

            switch (m_function_active)
            {
                case Func_Active::Satellite:
                {
                    m_menu_satellite = std::make_unique<OSD_Menu_Satellite>(this);
                    m_menu_satellite->show_menu_satellite(std::bind(&OSD_Menu_Ajustes::osd_menu_satellite_callback, this, std::placeholders::_1), m_bdg_box);
                    break;
                }

                case Func_Active::Info:
                {
                    mb_osd_menu_info = std::make_unique<OSD_Menu_Info>(this);
                    mb_osd_menu_info->show_menu_info(std::bind(&OSD_Menu_Ajustes::osd_menu_info_callback, this), m_bdg_box);
                    break;
                }

                case Func_Active::Adjust:
                {
                    m_config = std::make_unique<OSD_Menu_Config>(this);
                    m_config->show_menu_config(std::bind(&OSD_Menu_Ajustes::osd_menu_config_callback, this, std::placeholders::_1), m_bdg_box);
                    break;
                }

                case Func_Active::Updates:
                {
                    m_update = std::make_unique<OSD_Menu_Update>(this);
                    m_update->show_menu_update(std::bind(&OSD_Menu_Ajustes::osd_menu_updates_callback, this), m_bdg_box);
                    break;
                }

                case Func_Active::Support:
                {
                    m_support = std::make_unique<OSD_Menu_Support>(this);
                    m_support->show_menu_support(std::bind(&OSD_Menu_Ajustes::osd_menu_support_callback, this), m_bdg_box);
                    break;
                }

                case Func_Active::COUNT:
                    break;
            }

            return true;
        }

        case Remote_Control_Key::KEY_CHUP:
            m_keys.previous();
            update_breadcrumb();
            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            m_keys.next();
            update_breadcrumb();
            return true;

        default:
            return true;
    }

    return true;
}

void OSD_Menu_Ajustes::update_breadcrumb()
{
    auto function_active = static_cast<Func_Active>(m_keys.get_selected());
    switch (function_active)
    {
        case Func_Active::Satellite:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Satelite));
            break;
        }

        case Func_Active::Info:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Informacoes));
            break;
        }

        case Func_Active::Adjust:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Ajustes_gerais));
            break;
        }

        case Func_Active::Updates:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Atualizacoes));
            break;
        }

        case Func_Active::Support:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Suporte));
            break;
        }

        case Func_Active::COUNT:
            break;
    }
}

void OSD_Menu_Ajustes::show_menu_ajustes(OSD_Menu_Ajustes_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    // Tela principal
    m_main_menu = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), main_x, main_y, main_w, main_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_menu);
    m_bdg_box = create_rect(m_main_menu, 250, 110, 820, 610, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bdg_box);
    // Desenha teclado
    m_keys.set_background(m_main_menu);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    //create_line(m_bgd, 148, 0, OSD_COLOR_GREY_DARK);
    m_bgd_fade = create_rect(m_main_menu, 0, 183, 247, 400, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_fade);
    lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    // Cria caminho de migalhas
    init_breadcrumb();
    Osd_Breadcrumb::s_instance.add_name(tr(__Satelite));
}

void OSD_Menu_Ajustes::init_breadcrumb()
{
    Osd_Breadcrumb::s_instance.init(nullptr);
    Osd_Breadcrumb::s_instance.add_name(tr(__Ajustes));
}

void OSD_Menu_Ajustes::got_focus()
{
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Menu_Ajustes::process()
{
}

void OSD_Menu_Ajustes::init_names()
{
    m_keys.clear();
    m_keys =
    {
         tr(__Satelite),
         tr(__Ajustes_gerais),
         tr(__Atualizacoes),
         tr(__Informacoes),
         tr(__Suporte)
    };
}

} // namespace mb
