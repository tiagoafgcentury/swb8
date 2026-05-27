#include "mb_osd_media_player.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"

#include "hal/mb_system.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_application.h"

#include <lvgl.h>
#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <time.h>

namespace {

static constexpr std::array<std::string_view, 6> logo_mp_functions =
{
    LOGO_MEDIA_PLAYER_PREVIOUS_34X34,
    LOGO_MEDIA_PLAYER_REWIND_34X34,
    LOGO_MEDIA_PLAYER_PLAY_34X34,
    LOGO_MEDIA_PLAYER_STOP_34X34,
    LOGO_MEDIA_PLAYER_FORWARD_34X34,
    LOGO_MEDIA_PLAYER_NEXT_34X34,
};

static constexpr std::array<std::string_view, 6> logo_mp_functions_sel =
{
    LOGO_MEDIA_PLAYER_PREVIOUS_SEL_34X34,
    LOGO_MEDIA_PLAYER_REWIND_SEL_34X34,
    LOGO_MEDIA_PLAYER_PLAY_SEL_34X34,
    LOGO_MEDIA_PLAYER_STOP_SEL_34X34,
    LOGO_MEDIA_PLAYER_FORWARD_SEL_34X34,
    LOGO_MEDIA_PLAYER_NEXT_SEL_34X34,
};

static constexpr std::array<std::string_view, 2> logo_mp_play_pause =
{
    LOGO_MEDIA_PLAYER_PLAY_34X34,
    LOGO_MEDIA_PLAYER_PAUSE_34X34,
};

static constexpr std::array<std::string_view, 7> logo_mp_fr =
{
    LOGO_MEDIA_PLAYER_REWIND_SEL_34X34,
    LOGO_MEDIA_PLAYER_REWIND_34X34,
    LOGO_MEDIA_PLAYER_SPEED_2X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_4X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_8X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_16X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_24X_34X34,
};

static constexpr std::array<std::string_view, 7> logo_mp_ff =
{
    LOGO_MEDIA_PLAYER_FORWARD_SEL_34X34,
    LOGO_MEDIA_PLAYER_FORWARD_34X34,
    LOGO_MEDIA_PLAYER_SPEED_2X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_4X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_8X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_16X_34X34,
    LOGO_MEDIA_PLAYER_SPEED_24X_34X34,
};

static constexpr std::array<std::string_view, 2> logo_mp_play_pause_sel =
{
    LOGO_MEDIA_PLAYER_PLAY_SEL_34X34,
    LOGO_MEDIA_PLAYER_PAUSE_SEL_34X34,
};

}

namespace mb {


OSD_Media_Player::OSD_Media_Player(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_style_main);
    lv_style_init(&m_style_indicator);
    lv_style_init(&m_label_style);
    lv_style_init(&m_back_style);
    lv_style_init(&m_front_style);
    lv_style_set_bg_opa(&m_style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style_main, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&m_style_main, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&m_style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style_indicator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&m_style_indicator, &m_transition_dsc);
}

// Destrutor
OSD_Media_Player::~OSD_Media_Player()
{
    remove_focus();
    lv_style_reset(&m_style_main);
    lv_style_reset(&m_style_indicator);
    lv_style_reset(&m_back_style);
    lv_style_reset(&m_label_style);
    lv_style_reset(&m_front_style);
    m_playlist.clear();
    DELETE_TIMER(m_player_info_timer);
    DELETE_TIMER(m_hide_timer);
    DELETE_OBJ(m_main_screen);
}

