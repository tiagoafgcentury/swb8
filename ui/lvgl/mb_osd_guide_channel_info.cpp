#include "mb_osd_guide_channel_info.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_data.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include <hal/mb_system.h>

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_eit_events.h"
#include "tasks/mb_task_tuner.h"
#include "mb_events.h"

#include <lvgl.h>

namespace mb {

OSD_Guide_Channel_Info::OSD_Guide_Channel_Info(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_quality_main);
    lv_style_init(&m_quality_indicator);
    lv_style_init(&m_strength_main);
    lv_style_init(&m_strength_indicator);
}

OSD_Guide_Channel_Info::~OSD_Guide_Channel_Info()
{
    lv_style_reset(&m_quality_main);
    lv_style_reset(&m_quality_indicator);
    lv_style_reset(&m_strength_main);
    lv_style_reset(&m_strength_indicator);
    Osd_Breadcrumb::s_instance.remove_name();
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_bgd);
    remove_focus();
}

bool OSD_Guide_Channel_Info::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(std::bind(m_callback, false));
            return true;

        default:
            return true;
    }

    return false;
}

// Inicia objetos no construtor
void OSD_Guide_Channel_Info::channel_detail_frame()
{
    m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd);
    lv_obj_set_style_bg_opa(m_bgd, LV_OPA_TRANSP, 0);
    create_rect(m_bgd, 1207, 120, 100, 200, OSD_COLOR_BLACK);
    m_box_info = create_rect(m_bgd, 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    m_fav_top = load_image(m_box_info, LOGO_FAVORITOS_BRANCO_27x25, START_POS_X, 2, 40, 40);
    lv_obj_null_on_delete(&m_fav_top);
    lv_obj_add_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    m_channel_title = set_label_text(m_box_info, "", START_POS_X, 0, font_bold_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_channel_title);
    lv_label_set_long_mode(m_channel_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_channel_title, 850);
    lv_obj_set_height(m_channel_title, 45);
    //linhas com informações do satelite
    m_satellite_line = set_label_text(m_box_info, "", START_POS_X, 61, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_satellite_line);
    m_service_line = set_label_text(m_box_info, "", START_POS_X, 94, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_service_line);
    m_box_main = create_rect(m_bgd, 0, 300, DISPLAY_WIDTH, 430, OSD_COLOR_BLACK);
    // Bloco com informações do programa atual
    m_current_event_title = set_label_text(m_box_main, "", START_POS_X, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_event_title);
    lv_label_set_long_mode(m_current_event_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_current_event_title, 350);
    lv_obj_set_height(m_current_event_title, 40);
    m_current_event_time = set_label_text(m_box_main, "", START_POS_X, 37, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_event_time);
    m_current_event_subtitle = set_label_text(m_box_main, "", START_POS_X, 75, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_event_subtitle);
    lv_label_set_long_mode(m_current_event_subtitle, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_current_event_subtitle, 360);
    lv_obj_set_height(m_current_event_subtitle, 115);
    m_center_line = create_rect(m_box_main, 473, 0, 3, 190, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_center_line);
    m_next_event_title = set_label_text(m_box_main, "", 496, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_event_title);
    lv_label_set_long_mode(m_next_event_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_next_event_title, 350);
    lv_obj_set_height(m_next_event_title, 40);
    m_next_event_time = set_label_text(m_box_main, "", 496, 37, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_event_time);
    m_next_event_subtitle = set_label_text(m_box_main, "", 496, 75, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_event_subtitle);
    lv_label_set_long_mode(m_next_event_subtitle, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_next_event_subtitle, 360);
    lv_obj_set_height(m_next_event_subtitle, 115);
    // Slider de qualidade e sinal
    create_quality_slider();
    create_strength_slider();
    m_snr_title = set_label_text(m_box_main, "SNR", 690, 226, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr_title);
    m_snr_value = set_label_text(m_box_main, "", 660, 280, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr_value);
    load_image(m_box_main, LOGO_MIDIABOX_BRANCO_256x37, 933, 292, 256, 37);
    Osd_Breadcrumb::s_instance.add_name(tr(__Mais_Informacoes));
    auto bgd_bottom = create_rect(m_bgd, 0, 630, DISPLAY_WIDTH, 90, OSD_COLOR_BLACK);
    MB_OSD_Footer::draw(bgd_bottom, tr(__Pressione_voltar_para_voltar), -40);
}

