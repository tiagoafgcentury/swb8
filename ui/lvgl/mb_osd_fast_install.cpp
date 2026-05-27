#include "mb_osd_fast_install.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_menu_data.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"

#include <lvgl.h>

namespace mb {

OSD_Fast_Install::OSD_Fast_Install(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_quality_main);
    lv_style_init(&m_quality_indicator);
    lv_style_init(&m_quality_knob);
    lv_style_init(&m_strength_main);
    lv_style_init(&m_strength_indicator);
    lv_style_init(&m_strength_knob);
    lv_style_set_bg_opa(&m_quality_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_main, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&m_quality_main, LV_RADIUS_CIRCLE);
    lv_style_set_anim_duration(&m_quality_main, ANIM_BAR_DURATION);
    lv_style_set_bg_opa(&m_quality_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_indicator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_quality_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&m_quality_indicator, &m_transition_dsc);
    lv_style_set_bg_opa(&m_quality_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_knob, lv_color_hex3(0xffffff));
    lv_style_set_border_color(&m_quality_knob, lv_color_hex3(0xffffff));
    lv_style_set_border_width(&m_quality_knob, 2);
    lv_style_set_radius(&m_quality_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&m_quality_knob, 4);
    lv_style_set_transition(&m_quality_knob, &m_transition_dsc);
}

OSD_Fast_Install::~OSD_Fast_Install()
{
    lv_style_reset(&m_quality_main);
    lv_style_reset(&m_quality_indicator);
    lv_style_reset(&m_quality_knob);
    lv_style_reset(&m_strength_main);
    lv_style_reset(&m_strength_indicator);
    lv_style_reset(&m_strength_knob);
}

