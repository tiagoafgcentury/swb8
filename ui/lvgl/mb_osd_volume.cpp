#include "mb_osd_volume.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"
#include "common/mb_globals.h"
#include "tasks/mb_task.h"

namespace mb {

OSD_Volume::OSD_Volume():
    OSD(nullptr)
{
    lv_style_init(&m_style_volume);
    lv_style_init(&m_style_prog_bar);
#ifdef MBGUI_ANIMATION
    lv_anim_init(&m_anim);
#endif
}

OSD_Volume::~OSD_Volume()
{
    DELETE_TIMER(m_tmr_volume_on);
    DELETE_OBJ(m_bgd_main);
    lv_style_reset(&m_style_volume);
    lv_style_reset(&m_style_prog_bar);
}

void OSD_Volume::create_volume(uint16_t _volume)
{
    static char volume [6];
    snprintf(volume, sizeof(volume), "%d", _volume);
    m_bgd_main = create_rect(get_main_screen(OSD_Layer::VOLUME_LAYER), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_main);
    lv_obj_set_style_bg_opa(m_bgd_main, LV_OPA_TRANSP, 0);
    lv_obj_move_background(m_bgd_main);
    m_bgd_volume = create_rect(m_bgd_main, vol_start_x, vol_start_y, vol_start_w, vol_start_h, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_bgd_volume, OSD_TRANPARENCY_LIVE, 0);
    lv_obj_set_style_radius(m_bgd_volume, 25, DEFAULT_SELECTOR);
    m_lbl_volume = set_label_text_static(m_bgd_volume, volume, 0, 0, font_bold_25, OSD_COLOR_WHITE);
    lv_obj_align(m_lbl_volume, LV_ALIGN_TOP_MID, 0, 10);
    m_logo_volume = load_image(m_bgd_volume, LOGO_VOLUME_ON_25x25, 0, 0, 25, 25);
    lv_obj_align(m_logo_volume, LV_ALIGN_BOTTOM_MID, 0, -10);
    m_logo_volume_off = load_image(m_bgd_volume, LOGO_VOLUME_OFF_25x25, 0, 0, 25, 25);
    lv_obj_align(m_logo_volume_off, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_flag(m_logo_volume_off, LV_OBJ_FLAG_HIDDEN);
    lv_style_set_bg_opa(&m_style_volume, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style_volume, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_style_prog_bar, OSD_COLOR_ORANGE);
    m_prgbar_volume = lv_bar_create(m_bgd_volume);
    lv_obj_set_width(m_prgbar_volume, 15);
    lv_obj_set_height(m_prgbar_volume, 312);
    lv_obj_align(m_prgbar_volume, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_value(m_prgbar_volume, _volume, LV_ANIM_ON);
    lv_style_set_anim_duration(&m_style_volume, ANIM_BAR_DURATION);
    lv_obj_add_style(m_prgbar_volume, &m_style_prog_bar, LV_PART_INDICATOR);
    lv_obj_add_style(m_prgbar_volume, &m_style_volume, LV_PART_MAIN);
    m_bgd_volume_off = create_rect(m_bgd_main, mute_start_x, mute_start_y, mute_start_w, mute_start_h, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_bgd_volume_off, OSD_TRANPARENCY_LIVE, 0);
    lv_obj_set_style_radius(m_bgd_volume_off, 25, DEFAULT_SELECTOR);
    m_logo_volume_off_sel = load_image(m_bgd_volume_off, LOGO_VOLUME_OFF_SEL_25x25, 0, 0, 50, 50);
    lv_obj_align(m_logo_volume_off_sel, LV_ALIGN_LEFT_MID, 0, 0);
    m_lbl_volume_off_sel = set_label_text(m_bgd_volume_off, tr(__Sem_audio), 0, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_align(m_lbl_volume_off_sel, LV_ALIGN_CENTER, -40, 0);
    lv_obj_add_flag(m_bgd_volume_off, LV_OBJ_FLAG_HIDDEN);
    m_bgd_volume_off_grey = create_rect(m_bgd_main, mute_start_x + 238, mute_start_y, 50, 50, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_bgd_volume_off_grey, LV_OPA_TRANSP, 0);
    m_img_volume_off = load_image(m_bgd_volume_off_grey, LOGO_VOLUME_OFF_CIRC_25x25, 0, 0, 50, 50);
    lv_obj_add_flag(m_bgd_volume_off_grey, LV_OBJ_FLAG_HIDDEN);
#ifdef MBGUI_ANIMATION
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&m_anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
    /*Set target of the animation*/
    lv_anim_set_var(&m_anim, m_bgd_volume);
    /*Length of the animation [ms]*/
    lv_anim_set_duration(&m_anim, 100);
    /*Set path (curve). Default is linear*/
    lv_anim_set_path_cb(&m_anim, lv_anim_path_ease_in);
    lv_anim_set_user_data(&m_anim, this);
    lv_obj_set_x(m_bgd_volume, START_POS_X);
    show_volume_bar();
#endif
}

void OSD_Volume::update_volume_cb(lv_timer_t *_tm)
{
    auto thiz = (OSD_Volume *)lv_timer_get_user_data(_tm);
#ifdef MBGUI_ANIMATION
    thiz->hide_volume_bar();
#else
    thiz->delete_volume();
#endif
}

void OSD_Volume::set_volume(uint16_t _volume, bool _mute, Volume_CB_t _callback)
{
    m_callback = _callback;

    if (!m_bgd_main)
    {
        create_volume(_volume);
    }
    else
    {
        lv_label_set_text_fmt(m_lbl_volume, "%d", _volume);
        lv_slider_set_value(m_prgbar_volume, _volume, LV_ANIM_ON);
        lv_obj_add_flag(m_bgd_volume_off_grey, LV_OBJ_FLAG_HIDDEN);
#ifdef MBGUI_ANIMATION

        if (lv_obj_get_x(m_bgd_volume) != END_POS_X)
        {
            show_volume_bar();
        }

#endif
    }

    if (!m_tmr_volume_on)
    {
        m_tmr_volume_on = lv_timer_create(update_volume_cb, 5000, this);
        lv_timer_set_auto_delete(m_tmr_volume_on, false);
        lv_timer_pause(m_tmr_volume_on);
    }

    lv_obj_remove_flag(m_bgd_volume, LV_OBJ_FLAG_HIDDEN);

    if ((_volume == 0) or (_mute))
    {
        lv_obj_remove_flag(m_logo_volume_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(m_bgd_volume_off, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(m_lbl_volume_off_sel, tr(__Sem_audio).data());
        lv_obj_add_flag(m_logo_volume, LV_OBJ_FLAG_HIDDEN);
        m_muted = true;
    }
    else
    {
        lv_obj_add_flag(m_logo_volume_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_bgd_volume_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(m_logo_volume, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_bgd_volume_off_grey, LV_OBJ_FLAG_HIDDEN);
        m_muted = false;
    }

    lv_timer_reset(m_tmr_volume_on);
    lv_timer_resume(m_tmr_volume_on);
}

#ifdef MBGUI_ANIMATION
void OSD_Volume::show_volume_bar()
{
    lv_obj_remove_flag(m_bgd_volume, LV_OBJ_FLAG_HIDDEN);
    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&m_anim, lv_obj_get_x(m_bgd_volume), END_POS_X);
    /*Set a callback to indicate when the animation is completed.*/
    lv_anim_set_completed_cb(&m_anim, nullptr);
    lv_anim_start(&m_anim);
}

void OSD_Volume::hide_volume_bar()
{
    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&m_anim, lv_obj_get_x(m_bgd_volume), START_POS_X);
    /*Set a callback to indicate when the animation is completed.*/
    lv_anim_set_completed_cb(&m_anim, anim_delete_callback);
    lv_anim_start(&m_anim);
}

void OSD_Volume::anim_delete_callback(lv_anim_t *_anim)
{
    auto thiz = (OSD_Volume *)lv_anim_get_user_data(_anim);

    // Check we are really in the phase-out animation.
    // Ignore if we were called in a phase-in.
    if (lv_obj_get_x(thiz->m_bgd_volume) == START_POS_X)
    {
        thiz->delete_volume();
    }
}

#endif

void OSD_Volume::delete_volume()
{
    lv_timer_pause(m_tmr_volume_on);
    lv_obj_add_flag(m_bgd_volume, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_bgd_volume_off, LV_OBJ_FLAG_HIDDEN);

    if (m_muted)
    {
        lv_obj_remove_flag(m_bgd_volume_off_grey, LV_OBJ_FLAG_HIDDEN);
    }

    Task::post_event(std::bind(m_callback, m_muted));
}

} //mb