void OSD_Guide_Channel_Info::print_channel_info()
{
    if (!m_srv)
    {
        DEBUG_MSG(OSD, WARN, "Show channel info: <null>\n");
        return;
    }

    // Exibe ou esconde estrela de favorito
    char channel_number[20] = {0};
    int ch = m_srv->viewer_channel();
    int width = (ch >= 10000) ? 5 : 4;

    snprintf(channel_number, sizeof(channel_number), "%0*d", width, ch);

    if (m_srv->is_favorite())
    {
        std::string str = "     " + std::string(channel_number) + " - " + m_srv->name().data();
        lv_label_set_text(m_channel_title, str.c_str());
        // Exibe estrela
        lv_obj_remove_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        std::string str = std::string(channel_number) + " - " + m_srv->name().data();
        lv_label_set_text(m_channel_title, str.c_str());
        lv_obj_add_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    }

    // Informações do transponder sintonizado
    auto frequency = m_srv->transponder_id().frequency() / 1000;
    auto polarity = m_srv->transponder_id().polarity();
    auto pol = polarity == Polarity::Horizontal ? "H" : "V";
    auto tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(m_srv->transponder_id());
    auto sr = tp ? tp->symbol_rate : 0;
    char buf[1024];
    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    snprintf(buf, sizeof(buf) - 1, "%s: %s %s | %dMHz/%dKbps | %s", tr(__Satelite).data(), sat_config.name.data(), config->band() == Band::Ku ? "KU" : "C", frequency, sr, pol);
    lv_label_set_text(m_satellite_line, buf);
    // Informações do canal sintonizados
    const Service::AudioPid *audio_service { nullptr };

    if (!m_srv->audio_pids().empty())
    {
        audio_service = &m_srv->current_audio();
    }

    std::string audio = audio_service ? std::to_string(audio_service->pid) : "";
    std::string_view audio_codec = audio_service ? to_str(audio_service->codec).data() : "";
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "%s: %d | %s: %s | PCR: %d | %s: %s | %s: %s | %s: %d",
             tr(__Video).data(), m_srv->video_pid(),
             tr(__Tipo).data(), to_str(m_srv->video_codec()).data(),
             m_srv->pcr_pid(),
             tr(__Audio).data(), audio.data(),
             tr(__Tipo).data(), audio_codec.data(),
             tr(__Servico).data(), m_srv->service_id());
    lv_label_set_text(m_service_line, buffer);
    parse_events();
    std::string curr_program_txt = "     " + m_current.program;
    Parental_Control curr_pr = m_current.parental_rating;
    m_current_class_indic = classificacao_indicativa(m_box_main, curr_pr, curr_parental_rating_x, curr_parental_rating_y, 0);
    lv_obj_set_size(m_current_class_indic, 27, 28);
    // Exibe informações do evento atual e do próximo evento
    std::string curr_time = m_current.start_str + " - " + m_current.end_str;
    lv_label_set_text(m_current_event_title, curr_program_txt.c_str());
    lv_label_set_text(m_current_event_time, curr_time.c_str());
    lv_label_set_text(m_current_event_subtitle, m_current.detail.c_str());
    std::string next_program_txt = "     " + m_next.program;
    Parental_Control next_pr = m_next.parental_rating;
    m_next_class_indic = classificacao_indicativa(m_box_main, next_pr, next_parental_rating_x, next_parental_rating_y, 0);
    lv_obj_set_size(m_next_class_indic, 27, 28);
    std::string nxt_time = m_next.start_str + " - " + m_next.end_str;
    lv_label_set_text(m_next_event_title, next_program_txt.c_str());
    lv_label_set_text(m_next_event_time, nxt_time.c_str());
    lv_label_set_text(m_next_event_subtitle, m_next.detail.c_str());
}

void OSD_Guide_Channel_Info::show_menu_info(OSD_Guide_Channel_Info_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    m_srv = Task::s_task_player->current_srv();
    channel_detail_frame();
    print_channel_info();
    start_refresh_timer();
}

