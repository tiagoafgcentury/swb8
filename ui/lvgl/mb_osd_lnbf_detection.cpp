#include "mb_osd_lnbf_detection.h"
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

OSD_Lnbf_Detection *OSD_Lnbf_Detection::s_instance { nullptr };

OSD_Lnbf_Detection::OSD_Lnbf_Detection(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, btn_w, btn_h, btn_s, btn_x, btn_y)
{
    m_current_progress = 0;
    s_instance = this;
}

OSD_Lnbf_Detection::~OSD_Lnbf_Detection()
{
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_main);
    s_instance = nullptr;
    Osd_Breadcrumb::s_instance.remove_name();
}

void OSD_Lnbf_Detection::save_config_params()
{
    Event_LNBF_Params lnbf_params;
    auto config = Config::get_config();
    const auto& sat_config = config->selected_satellite_config();
    lnbf_params.band = config->band();
    lnbf_params.lnbf_type = config->lnbf_type();
    lnbf_params.inverted = false;
}

bool OSD_Lnbf_Detection::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_PLUS:
        {
            DEBUG_MSG(OSD, DEBUG, "KEY_PLUS\n");
            if (m_status == Status::Point)
            {
                set_lnbf_type();
            }

            break;
        }

        case Remote_Control_Key::KEY_VOLTAR:
            DELETE_TIMER(m_beep_timer);
            Sound::get_instance()->stop_tone();
            Sound::get_instance()->mute_state();
            Task::post_event(std::bind(m_callback, false, Transponder_Id{}));
            return true;

        case Remote_Control_Key::KEY_OK:
        {
            switch(m_status)
            {
                case Status::Detect:
                {
                    Task::post_event(std::bind(m_callback, false, Transponder_Id{}));
                    break;
                }

                case Status::Fail:
                {
                    auto pressed = m_keys.get_selected();

                    if (pressed == 0)
                    {
                        Task::post_event(std::bind(m_callback, false, Transponder_Id{}));
                    }
                    else if (pressed == 1)
                    {
                        to_detect();
                    }
                    else
                    {
                        Task::post_event_set_default_lnbf();
                        to_point();
                    }
                    break;
                }

                case Status::To_Fail:
                    break;

                case Status::Success:
                {
                    to_point();
                    break;
                }

                case Status::To_Success:
                    break;

                case Status::Point:
                {
                    lv_timer_pause(m_refresh_timer);
                    DELETE_TIMER(m_beep_timer);
                    Sound::get_instance()->stop_tone();
                    Sound::get_instance()->mute_state();
                    auto pressed = m_keys.get_selected();

                    if (pressed == 0)
                    {
                        Task::post_event(std::bind(m_callback, false, Transponder_Id{}));
                    }
                    else
                    {
                        lv_obj_add_flag(m_main, LV_OBJ_FLAG_HIDDEN);
                        save_config_params();
                        lv_obj_remove_flag(m_main, LV_OBJ_FLAG_HIDDEN);
                        Task::post_event(std::bind(m_callback, true, Transponder_Id{}));
                    }
                    break;
                }
            }

            return true;
        }

        case Remote_Control_Key::KEY_VOLUP:
        {
            m_keys.next();
            break;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            m_keys.previous();
            break;
        }

        case Remote_Control_Key::KEY_CHUP:
        case Remote_Control_Key::KEY_CHDOWN:
            return true;

        default:
            return true;
    }

    return true;
}

void OSD_Lnbf_Detection::show_menu_lnbf_detection(osd_lnbf_detection_callback_t _callback, Satellite _sat)
{
    set_focus();
    m_callback = _callback;
    m_satellite = _sat;
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);

    // Cria interrupção periódica de 1 segundo
     if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }

    // Criar barra de progresso
    create_progress_bar();
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Detectar_lnbf));
    // Inicia detecção do lnbf
    to_detect();
}

void OSD_Lnbf_Detection::update_info(Event_Transponder_data _progress)
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

void OSD_Lnbf_Detection::create_progress_bar()
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

void OSD_Lnbf_Detection::refresh_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    auto thiz = static_cast<OSD_Lnbf_Detection *>(lv_timer_get_user_data(_tm));
    thiz->refresh_progress();
}

