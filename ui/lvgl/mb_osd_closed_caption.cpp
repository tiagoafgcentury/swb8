#include <iostream>
#include <map>
#include <sstream>

#include "mb_osd_closed_caption.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "tasks/mb_task_player.h"

namespace mb {

OSD_Closed_Caption::OSD_Closed_Caption(OSD *_parent):
    OSD(_parent)
{
}

OSD_Closed_Caption::~OSD_Closed_Caption()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Closed_Caption::~OSD_Closed_Caption" << "\n");
    remove_focus();
    DELETE_OBJ(m_main_box);
    DELETE_TIMER(m_exit_timer);
}

// Processa tecla recebida
bool OSD_Closed_Caption::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        case Remote_Control_Key::KEY_CC:
        {
            next();
            update_captions();
            lv_timer_reset(m_exit_timer);
            break;
        }

        default:
            break;
    }

    return true;
}

void OSD_Closed_Caption::next()
{
    m_value = static_cast<int>(m_selected);
    m_value += 1;

    const bool has_subtitle = m_current_srv && m_current_srv->has_dvb_subtitle();

    if (has_subtitle)
    {
        if (m_value >= static_cast<uint8_t>(CC_Type::COUNT))
            m_value = 0;
    }
    else
    {
        if (m_value >= static_cast<uint8_t>(CC_Type::Subtitle))
            m_value = 0;
    }
    m_selected = static_cast<CC_Type>(m_value);
}

void OSD_Closed_Caption::show_closed_caption(osd_closed_caption_cb _callback)
{
    DEBUG_MSG(OSD, DEBUG, "show_closed_caption" << "\n");
    m_callback = _callback;
    set_focus();
    m_current_srv = Task::s_task_player->current_srv();
    // Desenha caixa principal
    draw_main_box();
    // Create timer to exit after 15 seconds
    m_exit_timer = lv_timer_create(process_exit_cb, 5000, this);
}

void OSD_Closed_Caption::process_exit_cb(lv_timer_t *tm)
{

    OSD_Closed_Caption *thiz = static_cast<OSD_Closed_Caption *>(lv_timer_get_user_data(tm));
    Task::post_event_cc_enable(thiz->m_selected);
    Task::post_event(std::bind(thiz->m_callback));
}

void OSD_Closed_Caption::update_captions()
{
    lv_label_set_text(m_lbl, m_caption_names[static_cast<size_t>(m_selected)].data());
    //Task::post_event_player_change_audio(audio.pid);
}

void OSD_Closed_Caption::draw_main_box()
{
    m_main_box = create_rect(get_main_screen(OSD_Layer::CLOSED_CAPTION_LAYER), main_box_x, main_box_y, main_box_w, main_box_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_box);
    lv_obj_set_style_bg_opa(m_main_box, OSD_TRANPARENCY_LIVE, 0);
    lv_obj_set_style_radius(m_main_box, 25, DEFAULT_SELECTOR);
#ifdef MBGUI_USE_RLOTTIE
    auto usb_img = lv_rlottie_create_from_file(m_main_box, ANI_MARGIN, ANI_MARGIN, ANIM_CLOSED_CAPTIONS);
    lv_obj_align(usb_img, LV_ALIGN_LEFT_MID, 15, 0);
    lv_rlottie_set_play_mode(usb_img, LV_RLOTTIE_CTRL_LOOP);
#endif
    auto cc_lbl = set_label_text(m_main_box, "Closed Caption ", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(cc_lbl, LV_ALIGN_LEFT_MID, 20 + ANI_MARGIN, 0);
    m_lbl = set_label_text(m_main_box, tr(__Desativado), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(m_lbl, LV_ALIGN_LEFT_MID, 200 + ANI_MARGIN, 0);
}

} // namespace mb