void OSD_Guide_Channel_Info::parse_events()
{
    // Busca informações do programa atual
    auto [current_event, next_event] = Task_EIT_Events::get_event_for_service(m_srv);

    // Inicia variáveis do canal
    if (current_event)
    {
        // Dados do evento atual
        m_current.program = current_event->short_event_descriptor;
        m_current.detail = current_event->extended_event_descriptor;
        auto current_start_time = current_event->start_time.to_local_time();
        m_current.start_str = current_start_time.time_to_str();
        m_current.start_hour = current_start_time.hour();
        m_current.start_min = current_start_time.minute();
        auto current_end_time = current_start_time + current_event->duration;
        m_current.end_str = current_end_time.time_to_str();
        m_current.end_hour = current_end_time.hour();
        m_current.end_min = current_end_time.minute();
        m_current.parental_rating = static_cast<Parental_Control>(current_event->parental_rating);

        if (next_event)
        {
            m_next.program = next_event->short_event_descriptor;
            m_next.detail = next_event->extended_event_descriptor;
            auto next_start_time = next_event->start_time.to_local_time();
            m_next.start_str = next_start_time.time_to_str();
            m_next.start_hour = next_start_time.hour();
            m_next.start_min = next_start_time.minute();
            auto next_end_time = next_start_time + next_event->duration;
            m_next.end_str = next_end_time.time_to_str();
            m_next.end_hour = next_end_time.hour();
            m_next.end_min = next_end_time.minute();
            m_next.parental_rating = static_cast<Parental_Control>(next_event->parental_rating);
        }
        else
        {
            m_next.program = NULL_CHANNEL;
            m_next.detail = NULL_TIME;
            m_next.start_hour = 0;
            m_next.start_min = 0;
            m_next.start_str = NULL_TIME;
            m_next.end_hour = 0;
            m_next.end_min = 0;
            m_next.end_str = NULL_TIME;
            m_next.parental_rating = Parental_Control::CLASSIFICACAO_INDEFINIDA;
        }
    }
    else
    {
        m_current.program = NULL_CHANNEL;
        m_current.detail = NULL_CHANNEL;
        m_current.start_hour = 0;
        m_current.start_min = 0;
        m_current.start_str = NULL_TIME;
        m_current.end_str = NULL_TIME;
        m_current.end_hour = 0;
        m_current.end_min = 0;
        m_current.parental_rating = Parental_Control::CLASSIFICACAO_INDEFINIDA;
        m_next.program = NULL_CHANNEL;
        m_next.detail = NULL_CHANNEL;
        m_next.start_hour = 0;
        m_next.start_min = 0;
        m_next.start_str = NULL_TIME;
        m_next.end_str = NULL_TIME;
        m_next.end_hour = 0;
        m_next.end_min = 0;
        m_next.parental_rating = Parental_Control::CLASSIFICACAO_INDEFINIDA;
    }
}

void OSD_Guide_Channel_Info::start_refresh_timer()
{
    // Inicia timer para apagar informações do canal
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_ts_signal_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }
}

void OSD_Guide_Channel_Info::refresh_ts_signal_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    OSD_Guide_Channel_Info *thiz = static_cast<OSD_Guide_Channel_Info *>(lv_timer_get_user_data(tm));
    thiz->refresh_channel_detail();
}

void OSD_Guide_Channel_Info::refresh_channel_detail()
{
    auto signal_info = Task_Tuner::get_signal_info();
    char buffer[1024];
    float snr = signal_info.signal_noise_ratio / 100.0;
    int quality = signal_info.quality / 10;

    if (quality < 50)
    {
        lv_style_set_bg_color(&m_quality_indicator, OSD_COLOR_RED);
    }
    else if (quality > 50 && quality < 70)
    {
        lv_style_set_bg_color(&m_quality_indicator, OSD_COLOR_YELLOW);
    }
    else
    {
        lv_style_set_bg_color(&m_quality_indicator, OSD_COLOR_GREEN);
    }

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

void OSD_Guide_Channel_Info::create_quality_slider()
{
    if (m_quality_slider)
    {
        // m_quality_slider criado anteriormente
        return;
    }

    lv_style_set_bg_opa(&m_quality_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_main, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_quality_indicator, OSD_COLOR_GREEN);
    m_quality_slider = lv_bar_create(m_box_main);
    lv_obj_add_style(m_quality_slider, &m_quality_main, LV_PART_MAIN);
    lv_obj_add_style(m_quality_slider, &m_quality_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(m_quality_slider, 528, 20);
    lv_obj_align(m_quality_slider, LV_ALIGN_DEFAULT, START_POS_X, 237);
    // Texto com valores da qualidade
    m_quality_line = set_label_text(m_box_main, "", START_POS_X, 200, font_semi_25, OSD_COLOR_WHITE);
}

void OSD_Guide_Channel_Info::create_strength_slider()
{
    if (m_strength_slider)
    {
        // create_strength_slider criado anteriormente
        return;
    }

    lv_style_set_bg_opa(&m_strength_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_strength_main, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_strength_indicator, OSD_COLOR_YELLOW);
    m_strength_slider = lv_bar_create(m_box_main);
    lv_obj_add_style(m_strength_slider, &m_quality_main, LV_PART_MAIN);
    lv_obj_add_style(m_strength_slider, &m_strength_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(m_strength_slider, 528, 20);
    lv_obj_align(m_strength_slider, LV_ALIGN_DEFAULT, START_POS_X, 309);
    // Texto com valores de sinal
    m_strength_line = set_label_text(m_box_main, "", START_POS_X, 269, font_semi_25, OSD_COLOR_WHITE);
}

} // namespace mb