void OSD_Lnbf_Detection::refresh_progress()
{
    switch (m_status)
    {
        case Status::Detect:
            detect();
            break;

        case Status::To_Fail:
            to_fail();
            break;

        case Status::Fail:
            fail();
            break;

        case Status::To_Success:
            to_success();
            break;

        case Status::Success:
            success();
            break;

        case Status::Point:
            point();
            break;
    }
}

void OSD_Lnbf_Detection::success()
{
}

void OSD_Lnbf_Detection::detect()
{
    m_current_progress += 2;

    if (m_current_progress <= 100)
    {
        lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
    }
}

void OSD_Lnbf_Detection::point()
{
    auto signal_info = Task_Tuner::get_signal_info();
    int quality = signal_info.quality / 10;
    lv_label_set_text_fmt(m_slider_label, "%s: %d%%", tr(__Qualidade).data(), quality);
    lv_slider_set_value(m_slider, quality, LV_ANIM_ON);
    double snr = signal_info.signal_noise_ratio / 100;
    lv_label_set_text_fmt(m_snr, "SNR: %0.2f dB", snr);
    m_last_snr = snr;
    if(!m_beep_timer)
    {
        m_beep_timer = lv_timer_create(beep_cb, 50, this);
    }
}

void OSD_Lnbf_Detection::fail()
{
    if (--m_current_progress)
    {
        update_subtitle();
    }
    else if (m_current_progress == 0)
    {
        to_detect();
    }
}