void OSD_Fast_Install::create_quality_slider()
{
    DEBUG_MSG(OSD, DEBUG, "create_quality_slider\n");
    lv_style_set_bg_opa(&m_quality_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_main, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&m_quality_main, LV_RADIUS_CIRCLE);
    lv_style_set_anim_duration(&m_quality_main, ANIM_BAR_DURATION);
    lv_style_set_bg_opa(&m_quality_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_indicator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_quality_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&m_quality_indicator, &m_transition_dsc);
    lv_style_set_bg_opa(&m_quality_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_knob, lv_color_hex3(0xffffff));
    lv_style_set_border_color(&m_quality_knob, lv_color_hex3(0xffffff));
    lv_style_set_border_width(&m_quality_knob, 2);
    lv_style_set_radius(&m_quality_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&m_quality_knob, 4);
    lv_style_set_transition(&m_quality_knob, &m_transition_dsc);
    // Texto com valores da qualidade
    m_quality_line = set_label_text(m_bgd, "", 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(m_quality_line, LV_ALIGN_BOTTOM_MID, 0, -80);
}

void OSD_Fast_Install::create_strength_slider()
{
    DEBUG_MSG(OSD, DEBUG, "create_strength_slider\n");
    lv_style_set_bg_opa(&m_strength_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_strength_main, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&m_strength_main, LV_RADIUS_CIRCLE);
    lv_style_set_anim_duration(&m_strength_main, ANIM_BAR_DURATION);
    lv_style_set_bg_opa(&m_strength_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&m_strength_indicator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_strength_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&m_strength_indicator, &m_transition_dsc);
    lv_style_set_bg_opa(&m_strength_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&m_strength_knob, lv_color_hex3(0xffffff));
    lv_style_set_border_color(&m_strength_knob, lv_color_hex3(0xffffff));
    lv_style_set_border_width(&m_strength_knob, 2);
    lv_style_set_radius(&m_strength_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&m_strength_knob, 4);
    lv_style_set_transition(&m_strength_knob, &m_transition_dsc);
    // Texto com valores de sinal
    m_strength_line = set_label_text(m_bgd, "", 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(m_strength_line, LV_ALIGN_BOTTOM_MID, 0, -30);
}

static void update_antenna_signal_cb(lv_timer_t *timer)
{
    SignalInfo signalinfo;
    Signal_Data *psignalData = (Signal_Data *)lv_timer_get_user_data(timer);
    signalinfo = Task_Tuner::get_signal_info();
    double snr = signalinfo.signal_noise_ratio / 100.0;
    //double ber = signalinfo.bit_error_rate / 1000.0;
    lv_label_set_text_fmt(psignalData->lbl_snr, "SNR: %.02f dB", snr);
    lv_label_set_text_fmt(psignalData->lbl_quality, "%s %d%%", tr(__Qualidade).data(), signalinfo.quality / 10);
    lv_bar_set_value(psignalData->prg_bar_quality, (signalinfo.quality / 10), LV_ANIM_ON);
    lv_label_set_text_fmt(psignalData->lbl_strength, "%s %d%%", tr(__Sinal).data(), (int)signalinfo.strength);
    lv_bar_set_value(psignalData->prg_bar_strength, (int)(signalinfo.strength), LV_ANIM_ON);
}

void OSD_Fast_Install::show_menu_fast_install(lv_obj_t *m_main_menu, OSD_Fast_Install_CB_t _callback)
{
    char buf1[1024] = { 0 };
    set_focus();
    m_callback = _callback;

    if (m_main_menu)
    {
        m_bgd_main = create_rect(m_main_menu, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(m_bgd_main, LV_OPA_TRANSP, 0);
        m_bgd = create_rect(m_bgd_main, 60, 90, DISPLAY_WIDTH - 120, DISPLAY_HEIGHT - 180, OSD_COLOR_GREY_MEDIUM);
        m_lbl_title = set_label_text_static(m_bgd, tr(__Instala_Rapido), 0, 0, font_semi_40, OSD_COLOR_WHITE);
        lv_obj_align(m_lbl_title, LV_ALIGN_CENTER, 0, -230);
        m_lbl_message = set_label_text_static(m_bgd, tr(__Satelite_detectado_com_sucesso), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_lbl_message, LV_ALIGN_CENTER, 0, -160);
        auto config = Config::get_config();
        const auto &sat_config = config->selected_satellite_config();
        const auto bLnb = to_str(config->lnbf_type());
        // Informações do transponder sintonizado
        auto [transponder_id, _] = Task_Tuner::get_current_transponder();
        auto frequency = transponder_id.frequency() / 1000;
        auto polarity = transponder_id.polarity();
        auto pol = polarity == Polarity::Horizontal ? "H" : "V";
        auto tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(transponder_id);
        auto sr = tp ? tp->symbol_rate : 0;
        snprintf(buf1, sizeof(buf1) - 1, "%s %s | %dMHz/%dKbps | %s | Lnb: %s", sat_config.name.data(), config->band() == Band::Ku ? tr(__Banda_KU).data() : tr(__Banda_C).data(), frequency, sr, pol, bLnb.data());
        m_satellite_line = set_label_text(m_bgd, buf1, 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_satellite_line, LV_ALIGN_CENTER, 0, -125);
        //m_service_line = set_label_text(m_bgd, buf2, 0, 0, font_25sb, OSD_COLOR_WHITE);
        //lv_obj_align(m_service_line, LV_ALIGN_CENTER, 0, -95);
        m_signal_data.lbl_snr = set_label_text_static(m_bgd, "SNR: ", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_signal_data.lbl_snr, LV_ALIGN_CENTER, 350, 70);
        create_quality_slider();
        create_strength_slider();
        m_signal_data.prg_bar_quality = lv_slider_create(m_bgd);
        lv_obj_remove_style_all(m_signal_data.prg_bar_quality);
        lv_obj_set_width(m_signal_data.prg_bar_quality, width_event / 2);
        lv_obj_set_height(m_signal_data.prg_bar_quality, 4);
        lv_obj_add_style(m_signal_data.prg_bar_quality, &m_quality_knob, LV_PART_KNOB);
        lv_obj_add_style(m_signal_data.prg_bar_quality, &m_quality_main, LV_PART_MAIN);
        lv_obj_add_style(m_signal_data.prg_bar_quality, &m_quality_indicator, LV_PART_INDICATOR);
        lv_obj_align(m_signal_data.prg_bar_quality, LV_ALIGN_CENTER, 0, 50);
        lv_bar_set_value(m_signal_data.prg_bar_quality, 0, LV_ANIM_OFF);
        m_signal_data.lbl_quality = set_label_text_static(m_bgd, "", 0, 0, font_semi_30, OSD_COLOR_WHITE);
        lv_obj_align(m_signal_data.lbl_quality, LV_ALIGN_CENTER, 0, 10);
        m_signal_data.prg_bar_strength = lv_slider_create(m_bgd);
        lv_obj_remove_style_all(m_signal_data.prg_bar_strength);
        lv_obj_set_width(m_signal_data.prg_bar_strength, width_event / 2);
        lv_obj_set_height(m_signal_data.prg_bar_strength, 4);
        lv_obj_add_style(m_signal_data.prg_bar_strength, &m_strength_knob, LV_PART_KNOB);
        lv_obj_add_style(m_signal_data.prg_bar_strength, &m_strength_main, LV_PART_MAIN);
        lv_obj_add_style(m_signal_data.prg_bar_strength, &m_strength_indicator, LV_PART_INDICATOR);
        lv_obj_align(m_signal_data.prg_bar_strength, LV_ALIGN_CENTER, 0, 110);
        lv_bar_set_value(m_signal_data.prg_bar_strength, 0, LV_ANIM_OFF);
        m_signal_data.lbl_strength = set_label_text_static(m_bgd, "", 0, 0, font_semi_30, OSD_COLOR_WHITE);
        lv_obj_align(m_signal_data.lbl_strength, LV_ALIGN_CENTER, 0, 80);
        m_tmr_signal_data = lv_timer_create(update_antenna_signal_cb, 350,  &m_signal_data);
        update_antenna_signal_cb(m_tmr_signal_data);
        m_btn_back = create_rect(m_bgd, 0, 0, 250, 50, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(m_btn_back, 25, DEFAULT_SELECTOR);
        lv_obj_align(m_btn_back, LV_ALIGN_BOTTOM_MID, -150, -40);
        m_lbl_back = set_label_text_static(m_btn_back, tr(__Nao), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_lbl_back, LV_ALIGN_CENTER, 0, 0);
        m_btn_next = create_rect(m_bgd, 0, 0, 250, 50, OSD_COLOR_ORANGE);
        lv_obj_set_style_radius(m_btn_next, 25, DEFAULT_SELECTOR);
        lv_obj_align(m_btn_next, LV_ALIGN_BOTTOM_MID, 150, -40);
        m_lbl_next = set_label_text_static(m_btn_next, tr(__Sim), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_lbl_next, LV_ALIGN_CENTER, 0, 0);
        move_right();
    }
}

void OSD_Fast_Install::set_menu_selection()
{
    if (m_selected_item == 1)
    {
        lv_obj_set_style_bg_color(m_btn_next, OSD_COLOR_ORANGE, 0);
        lv_obj_set_style_bg_color(m_btn_back, OSD_COLOR_BLACK, 0);
    }
    else if (m_selected_item == 0)
    {
        lv_obj_set_style_bg_color(m_btn_next, OSD_COLOR_BLACK, 0);
        lv_obj_set_style_bg_color(m_btn_back, OSD_COLOR_ORANGE, 0);
    }
}

void OSD_Fast_Install::move_right()
{
    m_selected_item = 1;
    set_menu_selection();
}

void OSD_Fast_Install::move_left()
{
    m_selected_item = 0;
    set_menu_selection();
}

void OSD_Fast_Install::hide_menu()
{
    if (m_tmr_signal_data)
    {
        lv_timer_delete(m_tmr_signal_data);
        m_tmr_signal_data = nullptr;
    }

    DELETE_OBJ(m_strength_line);
    DELETE_OBJ(m_quality_line);
    DELETE_OBJ(m_strength_line);
    DELETE_OBJ(m_bgd);
    DELETE_OBJ(m_btn_back);
    DELETE_OBJ(m_lbl_back);
    DELETE_OBJ(m_btn_next);
    DELETE_OBJ(m_lbl_next);
    DELETE_OBJ(m_lbl_title);
    DELETE_OBJ(m_lbl_message);
    DELETE_OBJ(m_bgd_main);
    remove_focus();
    Task::post_event(m_callback);
}

bool OSD_Fast_Install::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
            //if (m_selected_item == 0)
        {
            hide_menu();
        }

#if 0
        else if (m_selected_item == 1)
        {
            if (!m_if_channel_list)
            {
                m_if_channel_list = std::make_unique<OSD_Instala_Facil_Lista_Canais>();
            }

            m_if_channel_list->show_menu(m_bgd_main, ([this]/*(bool _b_exit)*/
            {
                DEBUG_MSG("Apagando tela Instala Facil Lista de Canais\n");
                m_if_channel_list.reset();
                //hide_menu();
                /*
                                    if(_b_exit)
                                    {
                                        Task::post_event(m_callback);
                                        remove_focus();
                                    }
                */
            }));
        }

#endif
        return true;

        case Remote_Control_Key::KEY_VOLTAR:
            hide_menu();
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

    return false;
}

void OSD_Fast_Install::got_focus()
{
    hide_menu();
    remove_focus();
}

} // namespace mb
