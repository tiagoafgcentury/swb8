#include <lvgl.h>

#include "mb_osd_channel_detail.h"
#include "mb_osd.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_footer.h"
#include "mb_osd_fonts.h"
#include "tasks/mb_task_tuner.h"
#include "mb_osd_fonts.h"

#include "mb_menu_resources.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task_eit_events.h"
#include "../../project_version.h"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>

namespace mb {

//
//      Barra superior da tela de informações do canal

Channel_Detail::Channel_Detail(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_quality_main);
    lv_style_init(&m_quality_indicator);
    lv_style_init(&m_strength_main);
    lv_style_init(&m_strength_indicator);
    // // Monta o frame principal
    channel_detail_frame();
    // Direciona recepção de tecla
    set_focus();
}

Channel_Detail::~Channel_Detail()
{
    DELETE_OBJ(m_zone_id_label);
    DELETE_OBJ(m_zone_id_box);
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_main_board);
    lv_style_reset(&m_quality_main);
    lv_style_reset(&m_quality_indicator);
    lv_style_reset(&m_strength_main);
    lv_style_reset(&m_strength_indicator);
}

bool Channel_Detail::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Bloqueia recepção de teclas se estiver exibindo diagnóstico
    if ( m_block_cr )
    {
        return true;
    }

    // Scan m_incomming_keys variable
    switch (_event.key)
    {

        case Remote_Control_Key::KEY_OK:
            verify_sequence();
            return true;

        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event(m_callback);
            return true;
        }

        case Remote_Control_Key::KEY_PLUS:
        {
            show_diagnostics();
            return true;
        }

        default:
        {
            m_incomming_keys.push_back(_event.key);
            DEBUG_MSG(OSD, INFO, "Tecla pressionada: " << to_str(_event.key) << "\n");
            DEBUG_MSG(OSD, INFO, "Teclas pressionadas até agora: " << m_incomming_keys.size() << "\n");
            return true;
        }
    }
    return true;
}

void Channel_Detail::show_channel_detail(Channel_Detail_CB_t _callback)
{
    DEBUG_MSG(OSD, DEBUG, "Exibindo informações detalhadas do canal\n");
    m_callback = _callback;

    std::map<std::string, std::string> channel_info = {};
    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        channel_info = current_lineup->get_current_lock_info();
    }
    // Exibe ou esconde estrela de favorito
    char channel_number[20] = {0};
    int ch = channel_info["viewer_channel"].empty() ? 0 : std::stoi(channel_info["viewer_channel"]);
    int width = (ch >= 10000) ? 5 : 4;
    snprintf(channel_number, sizeof(channel_number), "%0*d", width, ch);

    if (channel_info["is_favorite"] == "1")
    {
        std::string str = "     " + std::string(channel_number) + " - " + channel_info["service_name"];
        lv_label_set_text(m_channel_title, str.data());
        // Exibe estrela
        lv_obj_remove_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        std::string str = std::string(channel_number) + " - " + channel_info["service_name"];
        lv_label_set_text(m_channel_title, str.data());
        lv_obj_add_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    }

    // Busca informações de eventos do canal atual
    parse_events();
    // Informações do transponder sintonizado
    char buf[1024];
    snprintf(buf, sizeof(buf) - 1, "%s: %s %s | %s/%s | %s",
        tr(__Satelite).data(),
        channel_info["satellite_name"].data(),
        channel_info["band"].data(),
        channel_info["frequency"].data(),
        channel_info["symbol_rate"].data(),
        channel_info["polarity"].data());
    lv_label_set_text(m_satellite_line, buf);
    // Informações do canal sintonizados
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "%s: %s | %s: %s | PCR: %s | %s: %s | %s: %s | %s: %s",
             tr(__Video).data(), channel_info["video_pid"].data(),
             tr(__Tipo).data(), channel_info["video_codec"].data(),
             channel_info["pcr_pid"].data(),
             tr(__Audio).data(), channel_info["audio_pid"].data(),
             tr(__Tipo).data(), channel_info["audio_codec"].data(),
             tr(__Servico).data(), channel_info["service_id"].data());
    lv_label_set_text(m_service_line, buffer);

    std::string curr_program_txt = "     " + m_current.program;
    Parental_Control curr_pr = m_current.parental_rating;
    m_current_class_indic = classificacao_indicativa(m_main_board, curr_pr, curr_parental_rating_x, curr_parental_rating_y, 0);
    lv_obj_set_size(m_current_class_indic, 27, 28);
    // Exibe informações do evento atual e do próximo evento
    std::string curr_time = m_current.start_str + " - " + m_current.end_str;
    lv_label_set_text(m_current_event_title, curr_program_txt.data());
    lv_label_set_text(m_current_event_time, curr_time.data());
    lv_label_set_text(m_current_event_subtitle, m_current.detail.data());
    std::string next_program_txt = "     " + m_next.program;
    Parental_Control next_pr = m_next.parental_rating;
    m_next_class_indic = classificacao_indicativa(m_main_board, next_pr, next_parental_rating_x, next_parental_rating_y, 0);
    lv_obj_set_size(m_next_class_indic, 27, 28);
    std::string nxt_time = m_next.start_str + " - " + m_next.end_str;
    lv_label_set_text(m_next_event_title, next_program_txt.data());
    lv_label_set_text(m_next_event_time, nxt_time.data());
    lv_label_set_text(m_next_event_subtitle, m_next.detail.data());
}

