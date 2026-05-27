#include "mb_osd_lnbf_snr.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"
#include <aui_nim.h>
#include "hal/mb_tuner.h"

#include <lvgl.h>
#include <stdio.h>

namespace mb {

OSD_Lnbf_Snr *OSD_Lnbf_Snr::s_instance { nullptr };

OSD_Lnbf_Snr::OSD_Lnbf_Snr(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, btn_w, btn_h, btn_s, btn_x, btn_y)
{
    m_current_progress = 0;
    s_instance = this;
}

OSD_Lnbf_Snr::~OSD_Lnbf_Snr()
{
    DELETE_TIMER(m_beep_timer);
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_main);
    s_instance = nullptr;
    Osd_Breadcrumb::s_instance.remove_name();
}

bool OSD_Lnbf_Snr::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_PLUS:
        {
            DEBUG_MSG(OSD, DEBUG, "KEY_PLUS\n");
            next_transponder();
            break;
        }

        case Remote_Control_Key::KEY_OK:
        case Remote_Control_Key::KEY_VOLTAR:
        {
            DELETE_TIMER(m_beep_timer);
            lv_timer_pause(m_refresh_timer);
            Sound::get_instance()->stop_tone();
            Sound::get_instance()->mute_state();
            Task::post_event(std::bind(m_callback, m_signal_found));
            break;
        }

        default:
            break;
    }
    return true;
}

void OSD_Lnbf_Snr::show_lnbf_snr(osd_lnbf_snr_callback_t _callback)
{ 
    set_focus();
    m_callback = _callback;
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);

    // Cria interrupção periódica de 1 segundo
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }

    if (!m_beep_timer)
    {
        m_beep_timer = lv_timer_create(beep_cb, 50, this);
        lv_timer_set_repeat_count(m_beep_timer, -1);
    }

    // Criar barra de progresso
    create_progress_bar();
    // Inicia detecção do lnbf
    to_point();
    // Inicia primeiro transponder
    next_transponder();
}

void OSD_Lnbf_Snr::update_info(Event_Transponder_data _progress)
{
    auto frequency = _progress.frequency;
    auto sr = _progress.symbol_rate;
    auto pol = _progress.polarity;
    auto sat = _progress.satellite;
    auto band = _progress.band == Band::Ku ? tr(__Banda_KU) : tr(__Banda_C);
    auto lnbf = _progress.lnbf_type == LNBF_Type::Universal ? tr(__Universal) : tr(__Multiponto);

    // Barra de informações
    if (m_info_box == nullptr)
    {
        m_info_box = create_rect(m_main, 0, info_y, width, info_h, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_info_box);
    }

    DELETE_OBJ(m_info);
    char text[100];
    snprintf(text, sizeof(text), "%s: %s - %s - %s - %s: %dMHz / %dKbps / %c",
             tr(__Satelite).data(), sat.data(), band.data(), lnbf.data(), tr(__Canal_de_referencia).data(), frequency, sr, pol);
    m_info = set_label_text(m_info_box, text, 0, 0, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_info);
    lv_obj_center(m_info);
}

void OSD_Lnbf_Snr::create_progress_bar()
{
    /*Create a slider in the center of the display*/
    m_slider = lv_slider_create(m_main);
    lv_obj_null_on_delete(&m_slider);
    lv_obj_set_pos(m_slider, slider_x, slider_y);
    lv_obj_set_size(m_slider, slider_w, slider_h);
    lv_obj_set_style_anim_duration(m_slider, 2000, 0);
    // Set the indicator color to green
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_BLUE, LV_PART_INDICATOR);
    // Set the background color to light gray
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREY, LV_PART_MAIN);
    // Make the knob completely transparent
    lv_obj_set_style_bg_opa(m_slider, LV_OPA_TRANSP, LV_PART_KNOB);
    /*Create a label below the slider*/
    m_slider_label = set_label_text(m_main, "", slider_label_x, slider_label_y, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_slider_label);
    lv_label_set_text(m_slider_label, "");
    lv_obj_align(m_slider_label, LV_ALIGN_TOP_MID, 0, slider_label_y);
}

void OSD_Lnbf_Snr::refresh_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }
    auto thiz = static_cast<OSD_Lnbf_Snr *>(lv_timer_get_user_data(_tm));
    thiz->refresh_progress();
}

void OSD_Lnbf_Snr::beep_cb(lv_timer_t* tm)
{
    if (!tm || !lv_timer_get_user_data(tm))
        return;

    auto thiz = static_cast<OSD_Lnbf_Snr*>(lv_timer_get_user_data(tm));
    thiz->process_beep();
}

void OSD_Lnbf_Snr::refresh_progress()
{
    auto signal_info = Task_Tuner::get_signal_info();
    int quality = signal_info.quality / 10;
    lv_label_set_text_fmt(m_slider_label, "%s: %d%%", tr(__Qualidade).data(), quality);
    lv_slider_set_value(m_slider, quality, LV_ANIM_ON);
    double snr = signal_info.signal_noise_ratio / 100;
    lv_label_set_text_fmt(m_snr, "SNR: %0.2f dB", snr);
    m_last_snr = snr;
    m_signal_found = quality >= 20;
}

