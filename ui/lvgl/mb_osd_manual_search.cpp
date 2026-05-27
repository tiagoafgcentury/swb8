#include "mb_osd_manual_search.h"

#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "mb_menu_resources.h"
#include "mb_osd_footer.h"
#include "mb_osd_fonts.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_tuner.h"
#include "hal/mb_sound.h"

#include <functional>
#include <lvgl.h>

namespace mb {

OSD_Manual_Search::OSD_Manual_Search(OSD *_parent):
    OSD(_parent),
    m_keys(0, 0, keys_w, keys_h, 0, keys_x, keys_y)
{
    Task::post_event_player_stop();
}

OSD_Manual_Search::~OSD_Manual_Search()
{
    //Task::post_event_player_restart();
    close_screen(false);
}

bool OSD_Manual_Search::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLUP:
        {
            m_keys.next();
            return true;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            m_keys.previous();
            return true;
        }

        case Remote_Control_Key::KEY_CHUP:
        {
            m_keys.previous_line();
            return true;
        }

        case Remote_Control_Key::KEY_CHDOWN:
        {
            m_keys.next_line();
            return true;
        }

        case Remote_Control_Key::KEY_VOLTAR:
            close_screen(false);
            return true;

        default:
        {
            m_pressed_key = _event.key;
            auto exec_func = m_keys.get_selected_callback();
            if (exec_func) exec_func();
            return true;
        }
    }
}

void OSD_Manual_Search::show_manual_search_list(manual_search_callback_t _callback)
{
    set_focus();
    m_callback = _callback;

    if (!m_bgd)
    {
        m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, height, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd);

        auto title = set_label_text_static(m_bgd, tr(__Busca_manual), 0, 0, font_bold_40, OSD_COLOR_WHITE);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

        create_quality_slider();
        create_strength_slider();
        create_snr_box();
        start_refresh_timer();
        draw_keys();
    }
}

void OSD_Manual_Search::draw_keys()
{
    m_keys.set_background(m_bgd);
    m_keys.set_horizontal();
    m_keys.set_spacing(keys_x_spacing,keys_y_spacing);
    // Load satellite list from globals
    m_satellites = Config::get_config()->get_satellite_list();
    auto name = m_satellites[m_selected_satellite].name;
    m_keys.add_label(name, std::bind(&OSD_Manual_Search::on_satellite_callback, this), static_cast<uint8_t>(Key_Index::Satellite_Name));
    auto polarity = to_str(m_polarity);
    m_keys.add_label(polarity, std::bind(&OSD_Manual_Search::on_polarity_callback, this), static_cast<uint8_t>(Key_Index::Polarity));
    auto freq = std::to_string(m_frequency) + " MHz";
    m_keys.add_label(freq, std::bind(&OSD_Manual_Search::on_frequency_callback, this), static_cast<uint8_t>(Key_Index::Frequency));
    auto sr = std::to_string(m_symbol_rate) + " Kbps";
    m_keys.add_label(sr, std::bind(&OSD_Manual_Search::on_symbol_rate_callback, this), static_cast<uint8_t>(Key_Index::Symbol_Rate));
    m_keys.set_x(keys_x);
    m_keys.set_columns(2);
    m_keys.set_lines(2);
    // Tecla fora do grid
    m_keys.add_group();
    m_keys.set_x(buscar_keys_x);
    m_keys.set_y(buscar_keys_y);
    m_keys.set_lines(1);
    m_keys.set_columns(1);
    m_keys.set_disabled(static_cast<uint8_t>(Key_Index::Buscar));
    m_keys.add_label(tr(__Buscar), std::bind(&OSD_Manual_Search::on_buscar_callback, this), static_cast<uint8_t>(Key_Index::Buscar));
    m_keys.draw();
    m_keys.select(static_cast<uint8_t>(Key_Index::Satellite_Name));
}

void OSD_Manual_Search::create_snr_box()
{
    m_snr_box = create_rect(m_bgd, 0, 0, snr_box_width, snr_box_height, OSD_COLOR_BLACK);
    lv_obj_align(m_snr_box, LV_ALIGN_CENTER, snr_box_x, snr_box_y);
    lv_obj_null_on_delete(&m_snr_box);
    m_snr_title = set_label_text(m_snr_box, "SNR", 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr_title);
    lv_obj_align(m_snr_title, LV_ALIGN_CENTER, 0, -20);
    m_snr_value = set_label_text(m_snr_box, "0 dB", 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr_value);
    lv_obj_align(m_snr_value, LV_ALIGN_CENTER, 0, 20);
}

