#include "mb_osd.h"
#include "mb_osd_radio_icon.h"
#include "mb_osd_translate.h"
#include "mb_zone_id.h"
#include "mb_osd_fonts.h"
#include "common/mb_config.h"

#include <lvgl.h>

namespace mb {

OSD_Radio_Icon::OSD_Radio_Icon(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_back_style);
    lv_style_init(&m_front_style);
    lv_style_set_bg_opa(&m_back_style, LV_OPA_COVER);
    lv_style_set_bg_color(&m_back_style, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&m_back_style, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&m_front_style, LV_OPA_COVER);
    lv_style_set_bg_color(&m_front_style, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_front_style, LV_RADIUS_CIRCLE);
}

OSD_Radio_Icon::~OSD_Radio_Icon()
{
    hide_radio_icon();
}

void OSD_Radio_Icon::hide_radio_icon()
{
    lv_style_reset(&m_back_style);
    lv_style_reset(&m_front_style);
    DELETE_OBJ(m_soundwave);
    DELETE_OBJ(m_music_logo);
    DELETE_OBJ(m_main_screen);
}

void OSD_Radio_Icon::show_radio_icon(std::string_view _current_service_name, std::string_view _next_service_name, std::string_view _previous_service_name)
{
    if (!m_main_screen)
    {
        create_main_screen();
    }

    update_radio_names(_current_service_name, _next_service_name, _previous_service_name);
}

void OSD_Radio_Icon::create_main_screen()
{
    m_main_screen = create_rect(get_main_screen(OSD_Layer::RADIO_INFO), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_main_screen, LV_OPA_60, 0);
    lv_obj_null_on_delete(&m_main_screen);
    load_image(m_main_screen, LOGO_MENU_CENTURY, START_POS_X, 42, 213, 51);
    load_image(m_main_screen, LOGO_MENU_CENTURY_Cinza_400x404, 137, 158, 400, 404);
    add_clock(m_main_screen, 300, 320);
    m_back_box = create_rect(m_main_screen, 680, 110, 410, 512, OSD_COLOR_GREY_MEDIUM);
    lv_obj_set_style_radius(m_back_box, 25, DEFAULT_SELECTOR);
    lv_style_set_outline_width(&m_back_style, 3);
    lv_style_set_outline_color(&m_back_style, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_outline_pad(&m_back_style, 4);
    lv_style_set_radius(&m_back_style, 25);
    lv_style_set_bg_color(&m_back_style, OSD_COLOR_GREY);
    lv_obj_add_style(m_back_box, &m_back_style, 0);

    auto box_transp = create_rect(m_main_screen, 630, 80, 510, 570, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(box_transp, LV_OPA_60, 0);

    auto _up_img = load_image(box_transp, LOGO_ARROW_DOWN_27x27, 0, 0, 27, 27);
    lv_img_set_angle(_up_img, 1800);
    lv_img_set_pivot(_up_img, 13, 13);
    lv_obj_align(_up_img, LV_ALIGN_TOP_MID, 0, 105);
    auto _down_img = load_image(box_transp, LOGO_ARROW_DOWN_27x27, 0, 0, 27, 27);
    lv_obj_align(_down_img, LV_ALIGN_BOTTOM_MID, 0, -105);

    m_front_box = create_rect(box_transp, 0, 0, 490, 270, OSD_COLOR_GREY_DARK);
    lv_obj_set_align(m_front_box, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(m_front_box, 25, DEFAULT_SELECTOR);
    lv_style_set_outline_width(&m_front_style, 3);
    lv_style_set_outline_color(&m_front_style, OSD_COLOR_ORANGE);
    lv_style_set_outline_pad(&m_front_style, 4);
    lv_style_set_radius(&m_front_style, 25);
    lv_obj_add_style(m_front_box, &m_front_style, 0);

    auto _lr_img = load_image(m_front_box, LOGO_AUDIO_LR_34x34, 0, 0, 40, 40);
    lv_obj_align(_lr_img, LV_ALIGN_BOTTOM_MID, -45, -45);
    auto _info_img = load_image(m_front_box, LOGO_INFO_34x34, 0, 0, 40, 40);
    lv_obj_align(_info_img, LV_ALIGN_BOTTOM_MID, 0, -45);
    auto _mais_img = load_image(m_front_box, LOGO_MAIS_34x34, 0, 0, 40, 40);
    lv_obj_align(_mais_img, LV_ALIGN_BOTTOM_MID, 45, -45);

    auto logo_midiabox = load_image(m_front_box, LOGO_MIDIABOX_BRANCO_150x22, 0, 0, 150, 22);
    lv_obj_align(logo_midiabox, LV_ALIGN_BOTTOM_MID, 0, -15);
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
}

void OSD_Radio_Icon::update_radio_names(std::string_view _current_service_name, std::string_view _next_service_name, std::string_view _previous_service_name)
{
    DELETE_OBJ(m_current_radio_label);
    DELETE_OBJ(m_previous_radio_label);
    DELETE_OBJ(m_next_radio_label);
    m_next_radio_label = set_label_text(m_back_box, _next_service_name, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_radio_label);
    lv_point_t pos;
    lv_text_get_size(&pos, _next_service_name.data(), font_25, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    DEBUG_MSG(OSD, DEBUG, "Next radio label width: " << pos.x << "\n");
    lv_obj_align(m_next_radio_label, LV_ALIGN_TOP_MID, 0, 20);

    if (pos.x > name_width)
    {
        lv_label_set_long_mode(m_next_radio_label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_next_radio_label, name_width);
        lv_obj_set_height(m_next_radio_label, name_height);
    }

    m_current_radio_label = set_label_text(m_front_box, _current_service_name, 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_radio_label);
    lv_text_get_size(&pos, _current_service_name.data(), font_semi_25, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    DEBUG_MSG(OSD, DEBUG, "Current radio label width: " << pos.x << "\n");
    lv_obj_align(m_current_radio_label, LV_ALIGN_TOP_MID, 0, 30);

    if (pos.x > name_width)
    {
        lv_label_set_long_mode(m_current_radio_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(m_current_radio_label, name_width);
        lv_obj_set_height(m_current_radio_label, name_height);
    }

    m_previous_radio_label = set_label_text(m_back_box, _previous_service_name, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_previous_radio_label);
    lv_text_get_size(&pos, _previous_service_name.data(), font_25, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    DEBUG_MSG(OSD, DEBUG, "Previous radio label width: " << pos.x << "\n");
    lv_obj_align(m_previous_radio_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    if (pos.x > name_width)
    {
        lv_label_set_long_mode(m_previous_radio_label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_previous_radio_label, name_width);
        lv_obj_set_height(m_previous_radio_label, name_height);
    }
}

} // namespace mb
