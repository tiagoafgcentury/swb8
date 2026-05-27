#include "mb_osd_menu_info.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"

#include "common/mb_globals.h"
#include "mb_events.h"
#include "../../project_version.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>


namespace mb {

OSD_Menu_Info::OSD_Menu_Info(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        tr(__Inf_do_Receptor),
        tr(__Acesso_Condicional),
        tr(__Tela_de_Diagnostico),
    };
}

OSD_Menu_Info::~OSD_Menu_Info()
{
    DELETE_OBJ(m_bckg_fade);
    DELETE_OBJ(m_footer_main);
    DELETE_OBJ(m_footer_opt);
    DELETE_OBJ(m_main);
    Osd_Breadcrumb::s_instance.remove_name(true);
    remove_focus();
}

bool OSD_Menu_Info::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        {
            switch (static_cast<Func_Active>(m_keys.get_selected()))
            {
                case Func_Active::Receiver:
                {
                    process_receiver();
                    break;
                }

                case Func_Active::CA:
                {
                    process_ca();
                    break;
                }

                case Func_Active::Diagnostic:
                {
                    process_diagnostic();
                    break;
                }
            }

            break;
        }

        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

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

void OSD_Menu_Info::update_breadcrumb()
{
    switch (static_cast<Func_Active>(m_keys.get_selected()))
    {
        case Func_Active::Receiver:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Inf_do_Receptor));
            break;
        }

        case Func_Active::CA:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Acesso_Condicional));
            break;
        }

        case Func_Active::Diagnostic:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Tela_de_Diagnostico));
            break;
        }
    }
}

void OSD_Menu_Info::process_diagnostic_callback()
{
    DEBUG_MSG(OSD, DEBUG, "process_diagnostic_callback\n");
    mb_osd_diagnostics.reset();
}

void OSD_Menu_Info::process_diagnostic()
{
    mb_osd_diagnostics = std::make_unique<OSD_Diagnosis>(this);
    mb_osd_diagnostics->show_diagnostics(std::bind(&OSD_Menu_Info::process_diagnostic_callback, this), true);
}

void OSD_Menu_Info::process_ca_callback()
{
    // Apaga background e objetos
    DELETE_OBJ(m_rigth_line);
    DELETE_OBJ(m_ca_info);
    mb_osd_ca_info.reset();
    lv_obj_add_flag(m_bckg_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_footer_opt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(m_footer_main, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
}

void OSD_Menu_Info::process_ca()
{
    lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_bckg_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(m_footer_opt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_footer_main, LV_OBJ_FLAG_HIDDEN);
    m_ca_info = create_rect(m_bgd, 300, 50, ca_info_w, ca_info_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_ca_info);
    m_rigth_line = create_rect(m_ca_info, 0, 0, 3, ca_info_h, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_rigth_line);
    mb_osd_ca_info = std::make_unique<OSD_Ca_Info>(this);
    mb_osd_ca_info->show_box_info(m_ca_info, std::bind(&OSD_Menu_Info::process_ca_callback, this));
}

void OSD_Menu_Info::process_receiver_callback()
{
    // Apaga background e objetos
    DELETE_OBJ(m_rigth_line);
    DELETE_OBJ(m_info_box);
    mb_osd_box_info.reset();
    lv_obj_add_flag(m_bckg_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_footer_opt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(m_footer_main, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
}

void OSD_Menu_Info::process_receiver()
{
    lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_bckg_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(m_footer_opt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_footer_main, LV_OBJ_FLAG_HIDDEN);
    m_info_box = create_rect(m_bgd, 300, 100, info_width, info_heigth, OSD_COLOR_BLACK);
    m_rigth_line = create_rect(m_info_box, 0, 0, 3, info_heigth, OSD_COLOR_ORANGE);
    mb_osd_box_info = std::make_unique<OSD_Box_Info>(this);
    mb_osd_box_info->show_box_info(m_info_box, std::bind(&OSD_Menu_Info::process_receiver_callback, this));
}

void OSD_Menu_Info::show_menu_info(Menu_Info_CB_t _callback, lv_obj_t *_bgd)
{
    set_focus();
    m_callback = _callback;
    m_bgd = _bgd;
    lv_obj_null_on_delete(&m_bgd);
    m_main = create_rect(m_bgd, 0, 100, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    m_left_line = create_rect(m_main, 0, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_left_line);
    m_bckg_fade = create_rect(m_bgd, 0, 100, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bckg_fade);
    lv_obj_set_style_bg_opa(m_bckg_fade, LV_OPA_50, 0);
    lv_obj_add_flag(m_bckg_fade, LV_OBJ_FLAG_HIDDEN);
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    Osd_Breadcrumb::s_instance.add_name(tr(__Inf_do_Receptor));
    m_footer_main = MB_OSD_Footer::draw(m_bgd, tr(__Selecione_a_opcao_desejada_e_pressione_ok_ou_pressione_voltar), -40);
    lv_obj_null_on_delete(&m_footer_main);
    lv_obj_align(m_footer_main, LV_ALIGN_BOTTOM_LEFT, 30, -40);
    m_footer_opt = MB_OSD_Footer::draw(m_bgd, tr(__Pressione_voltar_para_voltar), -40);
    lv_obj_null_on_delete(&m_footer_opt);
    lv_obj_align(m_footer_opt, LV_ALIGN_BOTTOM_LEFT, 130, -40);
    lv_obj_add_flag(m_footer_opt, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Menu_Info::got_focus()
{
}

} // namespace mb