void OSD_Lnbf_Detection::populate(Status _st)
{
    m_status = _st;
    const auto &sc = m_screen_content[_st];
    // Título
    DELETE_OBJ(m_title);
    m_title = set_label_text(m_main, sc.title, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    // Subtítulo
    DELETE_OBJ(m_subtitle);
    DELETE_OBJ(m_bgd_subtitle);
    m_bgd_subtitle = create_rect(m_main, subtitle_x, subtitle_y, subtitle_w, subtitle_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_subtitle);
    m_subtitle = set_label_text(m_bgd_subtitle, sc.subtitle, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, 0);
    // Rodapé
    DELETE_OBJ(m_footer);
    m_footer = MB_OSD_Footer::draw(m_main, sc.footer, -40);
    lv_obj_null_on_delete(&m_footer);
    // Seta cor do slider
    lv_obj_set_style_bg_color(m_slider, sc.bar_color, LV_PART_INDICATOR);
    // Altera status e redesenha teclas
    lv_timer_set_period(m_refresh_timer, sc.period);
    draw_buttons();
}

void OSD_Lnbf_Detection::to_detect()
{
    m_current_progress = 0;
    populate(Status::Detect);
    // Inicia detecção do lnbf
    lnbf_detection();
}

void OSD_Lnbf_Detection::to_point()
{
    Osd_Breadcrumb::s_instance.replace_last_name(tr(__Aponte_sua_antena));
    m_current_progress = 0;
    populate(Status::Point);
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

void OSD_Lnbf_Detection::to_success()
{
    // Verificação de satélite encontrado
#ifndef NDEBUG
    auto [tp, _] = Task_Tuner::get_current_transponder();
    DEBUG_MSG(OSD, DEBUG, "Frequency: " << tp.frequency() << "\n");
#endif
    populate(Status::Success);
    m_current_progress = 0;
    lv_label_set_text(m_slider_label, "100%");
    lv_slider_set_value(m_slider, 100, LV_ANIM_ON);
}

void OSD_Lnbf_Detection::to_fail()
{
    populate(Status::Fail);
    m_current_progress = 30;
    update_subtitle();
    lv_slider_set_value(m_slider, 100, LV_ANIM_OFF);
    lv_label_set_text(m_slider_label, "100%");
}

void OSD_Lnbf_Detection::update_subtitle()
{
    lv_label_set_text_fmt(m_subtitle, tr(__Nova_tentativa_em_xxx_segundos).data(), m_current_progress);
}

void OSD_Lnbf_Detection::draw_buttons()
{
    switch (m_status)
    {
        case Status::Detect :
        {
            m_keys.clear();
            m_keys.set_background(m_main);
            m_keys.add_label(tr(__Voltar));
            m_keys.set_x(530);
            m_keys.set_y(570);
            m_keys.set_width(220);
            m_keys.draw();
            m_keys.select();
        }
        break;

        case Status::Fail :
        {
            m_keys.clear();
            m_keys.set_background(m_main);
            m_keys.add_label(tr(__Voltar));
            m_keys.add_label(tr(__Detectar_novamente));
            m_keys.add_label(tr(__Proximo));
            m_keys.set_x(227);
            m_keys.set_y(570);
            m_keys.set_width(260);
            m_keys.set_spacing(523 - 227);
            m_keys.draw();
            m_keys.select();
            m_keys.next();
        }
        break;

        case Status::To_Fail:
            break;

        case Status::Success :
        {
            m_keys.clear();
            m_keys.set_background(m_main);
            m_keys.add_label(tr(__Proximo));
            m_keys.set_x(530);
            m_keys.set_y(570);
            m_keys.set_width(220);
            m_keys.draw();
            m_keys.select();
        }
        break;

        case Status::To_Success:
            break;

        case Status::Point :
        {
            m_keys.clear();
            m_keys.set_background(m_main);
            m_keys.add_label(tr(__Voltar));
            m_keys.add_label(tr(__Proximo));
            m_keys.set_x(409);
            m_keys.set_y(570);
            m_keys.set_width(220);
            m_keys.set_spacing(652 - 409);
            m_keys.draw();
            m_keys.select();
            m_keys.next();
        }
        break;
    }
}

void OSD_Lnbf_Detection::lnbf_detection()
{
    if (!m_detect_lnbf)
    {
        m_detect_lnbf = std::make_unique<MB_Detect_Lnbf>(this);
    }

    m_detect_lnbf->lnbf_detection_start(std::bind(&OSD_Lnbf_Detection::lnbf_detection_callback, this, std::placeholders::_1, std::placeholders::_2), m_satellite);
}

void OSD_Lnbf_Detection::lnbf_detection_callback(bool _result, [[maybe_unused]] Transponder_Id tp)
{

    DEBUG_MSG(OSD, DEBUG, (_result ? "Success" : "Fail") << ", Frequency: " << tp.frequency() << "\n");

    if (_result)
    {
        m_status = Status::To_Success;
        DEBUG_MSG(OSD, DEBUG, "Status::to_success\n");
    }
    else
    {
        m_status = Status::To_Fail;
        DEBUG_MSG(OSD, DEBUG, "Status::to_fail\n");
    }

    m_detect_lnbf.reset();
}

void OSD_Lnbf_Detection::hide_menu()
{
    remove_focus();
    Task::post_event(std::bind(m_callback, false, Transponder_Id{}));
}

void OSD_Lnbf_Detection::set_lnbf_type()
{
    auto config = Config::get_config();
    auto current_type = config->lnbf_type();
    current_type = current_type == LNBF_Type::Universal ? LNBF_Type::Multi : LNBF_Type::Universal;
    Task::post_event_set_lnbf_type(current_type);
}

void OSD_Lnbf_Detection::beep_cb(lv_timer_t* _tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    auto thiz = static_cast<OSD_Lnbf_Detection *>(lv_timer_get_user_data(_tm));
    thiz->process_beep();
}

void OSD_Lnbf_Detection::process_beep()
{
    double snr = m_last_snr;

    if(snr <= 7.0)
    {
        Sound::get_instance()->stop_tone();
        return;
    }

    if(snr > 12.0)
    {
        if(!m_beep_on)
        {
            Sound::get_instance()->set_tone(snr);
            m_beep_on = true;
        }
        return;
    }

    uint32_t on_time  = 0;
    uint32_t off_time = 0;

    if(snr > 11.0)
    {
        on_time  = 80;   // rápido
        off_time = 80;
    }
    else
    {
        on_time  = 200;  // médio
        off_time = 400;
    }

    m_beep_elapsed += 50; // timer de 50ms

    if(m_beep_on)
    {
        if(m_beep_elapsed >= on_time)
        {
            Sound::get_instance()->stop_tone();
            m_beep_on = false;
            m_beep_elapsed = 0;
        }
    }
    else
    {
        if(m_beep_elapsed >= off_time)
        {
            Sound::get_instance()->set_tone(snr);
            m_beep_on = true;
            m_beep_elapsed = 0;
        }
    }
}




void OSD_Lnbf_Detection::got_focus()
{
}

} // namespace mb