void OSD_Media_Player::reset_menu_selection()
{
    auto max_size = static_cast<uint8_t>(Player_Functions::MAX_PLAYER_FUNCTIONS);

    for (auto i = 0; i < max_size; i++)
    {
        lv_obj_add_flag(m_mp_functions[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(m_mp_functions_sel[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Media_Player::reset_play_pause_btn()
{
    for (auto idx = 0u; idx < m_mp_play_pause.size(); idx ++)
    {
        lv_obj_add_flag(m_mp_play_pause[idx], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_mp_play_pause_sel[idx], LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Media_Player::update_play_pause_btn(uint8_t _menu, bool _selected)
{
    if (_menu == 2)
    {
        reset_play_pause_btn();

        if (m_btn_play_state == Btn_Play_State::PLAY and _selected == false)
        {
            lv_obj_remove_flag(m_mp_play_pause[1], LV_OBJ_FLAG_HIDDEN);
            m_play_ptr = m_mp_play_pause[1];
            m_play_sel_ptr = m_mp_play_pause_sel[1];
        }
        else if (m_btn_play_state == Btn_Play_State::PAUSE and _selected == false)
        {
            lv_obj_remove_flag(m_mp_play_pause[0], LV_OBJ_FLAG_HIDDEN);
            m_play_ptr = m_mp_play_pause[0];
            m_play_sel_ptr = m_mp_play_pause_sel[0];
        }
        else if (m_btn_play_state == Btn_Play_State::PLAY and _selected)
        {
            lv_obj_remove_flag(m_mp_play_pause_sel[1], LV_OBJ_FLAG_HIDDEN);
            m_play_ptr = m_mp_play_pause[1];
            m_play_sel_ptr = m_mp_play_pause_sel[1];
        }
        else if (m_btn_play_state == Btn_Play_State::PAUSE and _selected)
        {
            lv_obj_remove_flag(m_mp_play_pause_sel[0], LV_OBJ_FLAG_HIDDEN);
            m_play_ptr = m_mp_play_pause[0];
            m_play_sel_ptr = m_mp_play_pause_sel[0];
        }
    }
}

void OSD_Media_Player::set_menu_selection(uint8_t _menu, bool _selected)
{
    if (_menu < static_cast<uint8_t>(Player_Functions::MAX_PLAYER_FUNCTIONS))
    {
        update_play_pause_btn(_menu, _selected);

        if (_selected)
        {
            lv_obj_add_flag(m_mp_functions[_menu], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(m_mp_functions_sel[_menu], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_remove_flag(m_mp_functions[_menu], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(m_mp_functions_sel[_menu], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void OSD_Media_Player::move_selection(uint8_t _from, uint8_t _to)
{
    set_menu_selection(_from, false);
    set_menu_selection(_to, true);
}

lv_obj_t *OSD_Media_Player::create_filename_label(lv_obj_t *_bgd)
{
    auto lbl = set_label_text(_bgd, "", 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl, 450);
    lv_obj_set_height(lbl, 70);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    return lbl;
}

void OSD_Media_Player::audio_player_frame()
{
    m_main_screen = create_rect(get_main_screen(OSD_Layer::MEDIA_PLAYER_LAYER), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_screen);
    load_image(m_main_screen, LOGO_MENU_CENTURY, START_POS_X, 42, 213, 51);
    load_image(m_main_screen, LOGO_MENU_CENTURY_Cinza_400x404, 137, 158, 400, 404);
    add_clock(m_main_screen, 300, 320);
    m_back_box = create_rect(m_main_screen, 680, 110, 410, 512, OSD_COLOR_GREY_MEDIUM);
    lv_obj_null_on_delete(&m_back_box);
    lv_obj_set_style_radius(m_back_box, 25, DEFAULT_SELECTOR);
    lv_style_set_outline_width(&m_back_style, 3);
    lv_style_set_outline_color(&m_back_style, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_outline_pad(&m_back_style, 4);
    lv_style_set_radius(&m_back_style, 25);
    lv_style_set_bg_color(&m_back_style, OSD_COLOR_GREY);
    lv_obj_add_style(m_back_box, &m_back_style, 0);
    m_box_transp = create_rect(m_main_screen, 630, 80, 510, 570, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_transp);
    lv_obj_set_style_bg_opa(m_box_transp, LV_OPA_60, 0);
    m_front_box = create_rect(m_box_transp, 0, 0, 490, 270, OSD_COLOR_GREY_DARK);
    lv_obj_null_on_delete(&m_front_box);
    lv_obj_set_align(m_front_box, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(m_front_box, 25, DEFAULT_SELECTOR);
    lv_style_set_outline_width(&m_front_style, 3);
    lv_style_set_outline_color(&m_front_style, OSD_COLOR_ORANGE);
    lv_style_set_outline_pad(&m_front_style, 4);
    lv_style_set_radius(&m_front_style, 25);
    //lv_style_set_bg_color(&m_front_style, OSD_COLOR_GREY);
    lv_obj_add_style(m_front_box, &m_front_style, 0);
    auto logo_midiabox = load_image(m_front_box, LOGO_MIDIABOX_BRANCO_150x22, 0, 0, 150, 22);
    lv_obj_align(logo_midiabox, LV_ALIGN_BOTTOM_MID, 0, -15);
    auto label_curr_box = create_rect(m_front_box, 0, 0, 410, 70, OSD_COLOR_GREY_DARK);
    lv_obj_align(label_curr_box, LV_ALIGN_TOP_MID, 0, 30);
    auto label_prev_box = create_rect(m_back_box, 0, 0, 400, 70, OSD_COLOR_GREY_MEDIUM);
    lv_obj_align(label_prev_box, LV_ALIGN_TOP_MID, 0, 20);
    auto label_next_box = create_rect(m_back_box, 0, 0, 400, 70, OSD_COLOR_GREY_MEDIUM);
    lv_obj_align(label_next_box, LV_ALIGN_BOTTOM_MID, 0, -20);
    m_filename = create_filename_label(label_curr_box);
    m_filename_previous = create_filename_label(label_prev_box);
    m_filename_next = create_filename_label(label_next_box);
#ifdef MBGUI_USE_RLOTTIE
    m_soundwave = lv_rlottie_create_from_file(m_front_box, 307, 200, ANIM_SOUND_WAVE);
    lv_obj_null_on_delete(&m_soundwave);
    lv_obj_align(m_soundwave, LV_ALIGN_DEFAULT, START_POS_X, 28);
    lv_rlottie_set_play_mode(m_soundwave, LV_RLOTTIE_CTRL_LOOP);
    m_music_logo = lv_rlottie_create_from_file(m_main_screen, 100, 100, ANIM_MEDIA_PLAYER_AUDIO);
    lv_obj_null_on_delete(&m_music_logo);
    lv_obj_align(m_music_logo, LV_ALIGN_DEFAULT, 50, DISPLAY_HEIGHT - 150);
    lv_rlottie_set_play_mode(m_music_logo, LV_RLOTTIE_CTRL_LOOP);
#endif
    // Cria barra de progresso
    m_slider_obj = lv_bar_create(m_front_box);
    lv_obj_null_on_delete(&m_slider_obj);
    lv_obj_align(m_slider_obj, LV_ALIGN_DEFAULT, START_POS_X, 125);
    lv_obj_set_width(m_slider_obj, 310);
    show_horizontal_bar();
    // Exibe imagens na barra inferior
    static constexpr auto base_line = 4;
    static constexpr auto x_start = 44;

    m_button_box = create_rect(m_front_box, START_POS_X, 170, 310, 44, OSD_COLOR_GREY_DARK);
    lv_obj_null_on_delete(&m_button_box);
    lv_obj_set_style_bg_opa(m_button_box, LV_OPA_TRANSP, 0);
    for (uint8_t idx = 0; idx < m_mp_play_pause.size(); idx++)
    {
        m_mp_play_pause[idx] = load_image(m_button_box, logo_mp_play_pause[idx].data(), (x_start * 2), base_line, 34, 34);
        lv_obj_null_on_delete(&m_mp_play_pause[idx]);
        m_mp_play_pause_sel[idx] = load_image(m_button_box, logo_mp_play_pause_sel[idx].data(), (x_start * 2), base_line, 34, 34);
        lv_obj_null_on_delete(&m_mp_play_pause_sel[idx]);
    }

    for (uint8_t idx = 0; idx < m_mp_functions.size(); idx++)
    {
        switch (static_cast<Player_Functions>(idx))
        {
            case Player_Functions::PLAYER_PLAY_PAUSE:
                m_play_ptr = m_mp_play_pause[0];
                m_mp_functions[idx] = m_play_ptr;
                m_play_sel_ptr = m_mp_play_pause_sel[0];
                m_mp_functions_sel[idx] = m_play_sel_ptr;
                break;

            case Player_Functions::PLAYER_REWIND:
            case Player_Functions::PLAYER_PREVIOUS:
            case Player_Functions::PLAYER_STOP:
            case Player_Functions::PLAYER_FORWARD:
            case Player_Functions::PLAYER_NEXT:
            default:
                m_mp_functions[idx] = load_image(m_button_box, logo_mp_functions[idx].data(), (x_start * idx), base_line, 34, 34);
                lv_obj_null_on_delete(&m_mp_functions[idx]);
                m_mp_functions_sel[idx] = load_image(m_button_box, logo_mp_functions_sel[idx].data(), (x_start * idx), base_line, 34, 34);
                lv_obj_add_flag(m_mp_functions_sel[idx], LV_OBJ_FLAG_HIDDEN);
                break;
        }
    }

    m_label_speed = set_label_text_static(m_button_box, "", (x_start * 6), base_line + 5, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_label_speed);
    m_file_time = set_label_text_static(m_front_box, "00:00", START_POS_X, 135, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_file_time);
    m_file_time_end = set_label_text_static(m_front_box, "00:00", START_POS_X + 260, 135, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_file_time_end);
}

void OSD_Media_Player::video_player_frame()
{
    m_main_screen = lv_canvas_create(get_main_screen(OSD_Layer::MEDIA_PLAYER_LAYER));
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
    m_logo_century = load_image(top_mask, LOGO_MENU_CENTURY, START_POS_X, 42, 213, 51);
    lv_obj_null_on_delete(&m_logo_century);
    m_filename = set_label_text_static(m_bottom_mask, "", START_POS_X, 60, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_filename);
    // Exibe imagens na barra inferior
    static constexpr auto base_line = 110;
    static constexpr auto x_start = 44;
    load_image(m_bottom_mask, LOGO_MIDIABOX_BRANCO_208x30, 983, 60, 208, 40);

    for (uint8_t idx = 0; idx < m_mp_play_pause.size(); idx++)
    {
        m_mp_play_pause[idx] = load_image(m_bottom_mask, logo_mp_play_pause[idx].data(), START_POS_X + (x_start * 2), base_line, 34, 34);
        lv_obj_null_on_delete(&m_mp_play_pause[idx]);
        m_mp_play_pause_sel[idx] = load_image(m_bottom_mask, logo_mp_play_pause_sel[idx].data(), START_POS_X + (x_start * 2), base_line, 34, 34);
        lv_obj_null_on_delete(&m_mp_play_pause_sel[idx]);
    }

    for (uint8_t idx = 0; idx < m_mp_functions.size(); idx++)
    {
        switch (static_cast<Player_Functions>(idx))
        {
            /*
                        case Player_Functions_t::PLAYER_REWIND:
                            m_mp_functions[idx] = load_image(m_bottom_mask, logo_mp_functions[idx].c_str(), START_POS_X + (x_start * 1), base_line, 34, 34);
                            lv_obj_null_on_delete(&m_mp_functions[idx]);
                            m_rewind_ptr = m_mp_fr_speed[0];
                            m_mp_functions_sel[idx] = m_rewind_ptr;
                            lv_obj_add_flag(m_mp_functions_sel[idx], LV_OBJ_FLAG_HIDDEN);
                            break;
            */
            case Player_Functions::PLAYER_PLAY_PAUSE:
                m_play_ptr = m_mp_play_pause[0];
                m_mp_functions[idx] = m_play_ptr;
                m_play_sel_ptr = m_mp_play_pause_sel[0];
                m_mp_functions_sel[idx] = m_play_sel_ptr;
                break;

            /*
                        case Player_Functions_t::PLAYER_FORWARD:
                            m_mp_functions[idx] = load_image(m_bottom_mask, logo_mp_functions[idx].data(), START_POS_X + (x_start * 1), base_line, 34, 34);
                            lv_obj_null_on_delete(&m_mp_functions[idx]);
                            m_forward_ptr = m_mp_ff_speed[0];
                            m_mp_functions_sel[idx] = m_forward_ptr;
                            lv_obj_add_flag(m_mp_functions_sel[idx], LV_OBJ_FLAG_HIDDEN);
                            break;
            */
            case Player_Functions::PLAYER_REWIND:
            case Player_Functions::PLAYER_PREVIOUS:
            case Player_Functions::PLAYER_STOP:
            case Player_Functions::PLAYER_FORWARD:
            case Player_Functions::PLAYER_NEXT:
            default:
                m_mp_functions[idx] = load_image(m_bottom_mask, logo_mp_functions[idx].data(), START_POS_X + (x_start * idx), base_line, 34, 34);
                lv_obj_null_on_delete(&m_mp_functions[idx]);
                m_mp_functions_sel[idx] = load_image(m_bottom_mask, logo_mp_functions_sel[idx].data(), START_POS_X + (x_start * idx), base_line, 34, 34);
                lv_obj_add_flag(m_mp_functions_sel[idx], LV_OBJ_FLAG_HIDDEN);
                break;
        }
    }

    m_label_speed = set_label_text_static(m_bottom_mask, "24X", START_POS_X + (x_start * 6), base_line + 5, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_label_speed);
    lv_anim_init(&m_animation_template);
    lv_anim_set_delay(&m_animation_template, 1000);
    lv_anim_set_repeat_delay(&m_animation_template, 3000);
    lv_style_set_anim(&m_label_style, &m_animation_template);
    lv_label_set_long_mode(m_filename, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_add_style(m_filename, &m_label_style, LV_STATE_DEFAULT);
    lv_obj_set_width(m_filename, 850);
    m_file_time = set_label_text_static(m_bottom_mask, "", 983, base_line, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_file_time);
    m_slider_obj = lv_bar_create(m_bottom_mask);
    lv_obj_null_on_delete(&m_slider_obj);
    lv_obj_align(m_slider_obj, LV_ALIGN_DEFAULT, START_POS_X, 100);
    lv_obj_set_width(m_slider_obj, 1100);
    show_horizontal_bar();
}

void OSD_Media_Player::midia_player_finish()
{
    del_player_info_timer();
    Task::post_event_media_player_stop();
    Task::post_event(std::bind(m_callback));
}

void OSD_Media_Player::pvr_player_finish()
{
    del_player_info_timer();
    Task::s_task_player->pvr_playback_stop();
    Task::post_event(std::bind(m_callback));
}

bool OSD_Media_Player::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if(_event.key ==  Remote_Control_Key::KEY_POWER)
    {
        if (m_player_type == Player_Type::VIDEO || m_player_type == Player_Type::AUDIO)
        {
            midia_player_finish();
        }
        else
        {
            pvr_player_finish();
        }
        Task::post_event_toggle_power();
    }

    if (m_hide_screen_enabled)
    {
        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLUP:
            case Remote_Control_Key::KEY_VOLDOWN:
            case Remote_Control_Key::KEY_MUTE:
                Task::s_task_application->handle_event_remote_control(_event);
                return true;

            default:
                show_screen();
                return true;
        }
    }
    else
    {

        lv_timer_reset(m_hide_timer);
        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLTAR:
                if (m_player_type == Player_Type::VIDEO || m_player_type == Player_Type::AUDIO)
                {
                    midia_player_finish();
                }
                else
                {
                    pvr_player_finish();
                }

                return true;

            case Remote_Control_Key::KEY_OK:
                switch (static_cast<Player_Functions>(m_selected_item))
                {
                    case Player_Functions::PLAYER_PREVIOUS:
                        previous();
                        break;

                    case Player_Functions::PLAYER_REWIND:
                        rewind();
                        break;

                    case Player_Functions::PLAYER_PLAY_PAUSE:
                        play();
                        break;

                    case Player_Functions::PLAYER_STOP:
                        if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::AUDIO)
                        {
                            midia_player_finish();
                        }
                        else
                        {
                            pvr_player_finish();
                        }

                        break;

                    case Player_Functions::PLAYER_FORWARD:
                        forward();
                        break;

                    case Player_Functions::PLAYER_NEXT:
                        next();
                        break;

                    default:
                        break;
                }

                return true;

            case Remote_Control_Key::KEY_VOLUP:
                move_right();
                return true;

            case Remote_Control_Key::KEY_VOLDOWN:
                move_left();
                return true;

            default:
                return true;
        }
    }

    return false;
}

void OSD_Media_Player::update_midia_informations()
{
    auto _mp_state = Task::s_task_player->get_media_player_state();
    auto _mpvr_state = Task::s_task_player->get_pvr_player_state();

    if (!((_mp_state == Media_Player::State::Paused) or (_mpvr_state == Task_Player::PVR_State::Paused)))
    {
        if (m_player_type == Player_Type::VIDEO || m_player_type == Player_Type::AUDIO)
        {
            m_current_time = Task::s_task_player->get_mp_curr_time();
            m_total_time = Task::s_task_player->get_mp_total_time();
        }
        else
        {
            m_current_time = Task::s_task_player->get_pvr_playback_curr_time();
            m_total_time = Task::s_task_player->get_pvr_playback_total_time();
        }
        if(m_current_time < 1)
        {
            reset_rewind_forward_icon();
        }

        if ((_mp_state != Media_Player::State::Error) or
                (_mpvr_state != Task_Player::PVR_State::Error))
        {
            std::time_t current_time = m_current_time;
            std::time_t total_time = m_total_time;
            char buf1[80];
            char buf2[80];

            if (m_player_type == Player_Type::AUDIO)
            {
                std::strftime(buf1, 80, "%M:%S", localtime(&current_time));
                std::strftime(buf2, 80, "%M:%S", localtime(&total_time));
                lv_label_set_text(m_file_time, buf1);
                lv_label_set_text(m_file_time_end, buf2);
            }
            else
            {
                std::strftime(buf1, 80, "%H:%M:%S", localtime(&current_time));
                std::strftime(buf2, 80, "%H:%M:%S", localtime(&total_time));
                std::string str = std::string(buf1) + std::string(" / ") + std::string(buf2);
                lv_label_set_text(m_file_time, str.c_str());
            }

            double progress = 0;

            if (m_total_time > 0)
            {
                progress = ((100 * m_current_time) / m_total_time);
            }

            // Exibe progresso do programa
            update_horizontal_bar(std::round(progress));

            if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::AUDIO)
            {
                if (_mp_state == Media_Player::State::Finish)
                {
                    midia_player_finish();
                }
            }
            else
            {
                if (_mpvr_state == Task_Player::PVR_State::Started and m_current_time == m_total_time)
                {
                    pvr_player_finish();
                }
            }
        }
    }
}

void OSD_Media_Player::update_filename()
{
    auto _filename = m_playlist[m_current_midia].filename();
    lv_label_set_text(m_filename, _filename.c_str());

    if (m_player_type == Player_Type::AUDIO)
    {
        // Obtendo os índices anterior, atual e próximo
        int previous = (m_current_midia > 0) ? m_current_midia - 1 : m_playlist.size() - 1;  // Se for o primeiro, pega o último
        int next  = (m_current_midia < (int)m_playlist.size() - 1) ? m_current_midia + 1 : 0;  // Se for o último, pega o primeiro
        auto filename_previous = m_playlist[previous].filename();
        lv_label_set_text(m_filename_previous, filename_previous.c_str());
        auto filename_next = m_playlist[next].filename();
        lv_label_set_text(m_filename_next, filename_next.c_str());
    }
}

// Exibe os dados do novo canal sintonizado
void OSD_Media_Player::show_media_player(Osd_Media_Player_CB_t _callback, Player_Type _player_type, std::vector<std::filesystem::path> _playlist, uint _selected)
{
    set_focus();
    DEBUG_MSG(OSD, DEBUG, "Show Video Player: \n");
    m_callback = _callback;
    m_player_type = _player_type;
    m_playlist = std::move(_playlist);
    m_current_midia = _selected;

    if (m_player_type == Player_Type::AUDIO)
    {
        audio_player_frame();
    }
    else
    {
        video_player_frame();
    }

    update_filename();
    update_midia_informations();
    move_selection(0, m_selected_item);
    start_hide_timer();
    play();
}

// Exibe progresso do programa
void OSD_Media_Player::update_horizontal_bar(int progress)
{
    // Atualiza barra de progresso
    lv_slider_set_value(m_slider_obj, progress, LV_ANIM_OFF);
}

// Exibe barra de progresso
void OSD_Media_Player::show_horizontal_bar()
{
    // Posiciona e define o tamanho da barra
    lv_obj_set_height(m_slider_obj, 5);
    lv_obj_add_style(m_slider_obj, &m_style_main, LV_PART_MAIN);
    lv_obj_add_style(m_slider_obj, &m_style_indicator, LV_PART_INDICATOR);
}

void OSD_Media_Player::hide_screen_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    OSD_Media_Player *thiz = static_cast<OSD_Media_Player *>(lv_timer_get_user_data(tm));

    if (thiz->m_player_type == Player_Type::VIDEO or thiz->m_player_type == Player_Type::TS)
    {
        lv_obj_add_flag(thiz->m_main_screen, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        thiz->resize_audio_player_screen(true);
    }

    thiz->m_hide_screen_enabled = true;
    thiz->m_hide_timer = nullptr;
}

void OSD_Media_Player::show_screen()
{
    if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::TS)
    {
        lv_obj_remove_flag(m_main_screen, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        resize_audio_player_screen(false);
    }
    m_hide_screen_enabled = false;
    start_hide_timer();
}

void OSD_Media_Player::resize_audio_player_screen(bool _hidden)
{

    if(_hidden)
    {
        lv_obj_set_size(m_back_box, 410, 468);
        lv_obj_align(m_back_box, LV_ALIGN_DEFAULT, 680, 132);
        lv_obj_set_size(m_box_transp, 510, 526);
        lv_obj_align(m_box_transp, LV_ALIGN_DEFAULT, 630, 102);
        lv_obj_set_size(m_front_box, 490, 226);
        lv_obj_set_align(m_front_box, LV_ALIGN_CENTER);
        lv_obj_add_flag(m_button_box, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_set_size(m_back_box, 410, 512);
        lv_obj_align(m_back_box, LV_ALIGN_DEFAULT, 680, 110);
        lv_obj_set_size(m_box_transp, 510, 570);
        lv_obj_align(m_box_transp, LV_ALIGN_DEFAULT, 630, 80);
        lv_obj_set_size(m_front_box, 490, 270);
        lv_obj_set_align(m_front_box, LV_ALIGN_CENTER);
        lv_obj_remove_flag(m_button_box, LV_OBJ_FLAG_HIDDEN);
    }
}


void OSD_Media_Player::start_hide_timer()
{
    // Inicia timer para apagar informações do canal
    if (m_hide_timer == nullptr)
    {
        m_hide_timer = lv_timer_create(hide_screen_cb, 5000, this);
        lv_timer_set_repeat_count(m_hide_timer, 1);
    }
    else
    {
        lv_timer_reset(m_hide_timer);
    }
}

void OSD_Media_Player::player_info_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    OSD_Media_Player *thiz = static_cast<OSD_Media_Player *>(lv_timer_get_user_data(tm));
    thiz->update_midia_informations();
}

void OSD_Media_Player::start_player_info_timer()
{
    // Inicia timer para ler informaçõesdo do video
    if (m_player_info_timer == nullptr)
    {
        m_player_info_timer = lv_timer_create(player_info_cb, 1000, this);
        lv_timer_set_repeat_count(m_player_info_timer, -1);
    }
    else
    {
        lv_timer_reset(m_player_info_timer);
    }
}

void OSD_Media_Player::del_player_info_timer()
{
    DELETE_TIMER(m_player_info_timer);
}

void OSD_Media_Player::play()
{
    if (m_playlist.empty())
    {
        return;
    }

    reset_rewind_forward_icon();
    auto _mp_state = Task::s_task_player->get_media_player_state();
    auto _mp_pvr_state = Task::s_task_player->get_pvr_player_state();

    if ((_mp_state == Media_Player::State::Started or _mp_state == Media_Player::State::Starting) or
        (_mp_pvr_state == Task_Player::PVR_State::Started or _mp_pvr_state == Task_Player::PVR_State::Starting))
    {
        lv_obj_add_flag(m_mp_play_pause[0], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_mp_play_pause[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_mp_play_pause_sel[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(m_mp_play_pause_sel[0], LV_OBJ_FLAG_HIDDEN);
        m_play_ptr = m_mp_play_pause[0];
        m_play_sel_ptr = m_mp_play_pause_sel[0];
        pause();
        m_btn_play_state = Btn_Play_State::PAUSE;
    }
    else if ((_mp_state == Media_Player::State::Paused or _mp_state == Media_Player::State::Pausing) or
             (_mp_pvr_state == Task_Player::PVR_State::Paused or _mp_pvr_state == Task_Player::PVR_State::Pausing))
    {
        lv_obj_add_flag(m_mp_play_pause[0], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_mp_play_pause[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_mp_play_pause_sel[0], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(m_mp_play_pause_sel[1], LV_OBJ_FLAG_HIDDEN);
        m_play_ptr = m_mp_play_pause[0];
        m_play_sel_ptr = m_mp_play_pause_sel[1];
        resume();
        m_btn_play_state = Btn_Play_State::PLAY;
    }
    else
    {
        update_filename();

        if (m_player_type == Player_Type::VIDEO || m_player_type == Player_Type::AUDIO)
        {
            Task::post_event_media_player_start(m_playlist[m_current_midia], static_cast<uint8_t>(m_player_type));
            DEBUG_MSG(OSD, DEBUG, "Tocando: " << m_playlist[m_current_midia] << "\n");
        }
        else
        {
            Task::post_event_pvr_playback_start(m_playlist[m_current_midia]);
            DEBUG_MSG(OSD, DEBUG, "Tocando: " << m_playlist[m_current_midia] << "\n");
        }
    }

    start_player_info_timer();
}

void OSD_Media_Player::pause()
{
    reset_rewind_forward_icon();

    if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::AUDIO)
    {
        Task::post_event_media_player_pause();
#ifdef MBGUI_USE_RLOTTIE

        if (m_soundwave and m_music_logo)
        {
            lv_rlottie_set_play_mode(m_soundwave, LV_RLOTTIE_CTRL_PAUSE);
            lv_rlottie_set_play_mode(m_music_logo, LV_RLOTTIE_CTRL_PAUSE);
        }

#endif
    }
    else
    {
        Task::post_event_cas_pvr_play_pause();
    }

    DEBUG_MSG(OSD, DEBUG, "Player pausado.\n");
}

void OSD_Media_Player::resume()
{
    reset_rewind_forward_icon();

    if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::AUDIO)
    {
        Task::post_event_media_player_resume();
#ifdef MBGUI_USE_RLOTTIE

        if (m_soundwave and m_music_logo)
        {
            lv_rlottie_set_play_mode(m_soundwave, LV_RLOTTIE_CTRL_LOOP);
            lv_rlottie_set_play_mode(m_music_logo, LV_RLOTTIE_CTRL_LOOP);
        }

#endif
    }
    else
    {
        Task::post_event_cas_pvr_play_resume();
    }

    DEBUG_MSG(OSD, DEBUG, "Player recomeçando.\n");
}

void OSD_Media_Player::next()
{
    reset_rewind_forward_icon();
    reset_play_pause_icon();
#ifdef MBGUI_USE_RLOTTIE

    if (m_soundwave and m_music_logo)
    {
        lv_rlottie_set_play_mode(m_soundwave, LV_RLOTTIE_CTRL_LOOP);
        lv_rlottie_set_play_mode(m_music_logo, LV_RLOTTIE_CTRL_LOOP);
    }

#endif

    if (m_playlist.empty())
    {
        return;
    }

    m_current_midia = (m_current_midia + 1) % m_playlist.size();
    update_filename();

    if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::AUDIO)
    {
        Task::post_event_media_player_next(m_playlist[m_current_midia], static_cast<uint8_t>(m_player_type));
    }
    else
    {
        Task::post_event_cas_pvr_play_next(m_playlist[m_current_midia]);
    }
}

void OSD_Media_Player::previous()
{
    reset_rewind_forward_icon();
    reset_play_pause_icon();
#ifdef MBGUI_USE_RLOTTIE

    if (m_soundwave and m_music_logo)
    {
        lv_rlottie_set_play_mode(m_soundwave, LV_RLOTTIE_CTRL_LOOP);
        lv_rlottie_set_play_mode(m_music_logo, LV_RLOTTIE_CTRL_LOOP);
    }

#endif

    if (m_playlist.empty())
    {
        return;
    }

    m_current_midia = (m_current_midia - 1 + m_playlist.size()) % m_playlist.size();
    update_filename();

    if (m_player_type == Player_Type::VIDEO or m_player_type == Player_Type::AUDIO)
    {
        Task::post_event_media_player_previous(m_playlist[m_current_midia], static_cast<uint8_t>(m_player_type));
    }
    else
    {
        Task::post_event_cas_pvr_play_next(m_playlist[m_current_midia]);
    }
}

void OSD_Media_Player::setRepeat(bool state)
{
    m_repeat_video = state;
}

void OSD_Media_Player::playRandom()
{
    if (m_playlist.empty())
    {
        return;
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    m_current_midia = std::rand() % m_playlist.size();
    play();
}

void OSD_Media_Player::reset_play_pause_icon()
{
    lv_obj_add_flag(m_mp_play_pause[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(m_mp_play_pause[1], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_mp_play_pause_sel[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_mp_play_pause_sel[1], LV_OBJ_FLAG_HIDDEN);
    m_play_ptr = m_mp_play_pause[0];
    m_play_sel_ptr = m_mp_play_pause_sel[1];
    m_btn_play_state = Btn_Play_State::PLAY;
}

void OSD_Media_Player::reset_rewind_forward_icon()
{
    m_forward_speed = 0;
    m_rewind_speed = 0;
    /*
        update_forward_icon();
        update_rewind_icon();
    */
    update_label_speed(static_cast<Speed_State>(m_forward_speed));
    update_label_speed(static_cast<Speed_State>(m_rewind_speed));
}

void OSD_Media_Player::update_label_speed(Speed_State _speed)
{
    switch (_speed)
    {
        case Speed_State::SPEED_2X:
            lv_label_set_text(m_label_speed, "2X");
            break;

        case Speed_State::SPEED_4X:
            lv_label_set_text(m_label_speed, "4X");
            break;

        case Speed_State::SPEED_8X:
            lv_label_set_text(m_label_speed, "8X");
            break;

        case Speed_State::SPEED_16X:
            lv_label_set_text(m_label_speed, "16X");
            break;

        case Speed_State::SPEED_24X:
            lv_label_set_text(m_label_speed, "24X");
            break;

        case Speed_State::NORMAL:
        default:
            lv_label_set_text(m_label_speed, "");
            break;
    }
}

void OSD_Media_Player::update_forward_icon()
{
    auto max_size =  static_cast<uint8_t>(Speed_State::MAX_SPEED_STATE);

    for (auto i = 0; i < max_size; i++)
    {
        lv_obj_add_flag(m_mp_ff_speed[i], LV_OBJ_FLAG_HIDDEN);

        if (m_forward_speed == i)
        {
            lv_obj_add_flag(m_mp_functions[4], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(m_mp_ff_speed[i], LV_OBJ_FLAG_HIDDEN);
            m_forward_ptr = m_mp_ff_speed[i];
        }
    }
}

void OSD_Media_Player::forward()
{
    auto _mp_state = Task::s_task_player->get_media_player_state();
    auto _mp_pvr_state = Task::s_task_player->get_pvr_player_state();

    if (_mp_state == Media_Player::State::Started or _mp_state == Media_Player::State::Starting)
    {
        auto count = 6; //Max forward options
        m_forward_speed = (m_forward_speed + count + 1) % count;
        if (m_player_type == Player_Type::VIDEO)
        {
            Task::post_event_media_player_forward(m_forward_speed, 1);
        }
        else if (m_player_type == Player_Type::AUDIO)
        {
            Task::post_event_media_player_forward(m_forward_speed, 0);
        }

        update_label_speed(static_cast<Speed_State>(m_forward_speed));
    }
    else if (_mp_pvr_state == Task_Player::PVR_State::Started or _mp_pvr_state == Task_Player::PVR_State::Starting)
    {
        auto count = 6; //Max forward options
        m_forward_speed = (m_forward_speed + count + 1) % count;
        Task::post_event_cas_pvr_play_forward(m_forward_speed);
        update_label_speed(static_cast<Speed_State>(m_forward_speed));
    }
}

void OSD_Media_Player::update_rewind_icon()
{
    auto max_size =  static_cast<uint8_t>(Speed_State::MAX_SPEED_STATE);

    for (auto i = 0; i < max_size; i++)
    {
        lv_obj_add_flag(m_mp_fr_speed[i], LV_OBJ_FLAG_HIDDEN);

        if (m_rewind_speed == i)
        {
            lv_obj_add_flag(m_mp_functions[1], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(m_mp_fr_speed[i], LV_OBJ_FLAG_HIDDEN);
            m_rewind_ptr = m_mp_fr_speed[i];
        }
    }
}

void OSD_Media_Player::rewind()
{
    auto _mp_state = Task::s_task_player->get_media_player_state();
    auto _mp_pvr_state = Task::s_task_player->get_pvr_player_state();

    if (_mp_state == Media_Player::State::Started or
            _mp_state == Media_Player::State::Starting)
    {
        auto count = 6; //Max rewind options
        m_rewind_speed = (m_rewind_speed + count + 1) % count;

        if (m_player_type == Player_Type::VIDEO)
        {
            Task::post_event_media_player_rewind(m_rewind_speed, 1);
        }
        else if (m_player_type == Player_Type::AUDIO)
        {
            Task::post_event_media_player_rewind(m_rewind_speed, 0);
        }

        update_label_speed(static_cast<Speed_State>(m_rewind_speed));
        //update_rewind_icon();
    }
    else if (_mp_pvr_state == Task_Player::PVR_State::Started or _mp_pvr_state == Task_Player::PVR_State::Starting)
    {
        auto count = 6; //Max forward options
        m_forward_speed = (m_forward_speed + count + 1) % count;
        Task::post_event_cas_pvr_play_rewind(m_forward_speed);
        update_label_speed(static_cast<Speed_State>(m_forward_speed));
    }
}

void OSD_Media_Player::config()
{
}

void OSD_Media_Player::move_right()
{
    auto currentPos = m_selected_item;
    auto count = static_cast<uint16_t>(Player_Functions::MAX_PLAYER_FUNCTIONS);
    m_selected_item = (m_selected_item + count + 1) % count;
    move_selection(currentPos, m_selected_item);
}

void OSD_Media_Player::move_left()
{
    auto currentPos = m_selected_item;
    auto count = static_cast<uint16_t>(Player_Functions::MAX_PLAYER_FUNCTIONS);
    m_selected_item = (m_selected_item + count - 1) % count;
    move_selection(currentPos, m_selected_item);
}

} // namespace mb
