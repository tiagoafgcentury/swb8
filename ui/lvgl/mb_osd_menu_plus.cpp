#include "mb_osd_menu_plus.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_clock.h"
#include "tasks/mb_task.h"
#include "common/mb_lineup.h"

#include <lvgl.h>
#include <filesystem>
#include <fstream>

namespace {
}

namespace mb {

OSD_Menu_Plus::OSD_Menu_Plus(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
}

OSD_Menu_Plus::~OSD_Menu_Plus()
{
    remove_focus();
    DELETE_OBJ(m_pvr_rec_logo);
    DELETE_OBJ(m_main_screen);
}

bool OSD_Menu_Plus::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        case Remote_Control_Key::KEY_MENU:
            Task::post_event(std::bind(m_callback));
            return true;

        case Remote_Control_Key::KEY_OK:
            if (static_cast<FunctionOption>(m_keys.get_selected()) == FunctionOption::Record)
            {
                check_usb_mounted() ? show_record_screen(Time_Point::max()) : show_message_box();
            }
            else if (static_cast<FunctionOption>(m_keys.get_selected()) == FunctionOption::Timeshift)
            {
                check_usb_mounted() ? show_timeshift_screen() : show_message_box();
            }
            else
            {
                show_sleep_screen();
            }

            return true;

        case Remote_Control_Key::KEY_VOLUP:
            m_keys.next();
            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            m_keys.previous();
            return true;

        default:
            return true;
    }

    return false;
}

void OSD_Menu_Plus::pvr_screen_base_frame(const Service *srv)
{
    if (m_main_screen == nullptr)
    {
        // Cria área da tela
        m_main_screen = lv_canvas_create(get_main_screen(OSD_Layer::MAIN_MENU));
        lv_obj_null_on_delete(&m_main_screen);
        // Cria áreas de transparência superior e inferior
        std::tie(m_top_rect, m_bottom_rect) = Fade_Canvas::make_info_mask(m_main_screen);
        // Desenha área superior da tela de informações do canal
        lv_obj_set_pos(m_top_rect->canvas, 0, TOPAREA_Y1 + (DISPLAY_HEIGHT / 8));
        auto top = create_rect(m_main_screen, AREA_X, TOPAREA_Y1, AREA_WIDTH, AREA_HEIGHT / 2, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(top, 202, LV_PART_MAIN);
        auto top_mask = create_rect(m_main_screen, AREA_X, TOPAREA_Y1, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_GREEN);
        lv_obj_set_style_bg_opa(top_mask, 0, LV_PART_MAIN);
        // Desenhar retângulo na base da tela
        lv_obj_set_pos(m_bottom_rect->canvas, 0, BOTTOMAREA_Y1);
        auto bottom = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 + AREA_HEIGHT / 2, AREA_WIDTH, AREA_HEIGHT / 2, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(bottom, 202, LV_PART_MAIN);
        m_bottom_mask = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_GREEN);
        lv_obj_null_on_delete(&m_bottom_mask);
        lv_obj_set_style_bg_opa(m_bottom_mask, 0, LV_PART_MAIN);

        // Área de nome e número do canal
        if (srv)
        {
            m_ch_name = set_label_text_static(top_mask, srv->name().data(), START_POS_X, 30, font_semi_40, OSD_COLOR_WHITE);
            m_ch_info = set_label_text_static(top_mask, "", START_POS_X, 78, font_20, OSD_COLOR_WHITE);
            char channel_number[20] = {0};
            int ch = srv->viewer_channel();
            int w = (ch >= 10000) ? 5 : 4;

            snprintf(channel_number, sizeof(channel_number), "%s %0*d", tr(__Canal).data(), w, ch);

            if (srv->is_favorite())
            {
                auto fav_top = load_image(top_mask, LOGO_FAVORITOS_BRANCO_17x15, 0, 0, 17, 15);
                std::string strCanal = "      " + std::string(channel_number);
                lv_label_set_text(m_ch_info, strCanal.c_str());
                lv_obj_set_pos(fav_top, START_POS_X, 82);
            }
            else
            {
                std::string strCanal = std::string(channel_number);
                lv_label_set_text(m_ch_info, strCanal.c_str());
            }
        }

        // Adiciona relógio
        add_clock(top_mask, 1090, 28);
#ifdef MBGUI_USE_RLOTTIE
        m_pvr_rec_logo = lv_rlottie_create_from_file(get_main_screen(OSD_Layer::MAIN_MENU), 100, 100, ANIM_PVR_VIDEO_REC);
        lv_obj_null_on_delete(&m_pvr_rec_logo);
        lv_obj_align(m_pvr_rec_logo, LV_ALIGN_TOP_LEFT, 90, 600);
        lv_rlottie_set_play_mode(m_pvr_rec_logo, LV_RLOTTIE_CTRL_PAUSE);
        lv_obj_add_flag(m_pvr_rec_logo, LV_OBJ_FLAG_HIDDEN);
#endif
    }
}