void OSD_Manual_Search::create_quality_slider()
{
    if (!m_quality_slider)
    {
        lv_style_init(&m_quality_main);
        lv_style_init(&m_quality_indicator);
        lv_style_set_bg_opa(&m_quality_main, LV_OPA_COVER);
        lv_style_set_bg_color(&m_quality_main, OSD_COLOR_GREY_MEDIUM);
        lv_style_set_bg_color(&m_quality_indicator, OSD_COLOR_GREEN);
        m_quality_slider = lv_bar_create(m_bgd);
        lv_obj_add_style(m_quality_slider, &m_quality_main, LV_PART_MAIN);
        lv_obj_add_style(m_quality_slider, &m_quality_indicator, LV_PART_INDICATOR);
        lv_obj_set_size(m_quality_slider, slider_width, slider_height);
        lv_obj_align(m_quality_slider, LV_ALIGN_CENTER, 0, 110);
        m_quality_line = set_label_text(m_bgd, tr(__Qualidade), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_quality_line, LV_ALIGN_CENTER, 0, 80);
        lv_slider_set_value(m_quality_slider, 0, LV_ANIM_ON);
    }
}

void OSD_Manual_Search::create_strength_slider()
{
    if (!m_strength_slider)
    {
        lv_style_init(&m_strength_main);
        lv_style_init(&m_strength_indicator);
        lv_style_set_bg_opa(&m_strength_main, LV_OPA_COVER);
        lv_style_set_bg_color(&m_strength_main, OSD_COLOR_GREY_MEDIUM);
        lv_style_set_bg_color(&m_strength_indicator, OSD_COLOR_YELLOW);
        m_strength_slider = lv_bar_create(m_bgd);
        lv_obj_add_style(m_strength_slider, &m_strength_main, LV_PART_MAIN);
        lv_obj_add_style(m_strength_slider, &m_strength_indicator, LV_PART_INDICATOR);
        lv_obj_set_size(m_strength_slider, slider_width, slider_height);
        lv_obj_align(m_strength_slider, LV_ALIGN_CENTER, 0, 180);
        m_strength_line = set_label_text(m_bgd, tr(__Sinal), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_strength_line, LV_ALIGN_CENTER, 0, 150);
        lv_slider_set_value(m_strength_slider, 0, LV_ANIM_ON);
    }
}

void OSD_Manual_Search::start_refresh_timer()
{
    if (m_refresh_timer)
    {
        return;
    }

    m_refresh_timer = lv_timer_create([](lv_timer_t *tm)
    {
        if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
        {
            return;
        }
        auto instance = static_cast<OSD_Manual_Search *>(lv_timer_get_user_data(tm));
        instance->refresh_signal_info();
    }, 1000, this);
    lv_timer_set_repeat_count(m_refresh_timer, -1);

    if (!m_beep_timer)
    {
        m_beep_timer = lv_timer_create(beep_cb, 50, this);
        lv_timer_set_repeat_count(m_beep_timer, -1);
    }
}

void OSD_Manual_Search::refresh_signal_info()
{
    if (!m_refresh_signal_info)
    {
        return;
    }

    auto signal_info = Task_Tuner::get_signal_info();
    char buffer[1024];
    double snr = signal_info.signal_noise_ratio / 100.0;
    int quality = signal_info.quality / 10;
    m_last_snr = snr;

    auto color = quality < 50 ? OSD_COLOR_RED : quality < 70 ? OSD_COLOR_YELLOW : OSD_COLOR_GREEN;
    lv_style_set_bg_color(&m_quality_indicator, color);

    lv_slider_set_value(m_quality_slider, quality, LV_ANIM_ON);
    snprintf(buffer, sizeof(buffer) - 1, "%s: %d%%", tr(__Qualidade).data(), quality);
    lv_label_set_text(m_quality_line, buffer);
    int signal = signal_info.strength;
    lv_slider_set_value(m_strength_slider, signal, LV_ANIM_ON);
    snprintf(buffer, sizeof(buffer) - 1, "%s: %d%%", tr(__Sinal).data(), (int)signal_info.strength);
    lv_label_set_text(m_strength_line, buffer);
    snprintf(buffer, sizeof(buffer) - 1, "%0.2f dB", snr);
    lv_label_set_text(m_snr_value, buffer);
}

