#include <iostream>
#include <map>
#include <sstream>

#include "mb_osd_audio_lr.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "hal/mb_system.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "tasks/mb_task_player.h"

namespace mb {

OSD_Audio_LR::OSD_Audio_LR(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
}

OSD_Audio_LR::~OSD_Audio_LR()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Audio_LR::~OSD_Audio_LR\n");
    remove_focus();
    DELETE_OBJ(m_left_icon);
    DELETE_OBJ(m_right_icon);
    DELETE_OBJ(m_lbl);
    DELETE_OBJ(m_main_box);
    DELETE_OBJ(m_audio_box);

    for (auto &lang_label : m_lang_label)
    {
        DELETE_OBJ(lang_label);
    }

    for (auto &lang_box : m_lang_box)
    {
        DELETE_OBJ(lang_box);
    }

    m_lang_label.clear();
    m_lang_box.clear();
    m_lang_str.clear();
    DELETE_TIMER(m_exit_timer);
    DELETE_OBJ(m_main);
    m_keys.clear();
}

// Processa tecla recebida
bool OSD_Audio_LR::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
            update_audio_lang();
            Task::post_event(m_callback);
            break;

        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        case Remote_Control_Key::KEY_CHUP:
        case Remote_Control_Key::KEY_CHDOWN:
        case Remote_Control_Key::KEY_LR:
        {
            m_keys.next();
            update_audio_lang();
            lv_timer_reset(m_exit_timer);
            break;
        }

        default:
            break;
    }

    return true;
}

void OSD_Audio_LR::show_audio_lr(Audio_LR_CB_t _callback)
{
    DEBUG_MSG(OSD, DEBUG, "show_audio_lr\n");
    m_callback = _callback;
    set_focus();
    // Desenha caixa principal
    draw_main_box();
    // Desenha caixa de audio
    draw_audio_box();
    // Desenha idiomas de audio disponíveis
    draw_audio_langs();
    // Create timer to exit after 15 seconds
    m_exit_timer = lv_timer_create(process_exit_cb, 5000, this);
}

void OSD_Audio_LR::process_exit_cb(lv_timer_t *_tm)
{
    OSD_Audio_LR *thiz = static_cast<OSD_Audio_LR *>(lv_timer_get_user_data(_tm));
    Task::post_event(std::bind(thiz->m_callback));
}

void OSD_Audio_LR::update_audio_lang()
{
    auto current_service = Task::s_task_player->current_srv();

    if (current_service and not current_service->audio_pids().empty())
    {
        size_t index = m_keys.get_selected();
        DEBUG_MSG(OSD, DEBUG, index << " / " << current_service->audio_pids().size() << "\n");

        if (index < current_service->audio_pids().size())
        {
            const auto &audio = current_service->audio_pids()[index];
            DEBUG_MSG(OSD, DEBUG, "Audio PID: " << audio.pid << ", Lang: " << audio.lang << "\n");
            Task::post_event_player_change_audio(audio.pid);
            draw_lr_icons(index);
        }
    }
}

void OSD_Audio_LR::draw_lr_icons(size_t index)
{
    DEBUG_MSG(OSD, DEBUG, "draw_lr_icons\n");
    DELETE_OBJ(m_left_icon);
    DELETE_OBJ(m_right_icon);
    auto y = index * spacing + 25;
    m_left_icon = load_image(m_audio_box, LOGO_AUDIO_L_27x27, button_w - 30, y, 27, 27);
    m_right_icon = load_image(m_audio_box, LOGO_AUDIO_R_27x27, button_w - 60, y, 27, 27);
}

void OSD_Audio_LR::draw_audio_langs()
{
    // Clear previous languages
    m_lang_str.clear();
    m_lang_box.clear();
    m_lang_label.clear();
    // Load available audios
    auto current_service = Task::s_task_player->current_srv();

    if (current_service and not current_service->audio_pids().empty())
    {
        for (size_t it = 0 ; it < current_service->audio_pids().size(); ++it)
        {
            const auto &audio = current_service->audio_pids()[it];
            DEBUG_MSG(OSD, DEBUG, "audio.lang: " << audio.lang << "\n");
            std::stringstream ss;
            std::string lang_upper(audio.lang, std::min<size_t>(strlen(audio.lang), Service::AudioPid::LANG_SIZE));
            std::transform(lang_upper.begin(), lang_upper.end(), lang_upper.begin(), ::toupper);
            ss << tr(__Audio) << " " << (it + 1) << " ( " << lang_upper << " )";
            DEBUG_MSG(OSD, DEBUG, "draw_audio_langs: " << ss.str() << "\n");
            m_lang_str.push_back(ss.str());
        }
    }
    else
    {
        m_lang_str.push_back(std::string(tr(__Nenhum_audio)));
    }

    m_keys.set_background(m_audio_box);
    m_keys.set_vertical();
    m_keys.set_selected_color(OSD_COLOR_ORANGE);

    for (size_t i = 0; i < m_lang_str.size(); ++i)
    {
        m_keys.add_label(m_lang_str[i].data());
    }

    size_t current = 0;

    if (current_service)
    {
        current = current_service->current_audio_index();
    }

    if (current >= m_lang_str.size())
    {
        current = 0;
    }

        printf("\nOSD audio sync: player=%zu, ui=%zu\n",
        current_service ? current_service->current_audio_index() : 999,
        m_lang_str.size()
    );

    m_keys.draw();
    m_keys.select(current);
    draw_lr_icons(current);
}

void OSD_Audio_LR::draw_main_box()
{
    m_main_box = create_rect(get_main_screen(OSD_Layer::AUDIO_LR_LAYER), main_box_x, main_box_y, main_box_w, main_box_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_bg_opa(m_main_box, OSD_TRANPARENCY_LIVE, 0);
    lv_obj_set_style_radius(m_main_box, 25, DEFAULT_SELECTOR);
    m_lbl = set_label_text(m_main_box, tr(__Audio), 0, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_align(m_lbl, LV_ALIGN_TOP_LEFT, 20, 20);
}

void OSD_Audio_LR::draw_audio_box()
{
    m_audio_box = create_rect(m_main_box, audio_box_x, audio_box_y, audio_box_w, audio_box_h, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_audio_box, OSD_TRANPARENCY_LIVE, 0);
    lv_obj_set_style_radius(m_audio_box, 25, DEFAULT_SELECTOR);
    lv_obj_align(m_audio_box, LV_ALIGN_BOTTOM_MID, 0, -10);
}

} // namespace mb