void OSD_Menu_Plus::show_menu_plus(const Service *srv,  Plus_CB_t _callback, bool _call_record_program, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    // Direciona recepção de tecla e função de retorno
    set_focus();
    m_callback = _callback;
    m_srv_atual = srv;
    pvr_screen_base_frame(srv);
    m_box_main = create_rect(m_bottom_mask, 0, 0, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_main);
    lv_obj_set_style_bg_opa(m_box_main, 0, LV_PART_MAIN);
    // Desenha botões
    m_keys.clear();
    m_keys.set_background(m_box_main);

    for (const auto &option : m_options)
    {
        m_keys.add_label(option.text);
    }

    m_keys.set_horizontal();
    m_keys.draw();
    //m_keys.set_disabled(static_cast<size_t>(FunctionOption::Timeshift));
    m_keys.select();
    // Cria rodapé
    MB_OSD_Footer::draw(m_box_main, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -30);

    if(_call_record_program)
    {

        if(std::chrono::system_clock::to_time_t(_time_to_end) > 0 and _time_to_end < Time_Point::max())
        {
            m_auto_exit = true;
        }
        show_record_screen(_time_to_end);
    }

    if(_call_sleep_timer)
    {
        m_keys.next();
        show_sleep_screen();
    }

}

void OSD_Menu_Plus::show_message_box()
{
    if (!m_messageBox)
    {
        m_messageBox = std::make_unique<OSD_Message_Box>(nullptr);
    }

    m_messageBox->show_message_box_ok(std::bind(&OSD_Menu_Plus::message_box_callback, this, std::placeholders::_1), tr(__Nenhum_dispositivo_USB_encontrado));
}

void OSD_Menu_Plus::show_record_screen(std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    if (!m_menuPlusRecord)
    {
        m_menuPlusRecord = std::make_unique<OSD_Menu_Plus_Record>(nullptr);
    }

    lv_obj_add_flag(m_box_main, LV_OBJ_FLAG_HIDDEN);
    m_menuPlusRecord->show_menu_plus_record(m_main_screen, m_srv_atual, _time_to_end, std::bind(&OSD_Menu_Plus::record_menu_callback, this));//, std::placeholders::_1));
#ifdef MBGUI_USE_RLOTTIE
    lv_rlottie_set_play_mode(m_pvr_rec_logo, LV_RLOTTIE_CTRL_LOOP);
    lv_obj_remove_flag(m_pvr_rec_logo, LV_OBJ_FLAG_HIDDEN);
#endif
}

void OSD_Menu_Plus::show_timeshift_screen()
{
    if (!m_menuPlusTimeshift)
    {
        m_menuPlusTimeshift = std::make_unique<OSD_Menu_Plus_Timeshift>(nullptr);
    }

    lv_obj_add_flag(m_box_main, LV_OBJ_FLAG_HIDDEN);
    m_menuPlusTimeshift->show_menu_plus_timeshift(m_main_screen, m_srv_atual, std::bind(&OSD_Menu_Plus::timeshift_menu_callback, this));
}


void OSD_Menu_Plus::show_sleep_screen()
{
    if (!m_menu_plus_sleep)
    {
        m_menu_plus_sleep = std::make_unique<OSD_Menu_Plus_Sleep>(nullptr);
    }

    m_auto_exit = true;
    lv_obj_add_flag(m_box_main, LV_OBJ_FLAG_HIDDEN);
    m_menu_plus_sleep->show_menu_plus_sleep(m_main_screen, std::bind(&OSD_Menu_Plus::sleep_menu_callback, this));
}

void OSD_Menu_Plus::message_box_callback(bool)
{
    m_messageBox.reset();
}

void OSD_Menu_Plus::record_menu_callback()
{
    m_menuPlusRecord.reset();
    lv_obj_remove_flag(m_box_main, LV_OBJ_FLAG_HIDDEN);
#ifdef MBGUI_USE_RLOTTIE
    lv_rlottie_set_play_mode(m_pvr_rec_logo, LV_RLOTTIE_CTRL_PAUSE);
    lv_obj_add_flag(m_pvr_rec_logo, LV_OBJ_FLAG_HIDDEN);
#endif
    if(m_auto_exit)
    {
        Task::post_event(std::bind(m_callback));
    }
}

void OSD_Menu_Plus::timeshift_menu_callback()
{
    m_menuPlusTimeshift.reset();
    lv_obj_remove_flag(m_box_main, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Menu_Plus::sleep_menu_callback()
{
    m_menu_plus_sleep.reset();
    lv_obj_remove_flag(m_box_main, LV_OBJ_FLAG_HIDDEN);
    if(m_auto_exit)
    {
        Task::post_event(std::bind(m_callback));
    }
}

bool OSD_Menu_Plus::check_usb_mounted()
{
    std::ifstream mounts("/proc/mounts");
    std::string line;

    while (std::getline(mounts, line))
    {
        if (line.find(USB_PATH) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

}
