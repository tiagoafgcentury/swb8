
#include "mb_osd_menu_channel_list.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"

#include "tasks/mb_task_database.h"
#include "tasks/mb_task_player.h"

#include "common/mb_globals.h"
#include "hal/mb_display.h"
#include "mb_events.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "mb_osd_keys.h"

namespace mb {

OSD_Menu_Channel_List::OSD_Menu_Channel_List(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        tr(__Editar_Canais),
        tr(__Guia_Programacao),
        tr(__Agendamentos)
    };
}

OSD_Menu_Channel_List::~OSD_Menu_Channel_List()
{
    DELETE_OBJ(m_info_box);
    DELETE_OBJ(m_bgd);
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name(false);
    Osd_Breadcrumb::s_instance.remove_name(true);
    Osd_Breadcrumb::s_instance.clear();
}

void OSD_Menu_Channel_List::show_menu_channel_callback()
{
    lv_obj_set_size(m_main_menu, DISPLAY_WIDTH, main_h);

    if (not Lineup_Mutex_Ref::is_empty())
    {
        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Q2);
        //Return to last channel
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        current_lineup->set_channel_list_type(m_current_channel_list);
        Task::post_event_channel_change(POST_CALLER m_current_srv);
    }

    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_osd_edit_ch.reset();
}

void OSD_Menu_Channel_List::show_menu_guide_channel_callback()
{
    lv_obj_set_size(m_main_menu, DISPLAY_WIDTH, main_h);

    if (not Lineup_Mutex_Ref::is_empty())
    {
        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Q2);
    }

    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_osd_guide_channel.reset();
}

void OSD_Menu_Channel_List::show_menu_schedule_list_callback()
{
    lv_obj_set_size(m_main_menu, DISPLAY_WIDTH, main_h);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_osd_scheduled_list.reset();
}

bool OSD_Menu_Channel_List::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_MENU:
        case Remote_Control_Key::KEY_VOLTAR:
        {
            if (Lineup_Mutex_Ref::is_empty())
            {
                Task::post_event(std::bind(m_callback, true));
            }
            else
            {
                Task::post_event(std::bind(m_callback, false));
            }

            break;
        }

        case Remote_Control_Key::KEY_OK:
        {
            lv_obj_remove_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
            m_function_active = static_cast<Func_Active>(m_keys.get_selected());

            switch (m_function_active)
            {
                case Func_Active::Edit_Channels:
                {
                    if (not Lineup_Mutex_Ref::is_empty())
                    {
                        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Q4);
                        m_current_srv = Task::s_task_player->current_srv();
                        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                        m_current_channel_list = current_lineup->get_channel_list_type();
                    }

                    lv_obj_set_size(m_main_menu, main_w, main_h);
                    m_osd_edit_ch = std::make_unique<OSD_Edit_Channel>(this);
                    m_osd_edit_ch->show_menu_edit_channel(std::bind(&OSD_Menu_Channel_List::show_menu_channel_callback, this), m_channel_list_type);
                    break;
                }

                case Func_Active::Guide:
                {
                    if (not Lineup_Mutex_Ref::is_empty())
                    {
                        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Q4);
                    }

                    lv_obj_set_size(m_main_menu, main_w, main_h);
                    m_osd_guide_channel = std::make_unique<OSD_Guide_Channel>(this);
                    m_osd_guide_channel->show_menu_guide_channel(std::bind(&OSD_Menu_Channel_List::show_menu_guide_channel_callback, this), m_channel_list_type);
                    break;
                }

                case Func_Active::Schedules:
                {
                    lv_obj_set_size(m_main_menu, main_w, main_h);
                    m_osd_scheduled_list = std::make_unique<OSD_Scheduled_List>(this);
                    m_osd_scheduled_list->show_scheduled_list(std::bind(&OSD_Menu_Channel_List::show_menu_schedule_list_callback, this));
                    break;
                }

                case Func_Active::COUNT:
                    break;
            }

            return true;
        }

        case Remote_Control_Key::KEY_CHUP:
        {
            m_keys.previous();
            update_breadcrumb();
            break;
        }

        case Remote_Control_Key::KEY_CHDOWN:
        {
            m_keys.next();
            update_breadcrumb();
            break;
        }

        default:
            return true;
    }

    return true;
}

void OSD_Menu_Channel_List::show_menu_channel_list(OSD_Menu_channel_list_CB_t _callback, OSD_Channels_List_Type _channel_list_type)
{
    set_focus();
    m_callback = _callback;
    m_channel_list_type = _channel_list_type;
    m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd);
    lv_obj_set_style_bg_opa(m_bgd, LV_OPA_TRANSP, 0);
    // Tela principal
    m_main_menu = create_rect(m_bgd, main_x, main_y, DISPLAY_WIDTH, main_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_menu);
    // Desenha teclado
    m_keys.set_background(m_main_menu);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    m_bgd_fade = create_rect(m_main_menu, 0, 183, 280, 400, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_fade);
    lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    //Esse retangulo é criado para cobrir a tela superior ao exibir lista de canais
    m_bgd_top = create_rect(m_bgd, 0, 0, DISPLAY_WIDTH, 120, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_top);
    Osd_Breadcrumb::s_instance.init(m_bgd);

    if (m_channel_list_type == OSD_Channels_List_Type::TV_Channels)
    {
        Osd_Breadcrumb::s_instance.add_name({tr(__Canais_de_TV), tr(__Editar_Canais_TV)});
    }
    else
    {
        Osd_Breadcrumb::s_instance.add_name({tr(__Canais_de_Radio), tr(__Editar_Radios)});
    }
}

void OSD_Menu_Channel_List::got_focus()
{
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Menu_Channel_List::process()
{
}

void OSD_Menu_Channel_List::update_breadcrumb()
{
    // Atualiza breadcrumb com a função ativa
    m_function_active = static_cast<Func_Active>(m_keys.get_selected());

    if (m_function_active == Func_Active::Edit_Channels)
    {
        Osd_Breadcrumb::s_instance.replace_last_name(tr(__Editar_Canais_TV));
    }
    else if (m_function_active == Func_Active::Guide)
    {
        Osd_Breadcrumb::s_instance.replace_last_name(tr(__Guia_Programacao));
    }
    else
    {
        Osd_Breadcrumb::s_instance.replace_last_name(tr(__Agendamentos));
    }
}

} // namespace mb
