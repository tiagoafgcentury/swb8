#include "mb_osd_main_menu.h"
#include "mb_osd_menu_home.h"
#include "mb_osd_menu_info.h"
#include "mb_osd_menu_channel_list.h"
#include "mb_osd_menu_ajustes.h"
#include "mb_osd_fade_canvas.h"
#include "mb_menu_resources.h"
#include "mb_menu_data.h"
#include "mb_osd_translate.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "hal/mb_display.h"

#include "tasks/mb_task_player.h"
#include "mb_events.h"

#include <lvgl.h>
#include <math.h>
#include <map>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>

namespace mb {

#ifdef MBGUI_ANIMATION
    constexpr auto START_POS = 160;
    constexpr auto END_POS = DISPLAY_WIDTH / 2;
#endif

OSD_Main_Menu::OSD_Main_Menu(OSD *_parent):
    OSD(_parent)
{
#ifdef MBGUI_ANIMATION
    //lv_anim_init(&m_menu_anim);
#endif
}

OSD_Main_Menu::~OSD_Main_Menu()
{
    m_menu_data.clear();
    DELETE_OBJ(m_main_menu);
}

void OSD_Main_Menu::reset_menu_selection()
{
    m_selected_item = Main_Menu_Options::MENU_HOME;
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);

    for (auto &menu : m_menu_data)
    {
        if (menu.img && menu.img_sel && menu.lbl)
        {
            lv_obj_remove_flag(menu.lbl, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(menu.sel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(menu.img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(menu.img_sel, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void OSD_Main_Menu::clear_btn_selection(uint8_t _item)
{
    lv_obj_remove_flag(m_menu_data[_item].img_sel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_menu_data[_item].sel, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Main_Menu::set_menu_selection(uint8_t _menu, bool _selected)
{
    if (_menu < m_menu_data.size())
    {
        auto &menu = m_menu_data[_menu];

        if (_selected)
        {
            lv_obj_add_flag(menu.img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(menu.img_sel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(menu.sel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(menu.lbl, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_ORANGE, 0);
        }
        else
        {
            lv_obj_remove_flag(menu.img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(menu.img_sel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(menu.lbl, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(menu.sel, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void OSD_Main_Menu::move_selection(uint8_t _from, uint8_t _to)
{
    set_menu_selection(_from, false);
    set_menu_selection(_to, true);
}

int OSD_Main_Menu::add_menu(std::string_view _title, std::string_view _icon, std::string_view _icon_sel)
{
    auto pos = m_menu_data.size();
    auto y = m_mn_top + (m_mn_spacing * pos);

    if (pos == 4)
    {
        y += m_mn_spacing;
    }

    auto sel = create_rect(m_bgd_menu, m_mn_left, y, 260, m_mn_logo_heigth, OSD_COLOR_ORANGE);
    lv_obj_set_style_radius(sel, m_mn_logo_heigth / 2, DEFAULT_SELECTOR);
    auto lbl_sel = set_label_text(sel, _title, 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl_sel, LV_ALIGN_LEFT_MID, 65, 0);
    auto lbl = set_label_text(m_bgd_menu, _title, m_mn_left + 80, y + 10, font_25, OSD_COLOR_WHITE);
    auto img = load_image(m_bgd_menu, _icon, m_mn_left, y, m_mn_logo_width, m_mn_logo_heigth);
    auto img_sel = load_image(m_bgd_menu, _icon_sel, m_mn_left, y, m_mn_logo_width, m_mn_logo_heigth);
    lv_obj_add_flag(sel, LV_OBJ_FLAG_HIDDEN);
    m_menu_data.emplace_back(sel, img, img_sel, lbl);
    m_menu_labels.emplace_back(lbl, lbl_sel);
    return pos;
}

void OSD_Main_Menu::show_main_menu(lv_obj_t *_bgd, OSD_Main_Menu_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    m_bgd = _bgd;

    if (m_main_menu == nullptr)
    {
        //MainMenu
        m_main_menu = create_rect(m_bgd, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_main_menu);
        lv_obj_set_style_bg_opa(m_main_menu, LV_OPA_TRANSP, 0);
        m_bgd_menu = create_rect(m_main_menu, 0, 0, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_menu);
        m_bgd_fade = create_rect(m_main_menu, 0, 170, 150, 520, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_fade);
        lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
        m_logo_century_c = load_image(m_bgd_menu, LOGO_MENU_CENTURY_C_CINZA, m_mn_left, 42, 50, 51);
        lv_obj_null_on_delete(&m_logo_century_c);
        m_logo_century = load_image(m_bgd_menu, LOGO_MENU_CENTURY, m_mn_left, 42, 213, 51);
        lv_obj_null_on_delete(&m_logo_century);
        lv_obj_add_flag(m_logo_century, LV_OBJ_FLAG_HIDDEN);
        add_menu(tr(__Inicio), LOGO_MENU_INICIO, LOGO_MENU_INICIO_SELECAO);
        add_menu(tr(__Canais_de_TV), LOGO_MENU_CANAIS_DE_TV, LOGO_MENU_CANAIS_DE_TV_SELECAO);
        add_menu(tr(__Canais_de_Radio), LOGO_MENU_CANAIS_DE_RADIOS, LOGO_MENU_CANAIS_DE_RADIOS_SELECAO);
        add_menu(tr(__Multimidia), LOGO_MENU_MULTIMIDIA, LOGO_MENU_MULTIMIDIA_SELECAO);
        add_menu(tr(__Ajustes), LOGO_MENU_AJUSTES, LOGO_MENU_AJUSTES_SELECAO);
    }
    else
    {
        lv_obj_add_flag(m_logo_century, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_background(m_bgd_menu);
    }

    if (Lineup_Mutex_Ref::is_empty())
    {
        enter_menu_ajustes();
        reset_menu_selection();
        clear_btn_selection(4);
    }
    else
    {
        enter_menu_home();
        reset_menu_selection();
        move_selection(0, 0);
        clear_btn_selection(0);
    }

    DEBUG_MSG(OSD, DEBUG, "SHOW MENU\n");
}

void OSD_Main_Menu::move_menu_up()
{
    const auto current_pos = static_cast<int>(m_selected_item);
    m_selected_item = static_cast<Main_Menu_Options>((current_pos + m_menu_data.size() - 1) % m_menu_data.size());
    move_selection(current_pos, static_cast<int>(m_selected_item));
}

void OSD_Main_Menu::move_menu_down()
{
    const auto current_pos = static_cast<int>(m_selected_item);
    m_selected_item = static_cast<Main_Menu_Options>((current_pos + 1) % m_menu_data.size());
    move_selection(current_pos, static_cast<int>(m_selected_item));
}

void OSD_Main_Menu::enter_menu_home()
{
    if (!m_osd_menu_home)
    {
        m_osd_menu_home = std::make_unique<OSD_Menu_Home>(this);
    }

    lv_obj_set_size(m_bgd_menu, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT);
    m_osd_menu_home->show_menu_home(std::bind(&OSD_Main_Menu::enter_menu_home_callback, this, std::placeholders::_1), m_main_menu);
}

void OSD_Main_Menu::enter_menu_home_callback(bool _result)
{
    if (_result)
    {
        m_osd_menu_home.reset();
        //Task::post_event(m_callback);
        //remove_focus();
    }
}

void OSD_Main_Menu::enter_menu_multimidia()
{
    Task::post_event_player_stop();
    Task::post_event_display_clear();

    if (not Lineup_Mutex_Ref::is_empty())
    {
        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Fullscreen);
    }

    if (!m_osd_menu_multimedia)
    {
        m_osd_menu_multimedia = std::make_unique<OSD_Menu_Multimidia>(this);
    }

    m_osd_menu_multimedia->show_menu_multimidia(std::bind(&OSD_Main_Menu::enter_menu_multimidia_callback, this), m_bgd);
}

void OSD_Main_Menu::enter_menu_multimidia_callback()
{
    Task::post_event_display_clear();
    Task::post_event_player_restart();

    if (not Lineup_Mutex_Ref::is_empty())
    {
        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Q4);
    }

    m_osd_menu_multimedia.reset();
}

void OSD_Main_Menu::show_menu_channel_list()
{
    mb_osd_menu_channel_list.reset();
}

void OSD_Main_Menu::enter_menu_channels_edit(OSD_Channels_List_Type _channel_lits_type)
{
    lv_obj_set_size(m_bgd_menu, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT);

    if (!mb_osd_menu_channel_list)
    {
        mb_osd_menu_channel_list = std::make_unique<OSD_Menu_Channel_List>(this);
    }

    lv_obj_remove_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    mb_osd_menu_channel_list->show_menu_channel_list(std::bind(&OSD_Main_Menu::show_menu_channel_list, this), _channel_lits_type);
}

void OSD_Main_Menu::enter_menu_ajustes()
{
    bool original_lang = OSD_Translate::is_portuguese();
    Task::post_event_player_stop();
    auto callback = [ =, this](bool _result)
    {
        if (original_lang != OSD_Translate::is_portuguese())
        {
            lv_label_set_text(m_menu_labels[0].first, tr(__Inicio).data());
            lv_label_set_text(m_menu_labels[1].first, tr(__Canais_de_TV).data());
            lv_label_set_text(m_menu_labels[2].first, tr(__Canais_de_Radio).data());
            lv_label_set_text(m_menu_labels[3].first, tr(__Multimidia).data());
            lv_label_set_text(m_menu_labels[4].first, tr(__Ajustes).data());
            lv_label_set_text(m_menu_labels[0].second, tr(__Inicio).data());
            lv_label_set_text(m_menu_labels[1].second, tr(__Canais_de_TV).data());
            lv_label_set_text(m_menu_labels[2].second, tr(__Canais_de_Radio).data());
            lv_label_set_text(m_menu_labels[3].second, tr(__Multimidia).data());
            lv_label_set_text(m_menu_labels[4].second, tr(__Ajustes).data());
        }

        mb_osd_menu_ajustes.reset();

        Task::post_event_player_restart();

        if (_result)
        {
            Task::post_event(m_callback);
            remove_focus();
        }
    };

    if (!mb_osd_menu_ajustes)
    {
        mb_osd_menu_ajustes = std::make_unique<OSD_Menu_Ajustes>(this);
    }

    lv_obj_remove_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    mb_osd_menu_ajustes->show_menu_ajustes(callback);
}

void OSD_Main_Menu::set_main_menu()
{
    //menu_anim(MENU_DECREASE);
    lv_obj_add_flag(m_logo_century, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(m_bgd_menu);
    clear_btn_selection(static_cast<int>(m_selected_item));

    switch (m_selected_item)
    {
        case Main_Menu_Options::MENU_HOME:
        {
            enter_menu_home();
            break;
        }

        case Main_Menu_Options::MENU_TV_CHANNEL:
        {
            enter_menu_channels_edit(OSD_Channels_List_Type::TV_Channels);
            break;
        }

        case Main_Menu_Options::MENU_RADIO_CHANNEL:
        {
            enter_menu_channels_edit(OSD_Channels_List_Type::Radio_Channels);
            break;
        }

        case Main_Menu_Options::MENU_MIDIA:
        {
            enter_menu_multimidia();
            break;
        }

        case Main_Menu_Options::MENU_AJUSTES:
        {
            enter_menu_ajustes();
            break;
        }
    }
}

bool OSD_Main_Menu::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if (lv_obj_has_flag(m_bgd, LV_OBJ_FLAG_HIDDEN))
    {
        return false;
    }

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_MENU:
        case Remote_Control_Key::KEY_VOLTAR:
            if (m_osd_menu_home)
            {
                m_osd_menu_home.reset();
            }

            Task::post_event(m_callback);
            remove_focus();
            return true;

        case Remote_Control_Key::KEY_OK:
            set_main_menu();
            return true;

        case Remote_Control_Key::KEY_CHUP:
            move_menu_up();
            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            move_menu_down();
            return true;

        case Remote_Control_Key::KEY_VOLUP:
            set_main_menu();
            //move_menu_right();
            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            //move_menu_left();
            return true;

        default:
            return true;
    }

    return false;
}

#ifdef MBGUI_ANIMATION
void OSD_Main_Menu::menu_anim(Main_Menu_Anim _menu_anim)
{
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&m_menu_anim, (lv_anim_exec_xcb_t) lv_obj_set_width);
    /*Set target of the animation*/
    lv_anim_set_var(&m_menu_anim, m_bgd_menu);
    /*Length of the animation [ms]*/
    lv_anim_set_time(&m_menu_anim, 500);

    /*Set start and end values. E.g. 0, 150*/
    if (_menu_anim == MENU_EXPAND)
    {
        lv_anim_set_values(&m_menu_anim, START_POS, END_POS);
    }
    else
    {
        lv_anim_set_values(&m_menu_anim, END_POS, START_POS);
    }

    /*Set path (curve). Default is linear*/
    lv_anim_set_path_cb(&m_menu_anim, lv_anim_path_ease_in);
    lv_anim_start(&m_menu_anim);
}

#endif

void OSD_Main_Menu::got_focus()
{
    if (m_bgd_menu)
    {
        lv_obj_set_size(m_bgd_menu, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        reset_menu_selection();
        move_selection(0, 0);
        lv_obj_remove_flag(m_logo_century, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(m_bgd_menu);
#ifdef MBGUI_ANIMATION
        //menu_anim(MENU_EXPAND);
#endif
    }
}

} // namespace mb
