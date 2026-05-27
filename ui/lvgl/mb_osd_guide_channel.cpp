#include "mb_osd_guide_channel.h"
#include "mb_osd_guide_channel_info.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_data.h"
#include "mb_menu_resources.h"
#include <hal/mb_system.h>
#include "mb_osd_scheduled_edit.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_application.h"
#include "tasks/mb_task_eit_events.h"
#include "mb_events.h"

#include <lvgl.h>

#include <vector>
#include <string>
#include <ctime>
#include <sstream>

namespace mb {

OSD_Guide_Channel::OSD_Guide_Channel(OSD *_parent):
    OSD(_parent)
{
    m_start_time = System::get_system_time();//.to_local_time();
    m_current_time = m_start_time;
    lv_style_init(&m_front_style);
    lv_style_set_bg_opa(&m_front_style, LV_OPA_COVER);
    lv_style_set_bg_color(&m_front_style, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_front_style, LV_RADIUS_CIRCLE);

}

OSD_Guide_Channel::~OSD_Guide_Channel()
{
    // Limpa lista de eventos
    for (auto &box : m_event_boxes)
    {
        DELETE_OBJ(box);
    }

    for (auto &box : m_channel_boxes)
    {
        DELETE_OBJ(box);
    }
    lv_style_reset(&m_front_style);
    DELETE_TIMER(m_timer_arrow);
    DELETE_TIMER(m_tmr_channel_preview);
    DELETE_OBJ(m_error_message);
    DELETE_OBJ(m_left_arrow);
    DELETE_OBJ(m_right_arrow);
    DELETE_OBJ(m_tm_dt_box);
    DELETE_OBJ(m_tm_box);
    DELETE_OBJ(m_tm_dt_clock);
    DELETE_OBJ(m_channel_name);
    DELETE_OBJ(m_channel_desc);
    DELETE_OBJ(m_program);
    DELETE_OBJ(m_subtitle);
    DELETE_OBJ(m_class_indic);
    DELETE_OBJ(m_img_fav);
    DELETE_OBJ(m_box_info);
    DELETE_OBJ(m_box_guide);
    DELETE_OBJ(m_center_box);
    DELETE_OBJ(m_main_sub_menu);
    DELETE_OBJ(m_bgd);
    remove_focus();
}

void OSD_Guide_Channel::change_channel()
{
}

static void channel_preview_cb(lv_timer_t *timer)
{
    OSD_Guide_Channel *thiz = static_cast<OSD_Guide_Channel *>(lv_timer_get_user_data(timer));
    thiz->change_channel(false);
    lv_timer_pause(timer);
}

void OSD_Guide_Channel::change_channel(bool save_channel_info)
{
    if (m_selected_item < m_all_channel_info.size() )
    {
        auto channelData =  m_all_channel_info[m_selected_item];

        Viewer_Channel_t _viewer_channel = channelData.viewer_channel;

        for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
        {
            if (srv.viewer_channel() == _viewer_channel)
            {
                Task::post_event_channel_change(POST_CALLER & srv);
                update_channel_info(save_channel_info, &srv);
                break;
            }
        }
    }
}

void OSD_Guide_Channel::print_channel_info()
{
    int16_t width = 0;

    if (!m_channel_name and !m_channel_desc and !m_program and !m_subtitle)
    {
        //Nome do Canal
        m_channel_name = set_label_text_static(m_box_info, "", offset_x, 0, font_bold_40, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_channel_name, LV_LABEL_LONG_DOT);
        m_channel_desc = set_label_text_static(m_box_info, "", offset_x, 45, font_20, OSD_COLOR_GREY);
        m_program = set_label_text_static(m_box_info, "", offset_x, 78, font_semi_25, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_program, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_program, MENU_MAX_TEXT_WIDTH);
        m_subtitle = set_label_text_static(m_box_info, "", offset_x, 0, font_20, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_subtitle, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_subtitle, MENU_MAX_TEXT_WIDTH);
    }

    //Nome do Canal
    lv_label_set_text(m_channel_name, m_channel_text_info.channel_name.data());
    lv_obj_set_width(m_channel_name, MENU_MAX_TEXT_WIDTH);
    //Número do canal
    char channel_number[15] = {};
    int ch = m_channel_text_info.channel_number;
    int w = (ch >= 10000) ? 5 : 4;

    snprintf(channel_number, sizeof(channel_number), "%s %0*d", tr(__Canal).data(), w, ch);
    //Descricao do Canal
    std::string desc;

    //Canal favorito
    if (m_channel_text_info.favorite)
    {
        DELETE_OBJ(m_img_fav);
        m_img_fav = load_image(m_box_info, LOGO_FAVORITOS_CINZA_17x15, offset_x, 51, 17, 15);
        width = 20;
        lv_obj_set_pos(m_channel_desc, offset_x + width, 45);
        desc = " | ";
    }

    desc += channel_number;
    desc += " | ";
    desc += m_channel_text_info.channel_category;
    lv_label_set_text(m_channel_desc, desc.data());
    DELETE_OBJ(m_class_indic);
    Parental_Control curr_pr = static_cast<Parental_Control>(m_channel_text_info.parental_rating);
    m_class_indic = classificacao_indicativa(m_box_info, curr_pr, MENU_PARENTAL_RATING_X, MENU_PARENTAL_RATING_Y, 0);
    lv_obj_set_size(m_class_indic, 27, 28);
    std::string text = "     ";
    text += m_channel_text_info.channel_program;
    lv_obj_set_height(m_program, 35);
    lv_label_set_text(m_program, text.data());
    lv_obj_align(m_program, LV_ALIGN_DEFAULT,  offset_x, 75);
    lv_obj_update_layout(m_program);

    if (not m_channel_text_info.program_description.empty())
    {
        lv_label_set_text(m_subtitle, m_channel_text_info.program_description.data());
    }

    lv_obj_align(m_subtitle, LV_ALIGN_DEFAULT,  offset_x, 105);
    lv_obj_set_height(m_subtitle, 70);
}

void OSD_Guide_Channel::update_channel_info(bool save_channel_info, Service *_srv)
{
    if (_srv)
    {
        m_channel_text_info.channel_name = _srv->name();
        m_channel_text_info.channel_number = _srv->viewer_channel();
        m_channel_text_info.favorite = _srv->is_favorite();

        switch (_srv->regionalizacao())
        {
            case Regionalizacao::Regionalizado:
                m_channel_text_info.channel_category = tr(__Regionalizado);
                break;

            case Regionalizacao::RegionalizadoNacional:
                m_channel_text_info.channel_category = tr(__Regionalizado_Nacional);
                break;

            case Regionalizacao::NaoRegionalizado:
                m_channel_text_info.channel_category = tr(__Nao_Regionalizado);
                break;

            case Regionalizacao::Undefined:
            default:
                m_channel_text_info.channel_category = tr(__Indefinido);
                break;
        }
    }

    auto [current_event, _] = Task_EIT_Events::get_event_for_service(_srv);

    if (current_event)
    {
        m_channel_text_info.channel_program = current_event->short_event_descriptor;
        m_channel_text_info.parental_rating = current_event->parental_rating;
        m_channel_text_info.program_description = current_event->extended_event_descriptor;
    }
    else
    {
        m_channel_text_info.channel_program =  tr(__Sem_informacoes);
        m_channel_text_info.parental_rating = static_cast<uint8_t>(Parental_Control::CLASSIFICACAO_INDEFINIDA);
        m_channel_text_info.program_description = tr(__Sem_informacoes);
    }

    if (save_channel_info)
    {
        m_atual_srv = _srv;
    }

    print_channel_info();
}

void OSD_Guide_Channel::show_menu_guide_channel(OSD_Guide_Channel_CB_t _callback, OSD_Channels_List_Type _channel_list_type)
{
    set_focus();
    m_callback = _callback;
    //MainMenu
    m_channel_list_type = _channel_list_type;
    m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd);
    lv_obj_set_style_bg_opa(m_bgd, LV_OPA_TRANSP, 0);
    create_rect(m_bgd, 1207, 120, 100, 200, OSD_COLOR_BLACK);
    m_box_info = create_rect(m_bgd, 0, offset_y, width, height, OSD_COLOR_BLACK);
    m_box_guide = create_rect(m_bgd, 0, 295, DISPLAY_WIDTH, 480, OSD_COLOR_BLACK);
    auto bgd_bottom = create_rect(m_bgd, 0, 630, DISPLAY_WIDTH, 90, OSD_COLOR_BLACK);
    m_atual_srv = Task::s_task_player->current_srv();