void Channel_Detail::refresh_ts_signal_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    Channel_Detail *thiz = static_cast<Channel_Detail *>(lv_timer_get_user_data(tm));
    thiz->refresh_channel_detail();
}

void Channel_Detail::refresh_channel_detail()
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

void Channel_Detail::channel_detail_frame()
{
    // Cria área da tela
    m_main_board = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_board);
    lv_obj_set_style_radius(m_main_board, 0, DEFAULT_SELECTOR);
    lv_obj_set_style_bg_opa(m_main_board, LV_OPA_80, 0);
    // Tela principal
    lv_obj_center(m_main_board);
    // Barra com número e nome do canal e logo
    load_image(m_main_board, LOGO_MENU_CENTURY, START_POS_X, 42, 213, 51);
    m_fav_top = load_image(m_main_board, LOGO_FAVORITOS_BRANCO_27x25, START_POS_X, 116, 40, 40);
    lv_obj_null_on_delete(&m_fav_top);
    lv_obj_add_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    m_channel_title = set_label_text(m_main_board, "", START_POS_X, 114, font_bold_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_channel_title);
    lv_label_set_long_mode(m_channel_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_channel_title, 1050);
    lv_obj_set_height(m_channel_title, 45);
    //linhas com informações do satelite
    m_satellite_line = set_label_text(m_main_board, "", START_POS_X, 177, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_satellite_line);
    m_service_line = set_label_text(m_main_board, "", START_POS_X, 210, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_service_line);
    // Bloco com informações do programa atual
    m_current_event_title = set_label_text(m_main_board, "", START_POS_X, 276, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_event_title);
    lv_label_set_long_mode(m_current_event_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_current_event_title, 350);
    lv_obj_set_height(m_current_event_title, 40);
    m_current_event_time = set_label_text(m_main_board, "", START_POS_X, 313, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_event_time);
    m_current_event_subtitle = set_label_text(m_main_board, "", START_POS_X, 351, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_current_event_subtitle);
    lv_label_set_long_mode(m_current_event_subtitle, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_current_event_subtitle, 360);
    lv_obj_set_height(m_current_event_subtitle, 140);
    m_center_line = create_rect(m_main_board, 473, 276, 3, 190, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_center_line);
    m_next_event_title = set_label_text(m_main_board, "", 496, 276, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_event_title);
    lv_label_set_long_mode(m_next_event_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_next_event_title, 350);
    lv_obj_set_height(m_next_event_title, 40);
    m_next_event_time = set_label_text(m_main_board, "", 496, 313, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_event_time);
    m_next_event_subtitle = set_label_text(m_main_board, "", 496, 351, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_next_event_subtitle);
    lv_label_set_long_mode(m_next_event_subtitle, LV_LABEL_LONG_DOT);
    lv_obj_set_width(m_next_event_subtitle, 360);
    lv_obj_set_height(m_next_event_subtitle, 140);
    // Slider de qualidade e sinal
    create_quality_slider();
    create_strength_slider();
    m_snr_title = set_label_text(m_main_board, "SNR", 690, 537, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr_title);
    m_snr_value = set_label_text(m_main_board, "", 660, 591, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_snr_value);
    add_clock(m_main_board, 1090, 28);
    load_image(m_main_board, LOGO_MIDIABOX_BRANCO_256x37, 933, 598, 256, 37);
    // Cria rodapé
    MB_OSD_Footer::draw(m_main_board, tr(__Pressione_voltar_para_voltar), -40);
}

void Channel_Detail::verify_sequence()
{
    // Mostra as teclas pressionadas para debug
    std::string text = "Teclas pressionadas: ";
    for ( auto key : m_incomming_keys)
    {
        text += "\n\t" + std::string(to_str(key));
    }
    DEBUG_MSG(OSD, WARN, text << "\n");

    if (m_incomming_keys.size() < ARRAY_LENGTH)
    {
        m_incomming_keys.clear();
        return;
    }

    // Verifica se a sequência de teclas corresponde a algum código especial
    new_zone_id();
    duplicate_channel();
    delete_channel();
    m_incomming_keys.clear();
}

void Channel_Detail::duplicate_channel()
{
    if (m_incomming_keys.size() != ARRAY_LENGTH)
    {
        return;
    }

    for (uint i = 0; i < ARRAY_LENGTH; i++)
    {
        if (m_incomming_keys[i] != s_channel_add_keys[i])
        {
            return;
        }
    }

    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        current_lineup->clone_current_service();
        auto next_service = current_lineup->get_last_service();
        Task::post_event_channel_change(POST_CALLER next_service);
        DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << next_service->viewer_channel() << " - " << next_service->name() << "\n");
    }

    Task::post_event(m_callback);
    return;
}

