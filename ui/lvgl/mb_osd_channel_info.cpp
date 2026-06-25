#include "mb_osd_channel_info.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include "hal/mb_system.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "tasks/mb_task_eit_events.h"

#include <lvgl.h>
#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <time.h>

namespace {

constexpr auto SLIDER_MAX_VALUE = 1000;

}

namespace mb {

std::unique_ptr<Channel_Info> Channel_Info::s_instance { nullptr };

Channel_Info *Channel_Info::get_instance(OSD *_parent)
{
    if (not s_instance)
    {
        s_instance.reset(new Channel_Info(_parent));
    }

    return s_instance.get();
}

Channel_Info::Channel_Info(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_style_main);
    lv_style_init(&m_style_indic_pr);
    lv_style_init(&m_style_indicator);
    lv_style_init(&m_style_main);
    lv_style_init(&m_style_top);
    lv_style_init(&m_label_style);
    lv_style_set_bg_opa(&m_style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style_main, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&m_style_main, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&m_style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style_indicator, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&m_style_indicator, &m_transition_dsc);
    // Monta o frame das barras superior e inferior
    //channel_info_frame();
}

// Destrutor
Channel_Info::~Channel_Info()
{
    lv_style_reset(&m_style_main);
    lv_style_reset(&m_style_indic_pr);
    lv_style_reset(&m_style_indicator);
    lv_style_reset(&m_style_main);
    lv_style_reset(&m_style_top);
    lv_style_reset(&m_label_style);
}

// Callback para apagar informações do canal
void Channel_Info::delete_channel_info_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    Channel_Info *thiz = static_cast<Channel_Info *>(lv_timer_get_user_data(tm));
    thiz->delete_channel_info();
}

// Apaga informações do canal
void Channel_Info::delete_channel_info()
{
    // Apaga relógio
    remove_clock();
    DELETE_TIMER(m_hide_timer);
    DELETE_OBJ(m_ch_name);
    DELETE_OBJ(m_ch_info);
    DELETE_OBJ(m_ch_date);
    DELETE_OBJ(m_ch_hour);
    DELETE_OBJ(m_ch_current);
    DELETE_OBJ(m_ch_current_time);
    DELETE_OBJ(m_ch_next_time);
    DELETE_OBJ(m_ch_next);
    DELETE_OBJ(m_bottom_mask);
    DELETE_OBJ(m_slider_obj);
    DELETE_OBJ(m_fav_top);
    DELETE_OBJ(m_current_footer);
    DELETE_OBJ(m_next_footer);
    DELETE_OBJ(m_main_screen);
    s_instance.reset();
}