void OSD_Manual_Search::close_screen(bool _result)
{
    remove_focus();
    DELETE_TIMER(m_beep_timer);
    Sound::get_instance()->stop_tone();
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_quality_slider);
    DELETE_OBJ(m_quality_line);
    DELETE_OBJ(m_strength_slider);
    DELETE_OBJ(m_strength_line);
    DELETE_OBJ(m_snr_box);
    DELETE_OBJ(m_snr_title);
    DELETE_OBJ(m_snr_value);
    DELETE_OBJ(m_bgd);

    lv_style_reset(&m_quality_main);
    lv_style_reset(&m_quality_indicator);
    lv_style_reset(&m_strength_main);
    lv_style_reset(&m_strength_indicator);

    if (m_callback)
    {
        Task::post_event(std::bind(m_callback, _result));
    }
}

void OSD_Manual_Search::on_satellite_callback()
{
    if (m_pressed_key != Remote_Control_Key::KEY_OK)
    {
        not_implemented_response(m_pressed_key);
        return;
    }
    // Cycle through satellites
    m_selected_satellite = (m_selected_satellite + 1) % m_satellites.size();
    auto name = m_satellites[m_selected_satellite].name;
    m_keys.set_label(static_cast<uint8_t>(Key_Index::Satellite_Name), name);
    // If satellite is mandatory, disable buscar_key
    if (m_satellites[m_selected_satellite].is_mandatory)
    {
        m_keys.set_disabled(static_cast<uint8_t>(Key_Index::Buscar));
    }
    else
    {
        m_keys.set_enabled(static_cast<uint8_t>(Key_Index::Buscar));
    }

    m_tp.set_satellite_id(m_satellites[m_selected_satellite].id);
    start_lock_timer();
}

void OSD_Manual_Search::on_polarity_callback()
{
    if (m_pressed_key != Remote_Control_Key::KEY_OK)
    {
        not_implemented_response(m_pressed_key);
        return;
    }
    // Toggle polarity
    m_polarity = m_polarity == Polarity::Horizontal ? Polarity::Vertical : Polarity::Horizontal;
    auto polarity = to_str(m_polarity);
    m_keys.set_label(static_cast<uint8_t>(Key_Index::Polarity), polarity);
    start_lock_timer();
}

void OSD_Manual_Search::on_frequency_callback()
{
    auto num = to_int(m_pressed_key);
    if (num < 0)
    {
        not_implemented_response(m_pressed_key);
        return;
    }

    // Append digit to frequency
    m_frequency = m_frequency * 10 + static_cast<uint32_t>(num);
    m_frequency = m_frequency > 12750 ? static_cast<uint32_t>(num) : m_frequency;
    auto freq = std::to_string(m_frequency) + " MHz";
    m_keys.set_label(static_cast<uint8_t>(Key_Index::Frequency), freq);
    start_lock_timer();
}

void OSD_Manual_Search::on_symbol_rate_callback()
{
    auto num = to_int(m_pressed_key);
    if (num < 0)
    {
        not_implemented_response(m_pressed_key);
        return;
    }

    // Append digit to frequency
    m_symbol_rate = m_symbol_rate * 10 + static_cast<uint32_t>(num);
    m_symbol_rate = m_symbol_rate > 45000 ? static_cast<uint32_t>(num) : m_symbol_rate;
    auto sr = std::to_string(m_symbol_rate) + " Kbps";
    m_keys.set_label(static_cast<uint8_t>(Key_Index::Symbol_Rate), sr);
    start_lock_timer();
}

void OSD_Manual_Search::on_buscar_callback()
{
    if (m_pressed_key != Remote_Control_Key::KEY_OK)
    {
        not_implemented_response(m_pressed_key);
        return;
    }

    std::vector<Transponder> tp_list;
    Transponder tp = 
    { 
        Transponder_Id{m_frequency * 1000, m_polarity, m_satellites[m_selected_satellite].id},
        m_symbol_rate,
        DVB_Mode::DVBS2,
        101,
        Network_Id_Claro,
        Network_Id_Claro,
        false
    };
    tp.satellite_id = m_satellites[m_selected_satellite].id;

    tp_list.push_back(tp);
    // Set config with current satellite and transponder
    Config::get_config()->set_satellite_config_by_id(m_satellites[m_selected_satellite].id, tp_list);

    using namespace std::placeholders;
    auto config = Config::get_config();
    auto satellite_list = config->get_satellite_list();

    bool has_sky = std::any_of(satellite_list.begin(), satellite_list.end(), [](const Satellite &s) {
        return s.id == 2 && s.switch_type != DiseqC_Type::None;
    });
    bool has_claro = std::any_of(satellite_list.begin(), satellite_list.end(), [](const Satellite &s) {
        return s.id == 1 && s.switch_type != DiseqC_Type::None;
    });

    m_channel_list_update_callback = std::make_shared<Event_List_Update>(Event_List_Update{
        .callback = std::bind(&OSD_Manual_Search::channel_list_update_callback, this, _1),
        .partial_callback = std::bind(&OSD_Manual_Search::channel_list_update_partial_callback, this, _1, _2),
        .clear_table = false,
        .emit_lineup_ready = true,
        .scan_sat_count = 1,
        .scan_sat_index = 0,
        .has_sky = has_sky,
        .has_claro = has_claro
    });
    Task::post_event_lineup_build(m_channel_list_update_callback);
}