void Channel_Detail::delete_channel()
{
    if (m_incomming_keys.size() != ARRAY_LENGTH)
    {
        return;
    }

    for (uint i = 0; i < ARRAY_LENGTH; i++)
    {
        if (m_incomming_keys[i] != s_channel_delete_keys[i])
        {
            return;
        }
    }

    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        auto next_service = current_lineup->get_next_service();
        current_lineup->delete_current_service();
        if(next_service)
        {
            Task::post_event_channel_change(POST_CALLER next_service);
            DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << next_service->viewer_channel() << " - " << next_service->name() << "\n");
        }
    }

    Task::post_event(m_callback);
    return;
}

void Channel_Detail::new_zone_id()
{
    // Verifica se a sequência de teclas pressionadas é válida
    for (uint i = 0; i < ARRAY_LENGTH; i++)
    {
        if (m_incomming_keys[i] != s_reference_keys[i])
        {
            return;
        }
    }

    // Sequência válida. Lê o restante das teclas de converte para inteiro
    int result = 0;

    for (uint i = ARRAY_LENGTH; i < m_incomming_keys.size() ; i++)
    {
        int value = to_int(m_incomming_keys[i]);
        if (value == -1)
        {
            // Sequência inválida
            return;
        }
        result = result * 10 + value;
        DEBUG_MSG(OSD, WARN, "Tecla convertida: " << value << "\n");
    }
    show_new_zone_id(result);
    DEBUG_MSG(OSD, WARN, "Sequência de teclas para novo zone id detectada. Zone ID: " << result << "\n");
    return;
}

void Channel_Detail::show_new_zone_id(int result)
{
    // Exibir novo zone id
    DELETE_OBJ(m_zone_id_label);
    DELETE_OBJ(m_zone_id_box);
    m_zone_id_box = create_rect(m_main_board, 0, 0, 400, 100, OSD_COLOR_GREEN);
    lv_obj_set_style_bg_opa(m_zone_id_box, LV_OPA_80, 0);
    lv_obj_set_style_radius(m_zone_id_box, 20, DEFAULT_SELECTOR);
    lv_obj_align(m_zone_id_box, LV_ALIGN_CENTER, 400, 0);
    std::string str = "Nova região: " + std::to_string(result);
    DEBUG_MSG(OSD, DEBUG, "Zone ID: " << str << "\n");
    m_zone_id_label = set_label_text(m_zone_id_box, str.data(), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_center(m_zone_id_label);
    // Salva novo zone id e salva lineup
    auto config = Config::get_config();
    Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    Task::post_event_lineup_save_zone_id(_oper, result);
    // Limpa variáveis
    m_incomming_keys.clear();
}

void Channel_Detail::parse_events()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto _srv = current_lineup->get_current_service();
    // Busca informações do programa atual
    auto [current_event, next_event] = Task_EIT_Events::get_event_for_service(_srv);
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

void Channel_Detail::start_refresh_timer()
{
    // Inicia timer para apagar informações do canal
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_ts_signal_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }
}

void Channel_Detail::create_quality_slider()
{
    if (m_quality_slider)
    {
        // m_quality_slider criado anteriormente
        return;
    }

    lv_style_set_bg_opa(&m_quality_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_quality_main, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_quality_indicator, OSD_COLOR_GREEN);
    m_quality_slider = lv_bar_create(m_main_board);
    lv_obj_add_style(m_quality_slider, &m_quality_main, LV_PART_MAIN);
    lv_obj_add_style(m_quality_slider, &m_quality_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(m_quality_slider, 528, 20);
    lv_obj_align(m_quality_slider, LV_ALIGN_DEFAULT, 90, 544);
    // Texto com valores da qualidade
    m_quality_line = set_label_text(m_main_board, "", 90, 506, font_semi_25, OSD_COLOR_WHITE);
}

void Channel_Detail::create_strength_slider()
{
    if (m_strength_slider)
    {
        // create_strength_slider criado anteriormente
        return;
    }

    lv_style_set_bg_opa(&m_strength_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_strength_main, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_strength_indicator, OSD_COLOR_YELLOW);
    m_strength_slider = lv_bar_create(m_main_board);
    lv_obj_add_style(m_strength_slider, &m_quality_main, LV_PART_MAIN);
    lv_obj_add_style(m_strength_slider, &m_strength_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(m_strength_slider, 528, 20);
    lv_obj_align(m_strength_slider, LV_ALIGN_DEFAULT, 90, 620);
    // Texto com valores de sinal
    m_strength_line = set_label_text(m_main_board, "", 90, 580, font_semi_25, OSD_COLOR_WHITE);
}

void Channel_Detail::show_diagnostics()
{
    m_block_cr = true;
    m_bg_cover = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    mb_osd_diagnostics = std::make_unique<OSD_Diagnosis>(this);
    mb_osd_diagnostics->show_diagnostics(std::bind(&Channel_Detail::show_diagnostics_callback, this), false);
}

void Channel_Detail::show_diagnostics_callback()
{
    DELETE_OBJ(m_bg_cover);
    mb_osd_diagnostics.reset();
    Task::post_event_player_restart();
    Task::post_event(m_callback);
}

} // namespace mb