// Inicia objetos no construtor
void Channel_Info::channel_info_frame()
{
    if (m_main_screen == nullptr)
    {
        // Cria área da tela
        m_main_screen = lv_canvas_create(get_main_screen(OSD_Layer::MAIN_INFO));
        lv_obj_null_on_delete(&m_main_screen);
        // Cria áreas de transparência superior e inferior
        std::tie(m_top_rect, m_bottom_rect) = Fade_Canvas::make_info_mask(m_main_screen);
        // Desenha área superior da tela de informações do canal
        lv_obj_set_pos(m_top_rect->canvas, 0, TOPAREA_Y1 + (DISPLAY_HEIGHT / 8));
        auto top = create_rect(m_main_screen, AREA_X, TOPAREA_Y1, AREA_WIDTH, AREA_HEIGHT / 2, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(top, 202, LV_PART_MAIN);
        auto top_mask = create_rect(m_main_screen, AREA_X, TOPAREA_Y1, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_GREEN);
        lv_obj_set_style_bg_opa(top_mask, 0, LV_PART_MAIN);
        // Desenhar retângulo na base da tela
        lv_obj_set_pos(m_bottom_rect->canvas, 0, BOTTOMAREA_Y1);
        auto bottom = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 + AREA_HEIGHT / 2, AREA_WIDTH, AREA_HEIGHT / 2, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(bottom, 202, LV_PART_MAIN);
        m_bottom_mask = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_GREEN);
        lv_obj_null_on_delete(&m_bottom_mask);
        lv_obj_set_style_bg_opa(m_bottom_mask, 0, LV_PART_MAIN);
        // Área de nome e número do canal
        m_ch_name = set_label_text_static(top_mask, "", START_POS_X, 30, font_semi_40, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_ch_name);
        m_ch_info = set_label_text_static(top_mask, "", START_POS_X, 78, font_20, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_ch_info);
        // Adiciona relógio
        add_clock(top_mask, 1090, 28);
        // Programa atual
        m_ch_current = set_label_text_static(m_bottom_mask, "", START_POS_X, 60, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_ch_current);
        //m_ch_current.label_set_long_mode(LV_LABEL_LONG_DOT);
        lv_anim_init(&m_animation_template);
        lv_anim_set_delay(&m_animation_template, 1000);
        lv_anim_set_repeat_delay(&m_animation_template, 3000);
        //lv_style_init(label_style);
        lv_style_set_anim(&m_label_style, &m_animation_template);
        lv_label_set_long_mode(m_ch_current, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_add_style(m_ch_current, &m_label_style, LV_STATE_DEFAULT);
        m_ch_current_time = set_label_text_static(m_bottom_mask, "", START_POS_X, 60, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_ch_current_time);
        // Próximo programa
        m_ch_next = set_label_text_static(m_bottom_mask, "", START_POS_X, 110, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_ch_next);
        m_ch_next_time = set_label_text_static(m_bottom_mask, "", START_POS_X, 110, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_ch_next_time);
        lv_label_set_long_mode(m_ch_next, LV_LABEL_LONG_DOT);
        // Exibe imagens na barra inferior
        static constexpr auto base_line = 110;
        static constexpr auto x_start = 693;
        load_image(m_bottom_mask, LOGO_MIDIABOX_BRANCO_208x30, x_start + 290, 60, 208, 40);
        load_image(m_bottom_mask, LOGO_INFO_34x34, x_start + 331, base_line, 40, 40);
        load_image(m_bottom_mask, LOGO_AUDIO_LR_34x34, x_start + 375, base_line, 40, 40);
        load_image(m_bottom_mask, LOGO_CC_34x34, x_start + 419, base_line, 40, 40);
        load_image(m_bottom_mask, LOGO_MAIS_34x34, x_start + 463, base_line, 40, 40);
        // Estrela de favoritos da barra superior
        m_fav_top = load_image(top_mask, LOGO_FAVORITOS_BRANCO_17x15, 0, 0, 17, 15);
        lv_obj_null_on_delete(&m_fav_top);
        lv_obj_add_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
        // Cria barra de progresso
        show_horizontal_bar();
    }
}

void copy_event_info(Channel_Info::Event_Info_t &_info, std::optional<Task_EIT_Events::EIT_Event> &_event)
{
    if (_event)
    {
        _info.program = _event->short_event_descriptor;
        auto current_start_time = _event->start_time;
        _info.start_str = current_start_time.to_local_time().time_to_str();
        _info.start = current_start_time.to_unix_epoch();
        auto current_end_time = current_start_time + _event->duration;
        _info.end_str = current_end_time.to_local_time().time_to_str();
        _info.duration = _event->duration.count();
        _info.parental_rating = static_cast<OSD::Parental_Control>(_event->parental_rating);
    }
    else
    {
        _info.program = Channel_Info::NULL_CHANNEL;
        _info.start_str = Channel_Info::NULL_TIME;
        _info.end_str = Channel_Info::NULL_TIME;
        _info.parental_rating = OSD::Parental_Control::CLASSIFICACAO_INDEFINIDA;
    }
}

// Exibe os dados do novo canal sintonizado
void Channel_Info::show_channel_info(const Service *_srv)
{
    channel_info_frame();

    if (!_srv)
    {
        // Informações do canal sintonizado não foram encontradas
        DEBUG_MSG(OSD, ERROR, "Show channel info: <null>\n");
        return;
    }

    DEBUG_MSG(OSD, DEBUG, "Show channel info: " << _srv->viewer_channel() << " " << _srv->name() << "\n");
    // Busca infiormações do programa atual
    auto [current_event, next_event] = Task_EIT_Events::get_event_for_service(_srv);
    Event_Info_t current_program;
    Event_Info_t next_program;
    copy_event_info(current_program, current_event);
    copy_event_info(next_program, next_event);
    //Nome do canal
    char channel_number[20] = {0};
    lv_label_set_text(m_ch_name, _srv->name().data());
    int ch = _srv->viewer_channel();
    int width = (ch >= 10000) ? 5 : 4;

    snprintf(channel_number, sizeof(channel_number), "%s %0*d", tr(__Canal).data(), width, ch);

    // Exibe ou esconde estrela de favorito
    if (_srv->is_favorite())
    {
        std::string strCanal = "      " + std::string(channel_number);
        lv_label_set_text(m_ch_info, strCanal.c_str());
        lv_obj_set_pos(m_fav_top, START_POS_X, 82);
        // Exibe estrela
        lv_obj_remove_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        std::string strCanal = std::string(channel_number);
        lv_label_set_text(m_ch_info, strCanal.c_str());
        // Esconde estrela
        lv_obj_add_flag(m_fav_top, LV_OBJ_FLAG_HIDDEN);
    }

    // Desenha rodapé com informações do programa atual e do próximo
    draw_footer(current_program, next_program);
    // Busca hora atual do sistema convertendo para decimal
    auto system_time = System::get_system_time().to_unix_epoch();
    // Cálculo de percentual passado
    double progress;

    if (current_program.duration > 0)
    {
        double elapsed = system_time - current_program.start;
        progress = (elapsed / current_program.duration) * static_cast<double>(SLIDER_MAX_VALUE);
    }
    else
    {
        progress = 0;
    }

    // Exibe prograsso do programa
    update_horizontal_bar(std::round(progress));
    start_hide_timer();
}

void Channel_Info::hide_channel_info()
{
    if (s_instance)
    {
        s_instance->delete_channel_info();
    }
}

void Channel_Info::reset_hide_timer()
{
    if (s_instance)
    {
        s_instance->start_hide_timer();
    }
}

bool Channel_Info::has_menuInfo()
{
    return s_instance.get() != nullptr;
}

// Desenha rodapé
void Channel_Info::draw_footer(const Event_Info_t &_current_program, const Event_Info_t &_next_program)
{
    std::string str;
    Parental_Control parental_rating;
    str = _current_program.start_str;
    str += " - ";
    str += _current_program.end_str;
    str += " | ";
    lv_label_set_text(m_ch_current_time, str.c_str());
    DELETE_OBJ(m_current_footer);

    if (_current_program.program != NULL_CHANNEL)
    {
        parental_rating = _current_program.parental_rating;
        m_current_footer = classificacao_indicativa(m_bottom_mask, parental_rating, START_POS_X + 170, 60, 0);
        lv_obj_null_on_delete(&m_current_footer);
        lv_obj_set_pos(m_ch_current, START_POS_X + 210, 60);
    }
    else
    {
        lv_obj_set_pos(m_ch_current, START_POS_X + 140, 60);
    }

    lv_label_set_text(m_ch_current, _current_program.program.c_str());
    lv_obj_set_width(m_ch_current, 670);
    str = _next_program.start_str;
    str += " - ";
    str += _next_program.end_str;
    str += " | ";
    lv_label_set_text(m_ch_next_time, str.c_str());
    DELETE_OBJ(m_next_footer);

    if (_next_program.program != NULL_CHANNEL)
    {
        parental_rating = _next_program.parental_rating;
        m_next_footer = classificacao_indicativa(m_bottom_mask, parental_rating, START_POS_X + 170, 110, 0);
        lv_obj_null_on_delete(&m_next_footer);
        lv_obj_set_pos(m_ch_next, START_POS_X + 210, 110);
    }
    else
    {
        lv_obj_set_pos(m_ch_next, START_POS_X + 140, 110);
    }

    lv_label_set_text(m_ch_next, _next_program.program.c_str());
    lv_obj_set_width(m_ch_next, 850);
    lv_obj_set_height(m_ch_next, 50);
}

// Inicia timer para apagar informações do canal
void Channel_Info::start_hide_timer()
{
    // Inicia timer para apagar informações do canal
    if (m_hide_timer == nullptr)
    {
        m_hide_timer = lv_timer_create(delete_channel_info_cb, 6000, this);
        lv_timer_set_repeat_count(m_hide_timer, 1);
    }
    else
    {
        lv_timer_reset(m_hide_timer);
    }
}

// Exibe progresso do programa
void Channel_Info::update_horizontal_bar(int progress)
{
    // Atualiza barra de progresso
    lv_slider_set_value(m_slider_obj, progress, LV_ANIM_OFF);
}

// Exibe barra de progresso
void Channel_Info::show_horizontal_bar()
{
    if (m_slider_obj)
    {
        return;
    }

    /*Create an object with the new style_pr*/
    m_slider_obj = lv_bar_create(m_bottom_mask);
    lv_obj_null_on_delete(&m_slider_obj);
    // Posiciona e define o tamanho da barra
    lv_obj_align(m_slider_obj, LV_ALIGN_DEFAULT, START_POS_X, 100);
    lv_obj_set_width(m_slider_obj, 1100);
    lv_obj_set_height(m_slider_obj, 5);
    lv_obj_add_style(m_slider_obj, &m_style_main, LV_PART_MAIN);
    lv_obj_add_style(m_slider_obj, &m_style_indicator, LV_PART_INDICATOR);
    lv_slider_set_range(m_slider_obj, 0, SLIDER_MAX_VALUE);
}

} // namespace mb