void OSD_Manual_Search::channel_list_update_callback(bool _is_done)
{
    DEBUG_MSG(OSD, DEBUG, "Channel list update completed with result: " << (_is_done ? "success" : "failure") << "\n");
    if (_is_done)
    {
        Task::post_event_lineup_save();
        close_screen(true);
    }
}

void OSD_Manual_Search::channel_list_update_partial_callback(size_t /*_transponder_seq*/, std::vector<Service> /*_services*/)
{
    // This can be used to update the UI with the progress of the channel list update, if desired.
    DEBUG_MSG(OSD, DEBUG, "Channel list update in progress...\n");
}

void OSD_Manual_Search::not_implemented_response(Remote_Control_Key _key)
{
    DEBUG_MSG(OSD, DEBUG, "Function not implemented yet for key \"" << to_str(_key) << "\"\n");
}

void OSD_Manual_Search::lock_transponder()
{
    // This function would lock the tuner to the selected satellite, frequency, symbol rate and polarity.
    if ( m_frequency < 12750 && m_frequency > 10000 && m_symbol_rate > 1000 && m_symbol_rate < 45000)
    {
        m_tp.transponder_id.set_frequency(m_frequency * 1000, m_polarity, m_tp.satellite_id);
        m_tp.symbol_rate = m_symbol_rate;
        m_tp.dvb_mode = DVB_Mode::DVBS2;
        Task::post_event_transponder_lock(POST_CALLER &m_tp);
        m_refresh_signal_info = true;
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "Invalid frequency or symbol rate\n");
        if (m_refresh_signal_info)
        {
            m_refresh_signal_info = false;
            // Reset signal info display
            lv_slider_set_value(m_quality_slider, 0, LV_ANIM_ON);
            lv_slider_set_value(m_strength_slider, 0, LV_ANIM_ON);
            lv_label_set_text(m_quality_line, tr(__Qualidade).data());
            lv_label_set_text(m_strength_line, tr(__Sinal).data());
            lv_label_set_text(m_snr_value, "0 dB");
        }
    }
}

void OSD_Manual_Search::beep_cb(lv_timer_t* tm)
{
    if (!tm || !lv_timer_get_user_data(tm))
        return;

    auto thiz = static_cast<OSD_Manual_Search*>(lv_timer_get_user_data(tm));
    thiz->process_beep();
}

void OSD_Manual_Search::process_beep()
{
    double snr = m_last_snr;

    int range = 0;

    if (snr > 12.0)          range = 3;
    else if (snr > 11.0)     range = 2;
    else if (snr > 1.0)      range = 1;
    else                     range = 0;

    if (range != m_last_range)
    {
        m_last_range = range;
        Sound::get_instance()->stop_tone();
        m_beep_on = false;
    }

    if (range == 0)
    {
        Sound::get_instance()->stop_tone();
        return;
    }

    if (range == 3)
    {
        if (!m_beep_on)
        {
            Sound::get_instance()->set_tone(50);
            m_beep_on = true;
        }
        return;
    }

    static uint32_t counter = 0;
    counter += 50;

    uint32_t on_time  = (range == 2) ? 100 : 200;
    uint32_t off_time = (range == 2) ? 100 : 400;

    if (m_beep_on && counter >= on_time)
    {
        Sound::get_instance()->stop_tone();
        m_beep_on = false;
        counter = 0;
    }
    else if (!m_beep_on && counter >= off_time)
    {
        Sound::get_instance()->set_tone(50);
        m_beep_on = true;
        counter = 0;
    }
}

void OSD_Manual_Search::start_lock_timer()
{
    if (m_lock_timer)
    {
        lv_timer_reset(m_lock_timer);
        return;
    }

    m_lock_timer = lv_timer_create([](lv_timer_t* tm)
    {
        auto thiz = static_cast<OSD_Manual_Search*>(lv_timer_get_user_data(tm));
        thiz->lock_transponder();
        lv_timer_del(tm);
        thiz->m_lock_timer = nullptr;
    }, 1000, this);

    lv_timer_set_repeat_count(m_lock_timer, 1);
}

} // namespace mb