void OSD_Lnbf_Snr::populate()
{
    // Título
    m_title = set_label_text(m_main, tr(__Aponte_sua_antena), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    // Subtítulo
    m_bgd_subtitle = create_rect(m_main, subtitle_x, subtitle_y, subtitle_w, subtitle_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_subtitle);
    m_subtitle = set_label_text(m_bgd_subtitle, tr(__Ajuste_antena_utilizando_medidor), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, 0);
    // Rodapé
    m_footer = MB_OSD_Footer::draw(m_main, tr(__Pressione_a_tecla_mais_para_alterar_o_tipo_de_lnbf), -40);
    lv_obj_null_on_delete(&m_footer);
    // Seta cor do slider
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREEN, LV_PART_INDICATOR);
    // Altera status e redesenha teclas
    lv_timer_set_period(m_refresh_timer, 1000);
    draw_buttons();
}

void OSD_Lnbf_Snr::to_point()
{
    Osd_Breadcrumb::s_instance.add_name(tr(__Aponte_sua_antena));
    m_current_progress = 0;
    populate();
    lv_label_set_long_mode(m_subtitle, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(m_subtitle, subtitle_w);
    lv_obj_set_style_text_align(m_subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(m_subtitle, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(m_slider_label, tr(__Qualidade).data());
    lv_slider_set_value(m_slider, 0, LV_ANIM_OFF);
    m_snr = set_label_text(m_main, "", snr_x, snr_y, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr);
    lv_label_set_text(m_snr, "SNR: 0 dB");
    Sound::get_instance()->set_volume(50);
}

void OSD_Lnbf_Snr::update_subtitle()
{
    lv_label_set_text_fmt(m_subtitle, tr(__Nova_tentativa_em_xxx_segundos).data(), m_current_progress);
}

void OSD_Lnbf_Snr::draw_buttons()
{
    m_keys.clear();
    m_keys.set_background(m_main);
    m_keys.add_label(tr(__Voltar));
    m_keys.set_x(btn_x);
    m_keys.set_y(btn_y);
    m_keys.set_width(btn_w);
    m_keys.draw();
    m_keys.select();
    m_keys.next();
}

void OSD_Lnbf_Snr::next_transponder()
{
    Task::post_event_change_lnbf_type(next_transponder_callback);
}

void OSD_Lnbf_Snr::next_transponder_callback(const std::string &result)
{
    if (s_instance == nullptr)
    {
        return;
    }
    s_instance->refresh_info_box(result);
}

void OSD_Lnbf_Snr::refresh_info_box(const std::string &result)
{
    if (m_info_box == nullptr)
    {
        m_info_box = create_rect(m_main, 0, info_y, width, info_h, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_info_box);
    }

    DELETE_OBJ(m_info);
    m_info = set_label_text(m_info_box, result.data(), 0, 0, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_info);
    lv_obj_center(m_info);
}

void OSD_Lnbf_Snr::got_focus()
{
}

void OSD_Lnbf_Snr::hide_menu()
{
    remove_focus();
    DELETE_TIMER(m_beep_timer);
    Sound::get_instance()->stop_tone();

    Task::post_event(std::bind(m_callback, false));
}

void OSD_Lnbf_Snr::process_beep()
{
    double snr = m_last_snr;

    int range = 0;

    if (snr > 12.0)          range = 3;  // contínuo
    else if (snr > 11.0)     range = 2;  // rápido
    else if (snr > 1.0)      range = 1;  // médio
    else                     range = 0;  // silêncio

    // Mudou de faixa?
    if (range != m_last_range)
    {
        m_last_range = range;
        m_beep_counter = 0;
        m_beep_on = false;
        Sound::get_instance()->stop_tone();
    }

    if (range == 0)
    {
        Sound::get_instance()->stop_tone();
        return;
    }

    // Contínuo
    if (range == 3)
    {
        if (!m_beep_on)
        {
            Sound::get_instance()->set_tone(50);
            m_beep_on = true;
        }
        return;
    }

    m_beep_counter += 50;

    uint32_t on_time;
    uint32_t off_time;

    if (range == 2)
    {
        on_time  = 70;   // mais rápido que antes
        off_time = 70;
    }
    else
    {
        on_time  = 200;
        off_time = 400;
    }

    if (m_beep_on && m_beep_counter >= on_time)
    {
        Sound::get_instance()->stop_tone();
        m_beep_on = false;
        m_beep_counter = 0;
    }
    else if (!m_beep_on && m_beep_counter >= off_time)
    {
        Sound::get_instance()->set_tone(50);
        m_beep_on = true;
        m_beep_counter = 0;
    }
}

} // namespace mb