    if (m_atual_srv)
    {
        update_channel_info(true, m_atual_srv);
    }

    load_image(bgd_bottom, LOGO_MAIS_27x27, 90, 10, 27, 27);
    set_label_text_static(bgd_bottom, tr(__Agendar_gravacao), 125, 10, font_20, OSD_COLOR_WHITE);
    load_image(bgd_bottom, LOGO_INFO_27x27, 390, 10, 27, 27);
    set_label_text_static(bgd_bottom, tr(__Mais_Informacoes), 425, 10, font_20, OSD_COLOR_WHITE);
    load_image(bgd_bottom, LOGO_VOLTAR_27x27, 645, 10, 27, 27);
    set_label_text_static(bgd_bottom, tr(__Voltar), 680, 10, font_20, OSD_COLOR_WHITE);
    m_logo_midia = load_image(bgd_bottom, LOGO_MIDIABOX_BRANCO_225x33, 965, 18, 225, 33);
    lv_obj_null_on_delete(&m_logo_midia);
    // Desenha caixa central
    draw_timedate_box();
    load_all_epg();
    draw_channel_list();
    draw_event_list();
    if (m_channel_list_type == OSD_Channels_List_Type::Radio_Channels)
    {
        m_radio_preview = create_rect(m_bgd, 890, offset_y, 317, height, OSD_COLOR_BLACK);
        draw_radio_box();
    }
    m_tmr_channel_preview = lv_timer_create(channel_preview_cb, 3000, this);
    //lv_timer_pause(m_tmr_channel_preview);
}

