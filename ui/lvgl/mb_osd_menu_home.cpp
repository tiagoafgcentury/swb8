#include "mb_osd_menu_home.h"
#include "mb_osd_fade_canvas.h"
#include "mb_menu_resources.h"
#include "mb_menu_data.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_application.h"
#include "tasks/mb_task_eit_events.h"
#include "mb_events.h"

#include <lvgl.h>
#include <math.h>
#include <map>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <stdio.h>
#include <string.h>


namespace mb {

OSD_Menu_Home::OSD_Menu_Home(OSD *_parent):
    OSD(_parent)
{
    DEBUG_MSG(OSD, DEBUG, "CLASS: " << __FUNCTION__ << "\n");
    lv_style_init(&m_menu_style);
    lv_style_init(&m_menu_sel_style);
    lv_style_init(&m_box_style);
    lv_style_init(&m_class_style);
}

OSD_Menu_Home::~OSD_Menu_Home()
{
    for (int i = 0; i < m_menu_home.number_of_categories_founds; i++)
    {
        m_menu_home.sub_menu_data[i].clear();
        m_menu_home.sub_menu_count[i] = 0;
        m_menu_home.category_title_name[i].clear();
    }

    m_menu_num_categories = 0;
    m_menu_title_idx = 0;
    m_menu_home.sub_menu_view.clear();
    m_bgd_video.reset();
    DELETE_OBJ(m_main_menu);
    DELETE_TIMER(m_tmr_channel_preview);
    lv_style_reset(&m_menu_style);
    lv_style_reset(&m_menu_sel_style);
    lv_style_reset(&m_box_style);
    lv_style_reset(&m_class_style);
}

std::string_view OSD_Menu_Home::get_category_title(uint8_t _index)
{
    switch (_index)
    {
        case 0:
            return tr(__Favoritos);

        case 1:
            return tr(__Regionais);

        case 2:
            return tr(__Entretenimento);

        case 3:
            return tr(__Noticias);

        case 4:
            return tr(__Educacao);

        case 5:
            return tr(__Religioso);

        case 6:
            return tr(__Agronegocio);

        case 7:
            return tr(__TV_Publica);

        case 8:
            return tr(__Compras);

        case 9:
            return tr(__Outros_canais_TV);

        case 10:
            return tr(__Todos_canais_TV);

        case 11:
            return tr(__Todas_Radios);
    }

    return "Century";
}

void OSD_Menu_Home::set_menu_selection(Sub_Menu_Data sub_menu, Sub_Menu_View &sub_menu_view, bool _selected, bool _hidden)
{
    lv_obj_align(m_menu_title, LV_ALIGN_DEFAULT, 20, 0);
    lv_obj_remove_flag(m_menu_title, LV_OBJ_FLAG_HIDDEN);

    if (m_menu_title_previous != nullptr)
    {
        lv_obj_align(m_menu_title_previous, LV_ALIGN_DEFAULT, 20, 10);
    }

    if (m_menu_title_next != nullptr)
    {
        //lv_obj_align(m_menu_title_next, LV_ALIGN_DEFAULT, -10, MENU_SPACING_SUBTITLE);
        lv_obj_align(m_menu_title_next, LV_ALIGN_DEFAULT, 20, MENU_SPACING_SUBTITLE);
        lv_obj_remove_flag(m_menu_title_next, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_remove_style(sub_menu_view.rect, _selected ? &m_menu_sel_style : &m_menu_style, 0);
    lv_obj_add_style(sub_menu_view.rect, _selected ? &m_menu_sel_style : &m_menu_style, 0);
    lv_label_set_text(sub_menu_view.channel_number, sub_menu.channel_number.data());
    lv_obj_set_style_text_color(sub_menu_view.channel_number, _selected ? OSD_COLOR_BLACK : OSD_COLOR_WHITE, DEFAULT_SELECTOR);
    lv_label_set_text(sub_menu_view.channel_name, sub_menu.channel_name.data());
    lv_obj_set_style_text_font(sub_menu_view.channel_name, _selected ? font_semi_25 : font_25, 0);
    lv_obj_set_style_text_color(sub_menu_view.channel_name, _selected ? OSD_COLOR_WHITE : OSD_COLOR_GREY_MEDIUM, DEFAULT_SELECTOR);

    if (_hidden)
    {
        lv_obj_add_flag(sub_menu_view.box, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(sub_menu_view.box, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Menu_Home::set_menu_selection(uint16_t _menu, uint16_t _menu_ver, uint16_t _menu_hor, bool _selected)
{
    auto menu_size = m_menu_home.sub_menu_data[_menu_ver].size();

    if (_menu < menu_size)
    {
        for (auto idx = 0; idx < m_menu_home.number_of_categories_founds; idx++)
        {
            auto &title = m_menu_home.menu_title[idx];

            if (title)
            {
                lv_obj_add_flag(title, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (_menu_ver != m_menu_current_ver_pos)
        {
            if (_menu_ver == 0 && m_menu_home.number_of_categories_founds == 1)
            {
                m_menu_title_previous = nullptr;
                m_menu_title = m_menu_home.menu_title[_menu_ver];
                m_menu_title_next = nullptr;
            }
            else if (_menu_ver == 0 && m_menu_home.number_of_categories_founds > 1)
            {
                m_menu_title_previous = nullptr;
                m_menu_title = m_menu_home.menu_title[_menu_ver];
                m_menu_title_next = m_menu_home.menu_title[_menu_ver + 1];
            }
            else if (_menu_ver == m_menu_home.number_of_categories_founds - 1)
            {
                m_menu_title_previous = m_menu_home.menu_title[_menu_ver - 1];
                m_menu_title = m_menu_home.menu_title[_menu_ver];
                m_menu_title_next = nullptr;
            }
            else
            {
                m_menu_title_previous = m_menu_home.menu_title[_menu_ver - 1];
                m_menu_title = m_menu_home.menu_title[_menu_ver];
                m_menu_title_next = m_menu_home.menu_title[_menu_ver + 1];
            }

            m_menu_current_ver_pos = _menu_ver;
        }

        //Verifica se necessita esconder última lista de canais
        if (m_menu_current_ver_pos >= (m_menu_home.number_of_categories_founds - 1))
        {
            for (auto j = 4; j < std::min<int>(m_menu_home.sub_menu_view.size(), MAX_SUBMENU_VIEW); j++)
            {
                lv_obj_add_flag(m_menu_home.sub_menu_view[j].box, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else
        {
            for (auto j = 4; j < std::min<int>(m_menu_home.sub_menu_view.size(), MAX_SUBMENU_VIEW); j++)
            {
                lv_obj_remove_flag(m_menu_home.sub_menu_view[j].box, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (_menu_hor < menu_size)
        {
            auto sub_menu = m_menu_home.sub_menu_data[_menu_ver][_menu_hor];
            auto &sub_menu_view = m_menu_home.sub_menu_view[0];
            set_menu_selection(sub_menu, sub_menu_view, _selected, 0);
            auto max_hor_size = (menu_size - (_menu_hor + 1));

            if (max_hor_size == 2)
            {
                for (auto z = 1u; z <= max_hor_size; z++)
                {
                    auto sub_menu = m_menu_home.sub_menu_data[_menu_ver][_menu_hor + z];
                    auto &sub_menu_view = m_menu_home.sub_menu_view[z];
                    set_menu_selection(sub_menu, sub_menu_view, false, 0);
                }

                if (m_menu_home.number_of_services >= MAX_SUBMENU_VIEW_LINE)
                {
                    lv_obj_add_flag(m_menu_home.sub_menu_view[3].box, LV_OBJ_FLAG_HIDDEN);
                }
            }
            else if (max_hor_size == 1)
            {
                auto sub_menu = m_menu_home.sub_menu_data[_menu_ver][_menu_hor + 1];
                auto &sub_menu_view = m_menu_home.sub_menu_view[1];
                set_menu_selection(sub_menu, sub_menu_view, false, 0);
                auto size = m_menu_home.number_of_services >= MAX_SUBMENU_VIEW_LINE ? MAX_SUBMENU_VIEW_LINE : menu_size;

                for (auto z = 2u; z < size; z++)
                {
                    lv_obj_add_flag(m_menu_home.sub_menu_view[z].box, LV_OBJ_FLAG_HIDDEN);
                }
            }
            else if (max_hor_size == 0)
            {
                auto size = m_menu_home.number_of_services >= MAX_SUBMENU_VIEW_LINE ? MAX_SUBMENU_VIEW_LINE : menu_size;

                for (auto z = 1u; z < size; z++)
                {
                    lv_obj_add_flag(m_menu_home.sub_menu_view[z].box, LV_OBJ_FLAG_HIDDEN);
                }
            }
            else
            {
                for (auto z = 1u; z < MAX_SUBMENU_VIEW_LINE; z++)
                {
                    auto sub_menu = m_menu_home.sub_menu_data[_menu_ver][_menu_hor + z];
                    auto &sub_menu_view = m_menu_home.sub_menu_view[z];
                    set_menu_selection(sub_menu, sub_menu_view, false, 0);
                }
            }
        }
    }
}

void OSD_Menu_Home::move_selection(uint16_t _ver, uint16_t _hor)
{
    lv_timer_pause(m_tmr_channel_preview);
    set_menu_selection(m_selected_item, _ver, _hor, true);
    lv_timer_reset(m_tmr_channel_preview);
    lv_timer_resume(m_tmr_channel_preview);
}

void OSD_Menu_Home::create_menu(int _cat_idx, int _sub_menu_index)
{
    //Create regular style
    lv_style_set_radius(&m_menu_style, 6);
    //lv_style_set_bg_opa(&subMenuStyle, LV_OPA_COVER);
    lv_style_set_bg_color(&m_menu_style, OSD_COLOR_GREY);
    //Create selected style
    lv_style_set_bg_color(&m_menu_sel_style, OSD_COLOR_WHITE);
    lv_style_set_outline_width(&m_menu_sel_style, 3);
    lv_style_set_outline_color(&m_menu_sel_style, OSD_COLOR_ORANGE_DARK);
    lv_style_set_outline_pad(&m_menu_sel_style, 4);
    lv_style_set_bg_color(&m_menu_sel_style, OSD_COLOR_WHITE);
    //Create a box
    auto box = create_rect(m_bgd_categories, 0, 0, 305, 210, OSD_COLOR_BLACK);
    lv_obj_align(box, LV_ALIGN_DEFAULT, 0, 0);
    //Create a grey rectangle
    auto rect = lv_obj_create(box);
    lv_obj_set_scrollbar_mode(rect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
    lv_obj_remove_style(rect, &m_menu_style, 0);
    lv_obj_add_style(rect, &m_menu_style, 0);
    lv_obj_set_size(rect, 285, 160);
    lv_obj_set_style_radius(rect, 8, DEFAULT_SELECTOR);
    lv_obj_align(rect, LV_ALIGN_TOP_LEFT, 10, 10);
    const auto &channel = m_menu_home.sub_menu_data[_cat_idx][_sub_menu_index];
    auto channel_number = not channel.channel_number.empty() ? channel.channel_number.data() : "00";
    auto lbl_channel_number = set_label_text_static(rect, channel_number, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(lbl_channel_number, LV_ALIGN_CENTER, 0, 0);
    auto channel_name = not channel.channel_name.empty() ? channel.channel_name.data() : "";
    auto lbl_channel_name = set_label_text_static(box, channel_name, 0, 0, font_25, OSD_COLOR_GREY);
    lv_label_set_long_mode(lbl_channel_name, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl_channel_name, 305);
    lv_obj_set_height(lbl_channel_name, 30);
    lv_obj_align(lbl_channel_name, LV_ALIGN_DEFAULT, 5, 180);
    m_menu_home.emplace_view_back(box, rect, lbl_channel_name, lbl_channel_number);
}

void OSD_Menu_Home::add_menu_title(std::string_view _category_name, std::string_view _logo)
{
    auto pos = 0;
    auto y = (MENU_SPACING * pos);
    m_menu_home.menu_title[m_menu_title_idx] = create_rect(m_bgd_categories, 40, y, 305, 35, OSD_COLOR_BLACK);
    lv_obj_align(m_menu_home.menu_title[m_menu_title_idx], LV_ALIGN_DEFAULT, 40, 20);

    if (not _logo.empty())
    {
        load_image(m_menu_home.menu_title[m_menu_title_idx], _logo, 0, 0, 27, 25);
    }

    set_label_text_static(m_menu_home.menu_title[m_menu_title_idx], _category_name, (_logo.empty() ? 0 : 35), -5, font_semi_30, OSD_COLOR_WHITE);

    if (m_menu_title_idx == 1)
    {
        lv_obj_align(m_menu_home.menu_title[m_menu_title_idx], LV_ALIGN_DEFAULT, 20, MENU_SPACING_SUBTITLE);
    }
    else if (m_menu_title_idx > 1)
    {
        lv_obj_add_flag(m_menu_home.menu_title[m_menu_title_idx], LV_OBJ_FLAG_HIDDEN);
    }

    m_menu_title_idx += 1;
}

int OSD_Menu_Home::add_menu_option(std::string_view _category_name, std::string_view _logo)
{
    add_menu_title(_category_name, _logo);
    auto pos = 0;
    auto current_lineup_size = get_current_lineup_size();
    auto number_of_channels = current_lineup_size >= MAX_CHANNEL_INFO_VIEW ? MAX_CHANNEL_INFO_VIEW : current_lineup_size;

    if (m_menu_num_categories == 0) //Desenha a primeira categoria
    {
        for (size_t idx = 0; idx < number_of_channels; idx++)
        {
            create_menu(m_menu_num_categories, idx);
            m_menu_home.sub_menu_view[idx].x = (20 + (idx * MENU_TEXT_SPACING));
            lv_obj_set_pos(m_menu_home.sub_menu_view[idx].box, m_menu_home.sub_menu_view[idx].x, 35);
        }
    }
    else if (m_menu_num_categories == 1) //Desenha a segunda categoria
    {
        for (size_t idx = 0; idx < number_of_channels and idx + MAX_CHANNEL_INFO_VIEW < number_of_channels; idx++)
        {
            create_menu(m_menu_num_categories, idx + MAX_CHANNEL_INFO_VIEW);
            m_menu_home.sub_menu_view[idx + MAX_CHANNEL_INFO_VIEW].x = (20 + (idx * MENU_TEXT_SPACING));
            lv_obj_set_pos(m_menu_home.sub_menu_view[idx + MAX_CHANNEL_INFO_VIEW].box, m_menu_home.sub_menu_view[idx + MAX_CHANNEL_INFO_VIEW].x, 310);
        }
    }

    m_menu_num_categories += 1;
    m_menu_current_ver_pos = m_menu_num_categories;
    return pos;
}

void OSD_Menu_Home::print_channel_info()
{
    int16_t width = 0;
    lv_point_t pos;

    if (!m_channel_name && !m_channel_desc && !m_program && !m_subtitle)
    {
        //Nome do Canal
        m_channel_name = set_label_text_static(m_bgd_sub_menu, "", 20, 32, font_bold_40, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_channel_name, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_channel_name, 480);
        lv_obj_set_height(m_channel_name, 45);
        m_channel_desc = set_label_text_static(m_bgd_sub_menu, "", 20, 87, font_20, OSD_COLOR_GREY);
        m_program = set_label_text_static(m_bgd_sub_menu, "", 20, 120, font_semi_30, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_program, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_program, MENU_MAX_TEXT_WIDTH);
        m_subtitle = set_label_text_static(m_bgd_sub_menu, "", 20, 0, font_20, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_subtitle, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_subtitle, MENU_MAX_TEXT_WIDTH);
    }

    //Nome do Canal
    lv_label_set_text(m_channel_name, m_channel_text_info.channel_name.data());
    //Descricao do Canal
    std::string desc = m_channel_text_info.channel_number + " | " + m_channel_text_info.channel_category;

    //Canal favorito
    if (m_channel_text_info.favorite)
    {
        if (m_img_fav)
        {
            lv_obj_delete(m_img_fav);
            m_img_fav = nullptr;
        }

        m_img_fav = load_image(m_bgd_sub_menu, LOGO_FAVORITOS_CINZA_17x15, 20, 80, 17, 15);
        width = 40;
        lv_obj_set_pos(m_channel_desc, width, 75);
        desc = " | " + m_channel_text_info.channel_number + " | " + m_channel_text_info.channel_category;
    }

    lv_label_set_text(m_channel_desc, desc.data());

    if (m_class_indic)
    {
        lv_obj_del(m_class_indic);
        m_class_indic = nullptr;
    }

    std::string text = "     " + m_channel_text_info.channel_program;
    Parental_Control curr_pr = static_cast<Parental_Control>(m_channel_text_info.parental_rating);
    m_class_indic = classificacao_indicativa(m_bgd_sub_menu, curr_pr, MENU_PARENTAL_RATING_X, MENU_PARENTAL_RATING_Y, 0);
    lv_obj_set_size(m_class_indic, 27, 28);

    if (text.length() >= 45)
    {
        text = text.substr(0, 72) + " ...";
        lv_obj_set_height(m_program, 86);
    }
    else
    {
        lv_obj_set_height(m_program, 43);
    }

    //Programa Atual
    lv_label_set_text(m_program, text.data());
    lv_obj_align(m_program, LV_ALIGN_DEFAULT,  20, 120);
    lv_obj_update_layout(m_program);
    lv_text_get_size(&pos, text.data(), font_semi_30, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    auto labelLines = lv_obj_get_height(m_program) / pos.y;
    auto posY = 143 + (labelLines * 28);
    std::string str_sub_title = m_channel_text_info.program_description;
    std::string tmp = m_channel_text_info.program_description;

    if (posY == 171 && str_sub_title.length() >= 380)
    {
        str_sub_title = tmp.substr(0, 380) + " ...";
    }
    else if (posY >= 199)
    {
        str_sub_title = tmp.substr(0, 285) + " ...";
    }

    //lv_textarea_set_max_length(m_subtitle, 200);
    lv_label_set_text(m_subtitle, str_sub_title.data());
    lv_obj_align(m_subtitle, LV_ALIGN_DEFAULT,  20, posY);
    lv_obj_set_height(m_subtitle, 130);
}

void OSD_Menu_Home::update_channel_info(bool save_channel_info, Service *_srv)
{
    if (_srv)
    {
        m_channel_text_info.channel_name = _srv->name();
        char channel_number[20] = {0};
        int ch = _srv->viewer_channel();
        int w = (ch >= 10000) ? 5 : 4;

        snprintf(channel_number, sizeof(channel_number), "%s %0*d", tr(__Canal).data(), w, ch);
        m_channel_text_info.channel_number = channel_number;
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
        m_channel_text_info.parental_rating = static_cast<OSD::Parental_Control>(current_event->parental_rating);
        m_channel_text_info.program_description = current_event->extended_event_descriptor;
    }
    else
    {
        m_channel_text_info.channel_program =  tr(__Sem_informacoes);
        m_channel_text_info.parental_rating = OSD::Parental_Control::CLASSIFICACAO_INDEFINIDA;
        m_channel_text_info.program_description = tr(__Sem_informacoes);
    }

    m_transponder_id = _srv->transponder_id();
    m_service_id = _srv->service_id();

    if (save_channel_info)
    {
        m_atual_srv = _srv;
    }

    print_channel_info();
}

static void channel_preview_cb(lv_timer_t *timer)
{
    OSD_Menu_Home *thiz = static_cast<OSD_Menu_Home *>(lv_timer_get_user_data(timer));
    thiz->change_channel(false);
    lv_timer_pause(timer);
}

void OSD_Menu_Home::change_channel(bool save_channel_info)
{
    if (m_selected_item_ver < m_menu_home.sub_menu_data.size() and
            m_selected_item_hor < m_menu_home.sub_menu_data[m_selected_item_ver].size())
    {
        auto channelData =  m_menu_home.sub_menu_data[m_selected_item_ver][m_selected_item_hor];

        if (channelData.channel_number.size() > 0)
        {
            Viewer_Channel_t _viewer_channel = std::stoi(channelData.channel_number);

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
}

void OSD_Menu_Home::show_menu_home(Menu_Home_CB_t _callback, lv_obj_t *m_bgd)
{
    mb_assert(m_bgd);
    set_focus();
    m_callback = _callback;

    if (!m_main_menu)
    {
        //MainMenu
        m_main_menu = create_rect(m_bgd, 166, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_main_menu);
        lv_obj_set_style_bg_opa(m_main_menu, LV_OPA_TRANSP, 0);
        create_rect(m_main_menu, 0, 0, 480, DISPLAY_HEIGHT / 2, OSD_COLOR_BLACK);

        if (!m_bgd_video)
        {
            m_bgd_video = Fade_Canvas::make_video_mask(m_main_menu);
            lv_obj_align(m_bgd_video->canvas, LV_ALIGN_DEFAULT, 480, -20);
        }

        //m_bgd_sub_menu = create_rect(m_main_menu, 0, 0, DISPLAY_WIDTH - 148, DISPLAY_HEIGHT / 2, OSD_COLOR_BLUE);
        m_bgd_sub_menu = create_rect(m_main_menu, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT / 2, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_sub_menu);
        lv_obj_set_style_bg_opa(m_bgd_sub_menu, LV_OPA_TRANSP, 0);
        add_clock(m_bgd_sub_menu, 924, 28);
        m_bgd_categories = create_rect(m_main_menu, 0, 360, 1120, 380, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_categories);
        //create_line(m_main_menu, 148, 0, OSD_COLOR_GREY_DARK);
        m_atual_srv = Task::s_task_player->current_srv();

        if (m_atual_srv)
        {
            m_service_id = m_atual_srv->service_id();
            m_transponder_id = m_atual_srv->transponder_id();
        }

        char channel_number[10];
        m_selected_item_ver = m_selected_item_hor = 0;
        int posV = 0;
        m_menu_home.sub_menu_count.fill(0);
        m_menu_home.number_of_services = 0;

        for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
        {
            int ch = srv.viewer_channel();
            int w = (ch >= 10000) ? 5 : 4;

            snprintf(channel_number, sizeof(channel_number), "%0*d", w, ch);
            std::string str(channel_number);
            m_menu_home.number_of_services += 1;

            if (srv.is_favorite())
            {
                m_menu_home.sub_menu_data[m_menu_home.FAVORITOS].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                m_menu_home.sub_menu_count[m_menu_home.FAVORITOS] += 1;
                m_menu_home.category_title_name[m_menu_home.FAVORITOS] = get_category_title(m_menu_home.FAVORITOS);
            }

            Basic_Service_Type srvType = to_basic_type(srv.service_type());

            if (srvType == Basic_Service_Type::TV)
            {
                m_menu_home.sub_menu_data[m_menu_home.TODOS_CANAIS_TV].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                m_menu_home.sub_menu_count[m_menu_home.TODOS_CANAIS_TV] += 1;
                m_menu_home.category_title_name[m_menu_home.TODOS_CANAIS_TV] = get_category_title(m_menu_home.TODOS_CANAIS_TV);
                posV = m_menu_home.TODOS_CANAIS_TV;
            }

            switch (srv.category())
            {
                case Service_Category::Regionais:
                    m_menu_home.sub_menu_data[m_menu_home.REGIONAIS].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.REGIONAIS] += 1;
                    m_menu_home.category_title_name[m_menu_home.REGIONAIS] = get_category_title(m_menu_home.REGIONAIS);
                    posV = m_menu_home.REGIONAIS;
                    break;

                case Service_Category::Entretenimento:
                    m_menu_home.sub_menu_data[m_menu_home.ENTRETENIMENTO].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.ENTRETENIMENTO] += 1;
                    m_menu_home.category_title_name[m_menu_home.ENTRETENIMENTO] = get_category_title(m_menu_home.ENTRETENIMENTO);
                    posV = m_menu_home.ENTRETENIMENTO;
                    break;

                case Service_Category::Noticias:
                    m_menu_home.sub_menu_data[m_menu_home.NOTICIAS].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.NOTICIAS] += 1;
                    m_menu_home.category_title_name[m_menu_home.NOTICIAS] = get_category_title(m_menu_home.NOTICIAS);
                    posV = m_menu_home.NOTICIAS;
                    break;

                case Service_Category::Educacao:
                    m_menu_home.sub_menu_data[m_menu_home.EDUCACAO].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.EDUCACAO] += 1;
                    m_menu_home.category_title_name[m_menu_home.EDUCACAO] = get_category_title(m_menu_home.EDUCACAO);
                    posV = m_menu_home.EDUCACAO;
                    break;

                case Service_Category::Religioso:
                    m_menu_home.sub_menu_data[m_menu_home.RELIGIOSO].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.RELIGIOSO] += 1;
                    m_menu_home.category_title_name[m_menu_home.RELIGIOSO] = get_category_title(m_menu_home.RELIGIOSO);
                    posV = m_menu_home.RELIGIOSO;
                    break;

                case Service_Category::Agronegocio:
                    m_menu_home.sub_menu_data[m_menu_home.AGRONEGOCIO].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.AGRONEGOCIO] += 1;
                    m_menu_home.category_title_name[m_menu_home.AGRONEGOCIO] = get_category_title(m_menu_home.AGRONEGOCIO);
                    posV = m_menu_home.AGRONEGOCIO;
                    break;

                case Service_Category::TV_Publica:
                    m_menu_home.sub_menu_data[m_menu_home.TV_PUBLICA].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.TV_PUBLICA] += 1;
                    m_menu_home.category_title_name[m_menu_home.TV_PUBLICA] = get_category_title(m_menu_home.TV_PUBLICA);
                    posV = m_menu_home.TV_PUBLICA;
                    break;

                case Service_Category::Compras:
                    m_menu_home.sub_menu_data[m_menu_home.COMPRAS].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.COMPRAS] += 1;
                    m_menu_home.category_title_name[m_menu_home.COMPRAS] = get_category_title(m_menu_home.COMPRAS);
                    posV = m_menu_home.COMPRAS;
                    break;

                case Service_Category::Novos_Canais:
                    m_menu_home.sub_menu_data[m_menu_home.OUTROS_CANAIS_TV].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.OUTROS_CANAIS_TV] += 1;
                    m_menu_home.category_title_name[m_menu_home.OUTROS_CANAIS_TV] = get_category_title(m_menu_home.OUTROS_CANAIS_TV);
                    posV = m_menu_home.OUTROS_CANAIS_TV;
                    break;

                case Service_Category::Radios:
                    m_menu_home.sub_menu_data[m_menu_home.TODAS_AS_RADIOS].emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
                    m_menu_home.sub_menu_count[m_menu_home.TODAS_AS_RADIOS] += 1;
                    m_menu_home.category_title_name[m_menu_home.TODAS_AS_RADIOS] = get_category_title(m_menu_home.TODAS_AS_RADIOS);
                    posV = m_menu_home.TODAS_AS_RADIOS;
                    break;

                case Service_Category::Undefined:
                    //    m_menu_home.subMenuData[m_menu_home.RELIGIOSO].emplace_back(srv.name().data(), str, srv.is_favorite);
                    break;
            }

            if (m_atual_srv and m_atual_srv->viewer_channel() == srv.viewer_channel())
            {
                m_selected_item_ver = posV;
                m_selected_item_hor = m_menu_home.sub_menu_count[posV] - 1;
            }
        }

        //Organiza a lista de categorias do menu home
        for (size_t i = 0; i < m_menu_home.MAX_CATEGORIES; ++i)
        {
            if (m_menu_home.sub_menu_count[i] == 0)
            {
                for (size_t j = i + 1; j < m_menu_home.MAX_CATEGORIES; ++j)
                {
                    if (m_menu_home.sub_menu_count[j] > 0)
                    {
                        m_menu_home.sub_menu_count[i] = m_menu_home.sub_menu_count[j];
                        m_menu_home.category_title_name[i] = m_menu_home.category_title_name[j];

                        for (auto &data : m_menu_home.sub_menu_data[j])
                        {
                            m_menu_home.sub_menu_data[i].push_back(std::move(data));
                        }

                        m_menu_home.sub_menu_count[j] = 0;
                        m_menu_home.sub_menu_data[j].clear();
                        m_menu_home.category_title_name[j].clear();
                        break;
                    }
                }
            }
        }

        //Cria a lista de categorias do menu home
        m_menu_home.number_of_categories_founds = 0;

        for (size_t i = 0; i < m_menu_home.MAX_CATEGORIES; ++i)
        {
            if (m_menu_home.sub_menu_count[i] > 0)
            {
                m_menu_home.number_of_categories_founds += 1;

                if (i == m_menu_home.FAVORITOS)
                {
                    add_menu_option(m_menu_home.category_title_name[i], LOGO_FAVORITOS_BRANCO_27x25);
                }
                else
                {
                    add_menu_option(m_menu_home.category_title_name[i], {});
                }
            }
        }
    } //m_main_menu

    if (m_menu_home.sub_menu_data[m_selected_item_ver].size() == 0)
    {
        m_selected_item_ver = 0;
        m_selected_item_hor = 0;
    }

    set_menu_selection(m_selected_item, m_selected_item_ver, m_selected_item_hor, true);
    m_tmr_channel_preview = lv_timer_create(channel_preview_cb, 3000, this);
    lv_timer_pause(m_tmr_channel_preview);

    if (m_atual_srv)
    {
        update_channel_info(true, m_atual_srv);
    }

    DEBUG_MSG(OSD, DEBUG, "SHOW MENU HOME");
}

void OSD_Menu_Home::move_menu_left()
{
    auto size =  m_menu_home.sub_menu_data[m_selected_item_ver].size();

    if (m_selected_item_hor > size || m_selected_item_hor == 0)
    {
        m_selected_item_hor = 0;
    }
    else
    {
        m_selected_item_hor--;
    }

    move_selection(m_selected_item_ver, m_selected_item_hor);
}

void OSD_Menu_Home::move_menu_right()
{
    auto size = m_menu_home.sub_menu_data[m_selected_item_ver].size();

    if (++m_selected_item_hor >= size)
    {
        m_selected_item_hor = size - 1;
    }

    move_selection(m_selected_item_ver, m_selected_item_hor);
}

void OSD_Menu_Home::move_menu_up()
{
    if (m_selected_item_ver > 0)
    {
        m_selected_item_ver--;
        m_selected_item_hor = 0;
    }
    else
    {
        m_selected_item_ver = 0;
    }

    move_selection(m_selected_item_ver, m_selected_item_hor);
}

void OSD_Menu_Home::move_menu_down()
{
    if (++m_selected_item_ver < m_menu_num_categories)
    {
        m_selected_item_hor = 0;
    }
    else
    {
        m_selected_item_ver = m_menu_num_categories - 1;
    }

    move_selection(m_selected_item_ver, m_selected_item_hor);
}

void OSD_Menu_Home::return_main_menu()
{
    lv_timer_delete(m_tmr_channel_preview);
    m_tmr_channel_preview = nullptr;

    if (m_atual_srv && m_atual_srv->transponder_id() != m_transponder_id && m_atual_srv->service_id() != m_service_id)
    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

        for (auto i = 0u; i < current_lineup->services.size(); i++)
        {
            m_last_srv = &current_lineup->services[i];

            if (m_last_srv->service_id() == m_atual_srv->service_id() &&
                    m_last_srv->transponder_id() == m_atual_srv->transponder_id())
            {
                Task::s_task_application->post_event_channel_change(POST_CALLER m_last_srv);
                break;
            }
        }

        m_atual_srv = Task::s_task_player->current_srv();
    }

    remove_focus();
    Task::post_event(std::bind(m_callback, false));
}

bool OSD_Menu_Home::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_MENU:
            remove_focus();
            Task::post_event(std::bind(m_callback, true));
            return true;

        case Remote_Control_Key::KEY_VOLTAR:
            return_main_menu();
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
            move_menu_right();
            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            if (m_selected_item_hor == 0)
            {
                return_main_menu();
            }
            else
            {
                move_menu_left();
            }

            return true;

        default:
            return true;
    }

    return false;
}

void OSD_Menu_Home::got_focus()
{
}

} // namespace mb
