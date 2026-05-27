#include "mb_osd_menu_update.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_password.h"
#include <lvgl.h>

namespace {
}

namespace mb {

OSD_Menu_Update::OSD_Menu_Update(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        "Software",
        //tr(__Lista_de_Canais),
    };
}

OSD_Menu_Update::~OSD_Menu_Update()
{
    DELETE_OBJ(m_footer);
    DELETE_OBJ(m_bgd_main);
    DELETE_OBJ(m_main_box);
    // Retira item do caminho de migalhas
    Osd_Breadcrumb::s_instance.remove_name();
    // Remove foco do controle remoto
    remove_focus();
}

void OSD_Menu_Update::show_menu_update_usb_satellite_callback()
{
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    m_update_usb_satellite.reset();
}

#if 0
void OSD_Menu_Update::show_menu_channel_list_update_callback()
{
    m_osd_channel_list_update.reset();
}

#endif

void OSD_Menu_Update::hide_channel_list_screen()
{
    Osd_Breadcrumb::s_instance.remove_name();
    m_is_channel_list_screen = false;
    lv_obj_add_flag(m_bgd_main, LV_OBJ_FLAG_HIDDEN);
}

// Processa tecla recebida
bool OSD_Menu_Update::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Tecla OK: exibe lista de satélites
    if (_event.key == Remote_Control_Key::KEY_OK)
    {
        if (m_is_channel_list_screen == false)
        {
            auto active = static_cast<Func_Active>(m_keys.get_selected());

            switch (active)
            {
                case Func_Active::Software:
                {
                    lv_obj_remove_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_GREY, DEFAULT_SELECTOR);
                    m_update_usb_satellite = std::make_unique<OSD_Menu_Update_Usb_Satellite>(this);
                    m_update_usb_satellite->show_menu_update_usb_satellite(std::bind(&OSD_Menu_Update::show_menu_update_usb_satellite_callback, this), m_bgd);
                    break;
                }
#if 0
                case Func_Active::List:
                {
                    Osd_Breadcrumb::s_instance.add_name(tr(__Atualizacao_de_Lista_de_Canais));
                    m_is_channel_list_screen = true;
                    lv_obj_remove_flag(m_bgd_main, LV_OBJ_FLAG_HIDDEN);
                    break;
                }
#endif
                case Func_Active::COUNT:
                    break;
            }
        }
        else
        {
            hide_channel_list_screen();
        }

        return true;
    }

    // Pressionado "Voltar", retorna com false
    if (_event.key == Remote_Control_Key::KEY_VOLTAR)
    {
        if (m_is_channel_list_screen)
        {
            hide_channel_list_screen();
            return true;
        }
        else
        {
            Task::post_event(m_callback);
            return true;
        }
    }

    // Seleciona próximo botão
    if (_event.key == Remote_Control_Key::KEY_CHDOWN ||
            _event.key == Remote_Control_Key::KEY_CHUP)
    {
        if (m_is_channel_list_screen == false)
        {
            m_keys.next();
            //auto active = static_cast<Func_Active>(m_keys.get_selected());
            // Substitui item do caminho de migalhas
            Osd_Breadcrumb::s_instance.remove_name();
#if 0
            if (active == Func_Active::List)
            {
                Osd_Breadcrumb::s_instance.add_name(tr(__Lista_de_Canais));
            }
            else
#endif
            {
                Osd_Breadcrumb::s_instance.add_name("Software");
            }
        }
    }

    return true;
}

void OSD_Menu_Update::show_menu_update(Menu_Update_CB_t _callback, lv_obj_t *bgd)
{
    // Direciona recepção de tecla
    DEBUG_MSG(OSD, DEBUG, "show_menu_update()\n");
    set_focus();
    m_bgd = bgd;
    m_callback = _callback;
    // Cria área do menu
    //m_main_box = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), offset_x, 180, width, heigth, OSD_COLOR_BLACK);
    m_main_box = create_rect(m_bgd, 0, 80, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_box);
    // Desenha teclado
    m_keys.set_background(m_main_box);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    // Desenha linha vertical
    m_line_left = create_rect(m_main_box, 0, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_left);
    m_line_right = create_rect(m_main_box, width - 3, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_right);
    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    // Cria cortina sobe o menu que fica inicialmente apagada
    m_bgd_fade = create_rect(m_main_box, 0, 0, width - 3, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_fade);
    lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    // Preenche caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name("Software");
    // Cria rodapé
    m_footer = MB_OSD_Footer::draw(m_bgd, tr(__Selecione_a_opcao_desejada_e_pressione_ok_ou_pressione_voltar), -40);
    lv_obj_null_on_delete(&m_footer);
    lv_obj_align(m_footer, LV_ALIGN_BOTTOM_LEFT, 10, -40);
    //create_satellite_list_unavailable();
}

void OSD_Menu_Update::create_satellite_list_unavailable()
{
    m_bgd_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 180, DISPLAY_WIDTH, DISPLAY_HEIGHT - 180, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_main);
    auto title = set_label_text(m_bgd_main, tr(__Atualizacao_de_Lista_de_Canais), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
    auto subtitle = set_label_text(m_bgd_main, tr(__Nao_ha_atualizacao_disponivel_no_momento), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 67);
    // Desenha linha vertical
    auto button = create_rect(m_bgd_main, 0, 0, 220, 50, OSD_COLOR_ORANGE);
    lv_obj_set_style_radius(button, 25, DEFAULT_SELECTOR);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -100);
    auto lbl_ok = set_label_text(button, tr(__Voltar), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl_ok, LV_ALIGN_CENTER, 0, 0);
    MB_OSD_Footer::draw(m_bgd_main, tr(__Pressione_ok_para_continuar), footer_y);
    lv_obj_add_flag(m_bgd_main, LV_OBJ_FLAG_HIDDEN);
}

} // namespace mb