bool OSD_Guide_Channel::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(std::bind(m_callback, false));
            return true;

        case Remote_Control_Key::KEY_OK:
            change_channel(true);
            return true;

        case Remote_Control_Key::KEY_CHUP:
            move_menu_up();
            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            move_menu_down();
            return true;

        case Remote_Control_Key::KEY_VOLUP:
            move_right();
            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            move_left();
            return true;

        case Remote_Control_Key::KEY_INFO:
            m_osd_guide_channel_info = std::make_unique<OSD_Guide_Channel_Info>(this);
            m_osd_guide_channel_info->show_menu_info(std::bind(&OSD_Guide_Channel::show_menu_info_callback, this));
            return true;

        case Remote_Control_Key::KEY_PLUS:
            schedule_event();
            return true;

        default:
            return true;
    }

    return false;
}

void OSD_Guide_Channel::show_menu_info_callback()
{
    m_osd_guide_channel_info.reset();
}

void OSD_Guide_Channel::draw_radio_box()
{

    auto _front_box = create_rect(m_radio_preview, 0, 0, 300, height-16, OSD_COLOR_GREY_DARK);
    lv_obj_set_align(_front_box, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(_front_box, 25, DEFAULT_SELECTOR);
    lv_style_set_outline_width(&m_front_style, 3);
    lv_style_set_outline_color(&m_front_style, OSD_COLOR_ORANGE);
    lv_style_set_outline_pad(&m_front_style, 4);
    lv_style_set_radius(&m_front_style, 25);
    lv_obj_add_style(_front_box, &m_front_style, 0);
    auto logo_midiabox = load_image(_front_box, LOGO_MIDIABOX_BRANCO_150x22, 0, 0, 150, 22);
    lv_obj_align(logo_midiabox, LV_ALIGN_BOTTOM_MID, 0, -15);

#ifdef MBGUI_USE_RLOTTIE
    auto _soundwave = lv_rlottie_create_from_file(_front_box, 260, 140, ANIM_SOUND_WAVE);
    lv_obj_set_align(_soundwave, LV_ALIGN_CENTER);
    //lv_obj_align(_soundwave, LV_ALIGN_DEFAULT, START_POS_X, 28);
    lv_rlottie_set_play_mode(_soundwave, LV_RLOTTIE_CTRL_LOOP);
#endif

}


void OSD_Guide_Channel::draw_timedate_box()
{
    // Cria caixa com data e hora
    DELETE_OBJ(m_tm_dt_box);
    m_tm_dt_box = create_rect(m_box_guide, m_tm_dt_box_x, m_tm_dt_box_y, m_tm_dt_box_w, m_tm_dt_box_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_tm_dt_box);
    DELETE_OBJ(m_tm_box);
    m_tm_box = create_rect(m_box_guide, m_tm_box_x, m_tm_box_y, m_tm_box_w, m_tm_box_h, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_tm_box, LV_OPA_TRANSP, 0);
    lv_obj_null_on_delete(&m_tm_box);
    DELETE_OBJ(m_tm_dt_line);
    m_tm_dt_line = create_rect(m_box_guide, m_tm_box_x - 2, m_tm_box_h, m_tm_box_w, 2, OSD_COLOR_GREY_DARK);
    auto system_time = System::get_system_time().to_local_time();
    // Lê dia da semana e converte para texto
    auto wday = system_time.wday() % 7; // Ensure wday is in the range 0-6
    auto day_of_week = OSD_Translate::translate(static_cast<OSD_Translate::Text_Index>(static_cast<int>(OSD_Translate::Text_Index::__Weekday_Short_Sun) + wday));
    char date[16];
    snprintf(date, sizeof(date), "%s - %02d/%02d", day_of_week.data(), system_time.day(), system_time.month());
    m_tm_dt_clock = set_label_text(m_tm_dt_box, date, 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_center(m_tm_dt_clock);
    lv_obj_null_on_delete(&m_tm_dt_clock);
    // Carrega setas
    draw_arrow_right(LOGO_ARROW_RIGTH_20x20);
    draw_arrow_left(LOGO_ARROW_LEFT_20x20);
    // Obter a hora atual
    UTC_MJD current_time = m_current_time;

    for (int i = 0; i < 6; i++)
    {
        //DEBUG_MSG(TERM_RED_BOLD << "Current time: " << current_time.time_to_str() << "\n" << TERM_RESET);
        draw_timedate_clock(current_time);
        current_time = next_30_minutes(current_time);
    }
}

void OSD_Guide_Channel::draw_timedate_clock(UTC_MJD current_time)
{
    // Calculate the difference in minutes between m_current_time and current_time
    int difference_minutes = (current_time.to_unix_epoch() - m_current_time.to_unix_epoch()) / 60;
    difference_minutes = (difference_minutes && difference_minutes < 8) ? 8 : difference_minutes;
    //DEBUG_MSG("current_time: " << current_time.to_unix_epoch() << ", m_current_time: " << m_current_time.to_unix_epoch() << ", Difference in minutes: " << difference_minutes << "\n");
    auto ct = current_time.to_local_time();
    int x = minute_to_x(difference_minutes);
    std::string time_str = ct.time_to_str();
    auto label = set_label_text(m_tm_box, time_str, 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, x, 0);
}

void OSD_Guide_Channel::move_right()
{
    // Verifica se há eventos
    DEBUG_MSG(OSD, DEBUG, "Selected event: " << m_selected_item << "\n");
    auto events = m_all_channel_info[m_selected_item].events;

    if (events.empty())
    {
        return;
    }

    // Desenha a seta direita
    DELETE_OBJ(m_right_arrow);
    draw_arrow_right(LOGO_ARROW_RIGTH_SEL_20x20);
    init_arrow_timer();
    // Evento da linha selecionada
    DEBUG_MSG(OSD, DEBUG, "Event at channel " << m_all_channel_info[m_selected_item].name << ", m_current_time: " << m_current_time.time_to_str() << "\n");

    for (const auto &event : events)
    {
        print_event_info(event, __LINE__);

        if (event.start.to_unix_epoch() > m_current_time.to_unix_epoch())
        {
            // Atualiza o tempo atual para o início do próximo evento
            m_current_time = event.start;
            DEBUG_MSG(OSD, DEBUG, "new m_current_time: " << m_current_time.time_to_str() << "\n");
            draw_timedate_box();
            draw_event_list();
            break;
        }
    }
}

void OSD_Guide_Channel::move_left()
{
    // Verifica se há eventos
    DEBUG_MSG(OSD, DEBUG, "Selected event: " << m_selected_item << "\n");
    auto events = m_all_channel_info[m_selected_item].events;

    if (events.empty())
    {
        return;
    }

    // Desenha a seta esquerda
    DELETE_OBJ(m_left_arrow);
    draw_arrow_left(LOGO_ARROW_LEFT_SEL_20x20);
    init_arrow_timer();
    DEBUG_MSG(OSD, DEBUG, "Event at channel " << m_all_channel_info[m_selected_item].name << ", m_current_time: " << m_current_time.time_to_str() << "\n");

    for (int i = events.size() - 1; i >= 0; i--)
    {
        const auto &event = events[i];
        DEBUG_MSG(OSD, DEBUG, " - " << event.program << " at " << event.start.hour() << ":" << (event.start.minute() < 10 ? "0" : "") << event.start.minute() << "\n");

        if (event.start.to_unix_epoch() < m_current_time.to_unix_epoch())
        {
            // Atualiza o tempo atual para o início do evento anterior
            m_current_time = std::max(event.start, m_start_time); // Garante que não vá para antes do início
            draw_timedate_box();
            draw_event_list();
            break;
        }
    }
}

void OSD_Guide_Channel::draw_arrow_right(const char *text)
{
    DELETE_OBJ(m_right_arrow);
    m_right_arrow = load_image(m_tm_dt_box, text, 0, 0, 20, 20);
    lv_obj_align(m_right_arrow, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_null_on_delete(&m_right_arrow);
}

void OSD_Guide_Channel::draw_arrow_left(const char *text)
{
    DELETE_OBJ(m_left_arrow);
    m_left_arrow = load_image(m_tm_dt_box, text, 0, 0, 20, 20);
    lv_obj_align(m_left_arrow, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_null_on_delete(&m_left_arrow);
}

void OSD_Guide_Channel::init_arrow_timer()
{
    if (!m_timer_arrow)
    {
        m_timer_arrow = lv_timer_create([](lv_timer_t *timer)
        {
            auto thiz = static_cast<OSD_Guide_Channel *>(lv_timer_get_user_data(timer));
            thiz->draw_arrow_left(LOGO_ARROW_LEFT_20x20);
            thiz->draw_arrow_right(LOGO_ARROW_RIGTH_20x20);
            thiz->m_timer_arrow = nullptr;
        }, 250, this);

        lv_timer_set_repeat_count(m_timer_arrow, 1);
        lv_timer_set_auto_delete(m_timer_arrow, true);
    }
    else
    {
        lv_timer_reset(m_timer_arrow);
    }
}

/*
** Carrega EPG e imprime eventos de todos os canais
*/
void OSD_Guide_Channel::load_all_epg()
{
    // Limpa lista de canais
    m_all_channel_info.clear();
    m_selected_item = 0;
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto current_service = current_lineup->get_current_service();

    if (not current_lineup->services.empty())
    {
        for (const auto &service : current_lineup->services)
        {
            if (m_channel_list_type == OSD_Channels_List_Type::TV_Channels and current_lineup->is_radio_service(service.service_type()))
            {
                continue; // Pula serviços de rádio
            }
            else if (m_channel_list_type == OSD_Channels_List_Type::Radio_Channels and current_lineup->is_tv_service(service.service_type()))
            {
                continue;   // Pula serviços de TV
            }

            auto events = Task_EIT_Events::get_all_events_for_service(&service);
            std::vector<Event_Info_t>evs = {};

            if (not events.empty())
            {
                for (const auto &event : events)
                {
                    Event_Info_t e;
                    e.program = event.short_event_descriptor;
                    e.start = event.start_time; //.to_local_time();
                    e.duration = std::chrono::duration_cast<std::chrono::minutes>(event.duration);

                    if (is_valid_event(e))
                    {
                        evs.push_back(e);
                    }
                }
            }
            else
            {
                // Se não houver eventos, cria eventos padrão
                auto system_time = System::get_system_time();//.to_local_time();
                auto displacement = std::chrono::minutes(60); // 1 hora de deslocamento

                for (int i = 0 ; i < EVENT_COLUMNS; ++i)
                {
                    Event_Info_t e = {std::string(tr(__Sem_informacoes)), system_time, displacement};
                    system_time = system_time + displacement; // Incrementa 1 hora para o próximo evento
                    evs.push_back(e);
                }
            }

            // Atualiza vetor de canais
            m_all_channel_info.push_back({service.name(), service.viewer_channel(), service.service_id(), evs});

            // Verifica se o canal atual é o mesmo do serviço
            if (current_service->viewer_channel() == service.viewer_channel() && m_init_selected_item)
            {
                m_selected_item = m_all_channel_info.size() - 1;
                m_init_selected_item = false; // Apenas inicializa uma vez
            }
        }
    }

    // Determine first, last and selected items
    if (!m_all_channel_info.empty())
    {
        m_first_visible_item = m_last_visible_item = m_selected_item;
        m_last_visible_item = m_selected_item + MAX_CHANNEL_LIST_VIEW - 1;

        if (m_last_visible_item >= static_cast<int>(m_all_channel_info.size()))
        {
            m_last_visible_item = m_all_channel_info.size() - 1;
        }

        m_first_visible_item = m_last_visible_item - MAX_CHANNEL_LIST_VIEW + 1;

        if (m_first_visible_item < 0)
        {
            m_first_visible_item = 0;
        }
    }
}

void OSD_Guide_Channel::draw_channel_list()
{
    // Verifica se há canais para exibir
    if (m_all_channel_info.empty())
    {
        DEBUG_MSG(OSD, WARN, "No channels to display\n");
        return;
    }

    for (auto box : m_channel_boxes)
    {
        DELETE_OBJ(box);
    }

    m_channel_boxes.clear();
    // draw channel boxes
    auto position = 0;

    for (auto it = m_first_visible_item ; it <= m_last_visible_item; ++it)
    {
        //DEBUG_MSG("Drawing channel box for item " << it << " at position " << position << "\n");
        auto y = m_channel_box_y + position * m_channel_box_s;
        auto ch = m_all_channel_info[it];
        auto background = m_selected_item == it ? OSD_COLOR_ORANGE : OSD_COLOR_BLACK;
        create_rect(m_box_guide, m_channel_box_x, y - 2, m_channel_box_w, 2, OSD_COLOR_GREY_DARK);
        auto box = create_rect(m_box_guide, m_channel_box_x, y, m_channel_box_w, m_channel_box_h, background);
        lv_obj_set_style_radius(box, 10, 0);
        char channel_info[100] = {};
        snprintf(channel_info, sizeof(channel_info), "%.4d - %s", ch.viewer_channel, ch.name.data());
        auto label = set_label_text(box, channel_info, 0, 0, font_semi_20, OSD_COLOR_WHITE);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(label, 200);
        lv_obj_set_height(label, 25);
        // Salvar caixa de canal
        m_channel_boxes.push_back(box);
        position++;
    }
}

void OSD_Guide_Channel::draw_event_list()
{
    // Verifica se há canais para exibir
    if (m_all_channel_info.empty())
    {
        DEBUG_MSG(OSD, WARN, "No channels to display\n");
        return;
    }

    // Limpa lista de eventos
    for (size_t i = 0 ; i < m_event_boxes.size(); i++)
    {
        auto &box = m_event_boxes[i];

        if (box)
        {
            lv_obj_del(box);
        }
    }

    m_event_boxes.fill(nullptr);
    // draw event boxes from first to last visible item
    DEBUG_MSG(OSD, DEBUG, "Selected item: " << m_selected_item << ", First visible item: " << m_first_visible_item << ", Last visible item: " << m_last_visible_item << "\n");
    DEBUG_MSG(OSD, DEBUG, "m_current_time: " << m_current_time.time_to_str() << "\n");
    std::vector<Channel_Info_t> channels = {};

    for (int i = m_first_visible_item; i <= m_last_visible_item; ++i)
    {
        channels.push_back(m_all_channel_info[i]);
    }

    auto selected_line = m_selected_item - m_first_visible_item;
    auto line_counter = 0;

    for (const auto &channel : channels)
    {
        //DEBUG_MSG(OSD, DEBUG, "Channel: " << channel.name << ", Viewer Channel: " << channel.viewer_channel << "\n");
        auto column_counter = 0;

        for (auto event : channel.events)
        {
            // Calcula nova duração para eventos que começam antes da hora atual
            //print_event_info(event,__LINE__);
            // Verifica se o evento termina antes da hora atual
            if (!is_valid_event(event))
            {
                continue;
            }

            auto box = draw_event_box(event, line_counter);

            if (!box)
            {
                break; // Sai se a caixa não foi criada (fora da tela)
            }

            if (column_counter == 0 && line_counter == selected_line)
            {
                // Destaca evento do canal selecionado
                //print_event_info(event, __LINE__);
                //DEBUG_MSG("\tHighlighting event box for selected channel\n");
                lv_obj_set_style_border_color(box, OSD_COLOR_ORANGE, 0);
                lv_obj_set_style_border_width(box, 3, 0);
                // Inicializa eventos
                m_schedule_entry.id = 0; // ID do agendamento, pode ser definido posteriormente
                m_schedule_entry.service_id = channel.service_id;
                m_schedule_entry.time_to_start = std::chrono::system_clock::from_time_t(event.start.to_unix_epoch());
                m_schedule_entry.time_to_end = std::chrono::system_clock::from_time_t(event.start.to_unix_epoch() + (event.duration.count() * 60));
                m_schedule_entry.operation = Schedule_Operation::RECORD;
                m_schedule_entry.repeat = Schedule_Repeat::ONCE;
                m_schedule_entry.status = Schedule_Status::ACTIVE;
            }

            auto pos = line_counter * EVENT_COLUMNS + column_counter;
            m_event_boxes[pos] = box;

            if (++column_counter >= EVENT_COLUMNS)
            {
                break;
            }
        }

        ++line_counter;
    }
}

lv_obj_t *OSD_Guide_Channel::draw_event_box(Event_Info_t event, int line)
{
    using namespace std::chrono;
    // Calculate offset from event.start and m_current_time in minutes
    auto offset = (event.start.to_unix_epoch() - m_current_time.to_unix_epoch()) / 60;
    //DEBUG_MSG("Event offset: " << offset << "\n");
    offset = std::max(offset, 0l); // Garantir que o offset não seja negativo
    int x = minute_to_x(offset) + m_channel_box_x + m_channel_box_w;
    int w = minute_to_x(event.duration.count()) - 1; // Subtrai 1 para dar espaço entre as caixas
    w = std::max(w, 10);                        // Garantir que a largura não seja negativa
    w = std::min(w, DISPLAY_WIDTH - x);         // Garantir que não ultrapasse a largura da tela
    auto y = m_channel_box_y + line * m_channel_box_s + 2; // Adiciona 2 para alinhar com a caixa do canal;
    // Cria caixa de evento
    lv_obj_t *box = nullptr;

    if (x < DISPLAY_WIDTH)
    {
        // DEBUG_MSG(OSD, DEBUG, event.program << ": " << event.start.time_to_str()
        //           << ", Duration: " << event.duration.count() << " minutes"
        //           << " at X: " << x << ", end X: " << (x + w) << "\n");

        box = create_rect(m_box_guide, x, y, w - 2, m_channel_box_h - 1, OSD_COLOR_GREY_DARK);
        lv_obj_set_style_radius(box, 10, 0);
        // Adiciona label com o nome do programa
        auto label = set_label_text(box, event.program.data(), 10, 7, font_20, OSD_COLOR_WHITE);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(label, w);
        lv_obj_set_height(label, 25);
    }

    return box;
}

void OSD_Guide_Channel::move_menu_up()
{
    DEBUG_MSG(OSD, DEBUG, "m_selected_item: " << m_selected_item << "\n");

    lv_timer_pause(m_tmr_channel_preview);
    if (m_selected_item > 0)
    {
        m_selected_item--;
        DEBUG_MSG(OSD, DEBUG, "m_selected_item: " << m_selected_item << "\n");

        if (m_first_visible_item > 0 and m_selected_item < m_first_visible_item)
        {
            m_first_visible_item--;
            m_last_visible_item--;
            DEBUG_MSG(OSD, DEBUG, "m_first_visible_item: " << m_first_visible_item << "\n");
            DEBUG_MSG(OSD, DEBUG, "m_last_visible_item: " << m_last_visible_item << "\n");
        }
    }

    draw_channel_list();
    draw_event_list();
    lv_timer_reset(m_tmr_channel_preview);
    lv_timer_resume(m_tmr_channel_preview);
}

void OSD_Guide_Channel::move_menu_down()
{
    DEBUG_MSG(OSD, DEBUG, "m_selected_item: " << m_selected_item << "\n");

    lv_timer_pause(m_tmr_channel_preview);
    if (m_selected_item < static_cast<int>(m_all_channel_info.size()) - 1)
    {
        m_selected_item++;
        DEBUG_MSG(OSD, DEBUG, "m_selected_item: " << m_selected_item << "\n");

        if (m_last_visible_item < static_cast<int>(m_all_channel_info.size()) - 1 and m_selected_item > m_last_visible_item)
        {
            m_first_visible_item++;
            m_last_visible_item++;
            DEBUG_MSG(OSD, DEBUG, "m_first_visible_item: " << m_first_visible_item << "\n");
            DEBUG_MSG(OSD, DEBUG, "m_last_visible_item: " << m_last_visible_item << "\n");
        }
    }

    draw_channel_list();
    draw_event_list();
    lv_timer_reset(m_tmr_channel_preview);
    lv_timer_resume(m_tmr_channel_preview);
}

void  OSD_Guide_Channel::schedule_event()
{
    DEBUG_MSG(OSD, DEBUG, "Scheduling event for channel: " << m_channel_text_info.channel_name << "\n");
    DEBUG_MSG(OSD, DEBUG, "Hora atual: " << m_start_time.time_to_str() << "\n");

    // Se o horário de início é a hora atual, adicionar 2 minutos
    if (m_schedule_entry.time_to_start == std::chrono::system_clock::from_time_t(m_start_time.to_unix_epoch()))
    {
        m_schedule_entry.time_to_start += std::chrono::minutes(m_minimum_recording_time);
    }

#ifndef NDEBUG
    {
        time_t tm = std::chrono::system_clock::to_time_t(m_schedule_entry.time_to_start);
        DEBUG_MSG(OSD, DEBUG, "Início do agendamento: " << std::ctime(&tm));
        tm = std::chrono::system_clock::to_time_t(m_schedule_entry.time_to_end);
        DEBUG_MSG(OSD, DEBUG, "Fim do agendamento: " << std::ctime(&tm));
    }

#endif

    // Se o horário de término é menor que o horário de início, exibir erro
    if (m_schedule_entry.time_to_end <= m_schedule_entry.time_to_start)
    {
        DEBUG_MSG(OSD, WARN, "Error: End time is less than or equal to start time\n");
        DELETE_OBJ(m_error_message);
        m_error_message = create_rect(m_bgd, 0, offset_y, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 6, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(m_error_message, 20, 0);
        lv_obj_center(m_error_message);
        lv_obj_fade_out(m_error_message, 8000, 0);
        std::string message = std::string(tr(__Erro_tempo_de_gravacao_inferior_a)) + " " + std::to_string(m_minimum_recording_time);
        message += tr(__Minutos);
        auto error_message_label = set_label_text(m_error_message, message.c_str(), 0, 0, font_20, OSD_COLOR_RED);
        lv_obj_align(error_message_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(error_message_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(error_message_label, OSD_COLOR_RED, 0);
        lv_obj_set_style_text_font(error_message_label, font_semi_30, 0);
    }
    else
    {
        m_osd_scheduled_edit = std::make_unique<OSD_Scheduled_Edit>(this);
        m_osd_scheduled_edit->show_scheduled_edit(std::bind(&OSD_Guide_Channel::schedule_event_callback, this), m_schedule_entry);
    }
}

void  OSD_Guide_Channel::schedule_event_callback()
{
    m_osd_scheduled_edit.reset();
}

bool OSD_Guide_Channel::is_valid_event(Event_Info_t &event)
{
    using namespace std::chrono;

    if (event.program.empty())
    {
        DEBUG_MSG(OSD, WARN, "Invalid event: program is empty\n");
        return false;
    }

    if (event.duration.count() < 3)
    {
        DEBUG_MSG(OSD, WARN, "Invalid event, duration less than 3 minutes:\n");
        print_event_info(event, __LINE__);
        return false;
    }

    auto event_end_time = event.start + event.duration;

    if (event_end_time < m_current_time)
    {
        DEBUG_MSG(OSD, DEBUG, "Invalid event, end time is before current time:\n");
        DEBUG_MSG(OSD, DEBUG, "m_current_time: " << m_current_time.time_to_str() << "\n");
        DEBUG_MSG(OSD, DEBUG, "Invalid event: event end time " << event_end_time.time_to_str() << " is before current time\n");
        DEBUG_MSG(OSD, DEBUG, "Invalid event: event has already ended\n");
        return false;
    }

    if (event.start < m_current_time)
    {
        auto ev = event;
        ev.start = m_current_time;
        ev.duration = duration_cast<minutes>(seconds(event_end_time.to_unix_epoch() - ev.start.to_unix_epoch()));

        if (ev.duration.count() < 3)
        {
            //DEBUG_MSG(OSD, DEBUG, "Invalid event: ");
            //print_event_info(ev, __LINE__);
            return false;
        }

        event = ev;
        //print_event_info(event, __LINE__);
    }

    return true;
}

} // namespace mb
