#include "mb_osd_edit_channel.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_data.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task_application.h"
#include "mb_events.h"

#include <lvgl.h>

namespace mb {

OSD_Edit_Channel::OSD_Edit_Channel(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_front_style);
    lv_style_set_bg_opa(&m_front_style, LV_OPA_COVER);
    lv_style_set_bg_color(&m_front_style, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_front_style, LV_RADIUS_CIRCLE);
}

OSD_Edit_Channel::~OSD_Edit_Channel()
{
    lv_style_reset(&m_front_style);
    m_channel_data.clear();
    m_menu_data.clear();
    m_sub_menu_data.clear();
    DELETE_TIMER(m_tmr_channel_preview);
    DELETE_OBJ(m_bgd);
    remove_focus();
}

void OSD_Edit_Channel::draw_radio_box()
{

    auto _front_box = create_rect(m_radio_preview, 0, 0, 300, radio_height-16, OSD_COLOR_GREY_DARK);
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

void OSD_Edit_Channel::change_channel()
{
    Transponder_Id tp_id = m_channel_data[m_sub_selected_item].transponder_id;
    Service_ID_t service_id = m_channel_data[m_sub_selected_item].service_id;
    uint16_t i = 0;

    for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
    {
        if (srv.service_id() == service_id and srv.transponder_id() == tp_id)
        {
            Task::post_event_channel_change(POST_CALLER & srv);
            DEBUG_MSG(LINEUP, ERROR, "Changing channel to: " << dec << srv.viewer_channel() << " - " << srv.name() << "\n");
            m_current_channel = i;
            break;
        }

        i += 1;
    }
}

void OSD_Edit_Channel::set_favorite_channel(Menu_Data &ch_info, bool _selected)
{
    if (_selected)
    {
        lv_obj_remove_flag(ch_info.img, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ch_info.img, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Edit_Channel::reset_menu_selection()
{
    m_selected_item = 0;

    for (auto &menu : m_menu_data)
    {
        if (menu.sel)
        {
            lv_obj_set_style_text_font(menu.lbl, font_25, 0);
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
            lv_obj_set_style_bg_color(menu.lbl, OSD_COLOR_BLACK, 0);
        }
    }
}

void OSD_Edit_Channel::reset_sub_menu_selection()
{
    remove_menu_fade();

    for (auto &sub_menu : m_sub_menu_data)
    {
        if (sub_menu.sel)
        {
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Edit_Channel::display_channel_list(uint16_t start_idx, uint16_t selected_item)
{
    uint16_t size = 0;

    if (m_channel_data.size() >= start_idx)
    {
        size = std::min<uint16_t>(MAX_CHANNEL_LIST_VIEW, m_channel_data.size() - start_idx);
    }
    else
    {
        m_is_sub_menu = false;
        reset_sub_menu_selection();
    }

    for (uint16_t idx = start_idx; idx < start_idx + size; idx++)
    {
        auto &sub_menu = m_sub_menu_data[(idx - start_idx)];
        auto str = std::to_string(m_channel_data[idx].viewer_channel);
        str += " - ";
        str += m_channel_data[idx].channel_name;

        if (is_move_channel_pos_enabled)
        {
            auto mv_box = lv_obj_get_child(m_sub_menu_data[(idx - start_idx)].sel, 0);
            lv_obj_add_flag(mv_box, LV_OBJ_FLAG_HIDDEN);

            if (m_channel_data[idx].viewer_channel == m_channel_data[m_sub_selected_item].viewer_channel)
            {
                lv_obj_remove_flag(mv_box, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else
        {
            auto mv_box = lv_obj_get_child(m_sub_menu_data[(idx - start_idx)].sel, 0);
            lv_obj_add_flag(mv_box, LV_OBJ_FLAG_HIDDEN);
        }

        /*Mostra se o canal é favorito*/
        if (m_channel_data[idx].favorite)
        {
            lv_obj_remove_flag(sub_menu.img, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(sub_menu.img, LV_OBJ_FLAG_HIDDEN);
        }

        lv_label_set_text(sub_menu.lbl, str.c_str());

        if (idx == selected_item)
        {
            lv_obj_set_style_bg_color(sub_menu.sel, OSD_COLOR_ORANGE, 0);
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_COVER, 0);
        }
        else
        {
            lv_obj_set_style_bg_color(sub_menu.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_TRANSP, 0);
        }
    }

    if (size < MAX_CHANNEL_LIST_VIEW)
    {
        for (uint16_t idx = size; idx < MAX_CHANNEL_LIST_VIEW; idx++)
        {
            auto &sub_menu = m_sub_menu_data[idx];
            lv_obj_add_flag(sub_menu.img, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(sub_menu.lbl, "");
            lv_obj_set_style_bg_color(sub_menu.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Edit_Channel::set_sub_menu_selection(uint16_t _sub_menu_to)
{
    if (_sub_menu_to < m_start_pos)
    {
        m_start_pos = _sub_menu_to;
    }
    else if (_sub_menu_to >= (m_start_pos + MAX_CHANNEL_LIST_VIEW))
    {
        m_start_pos = _sub_menu_to - MAX_CHANNEL_LIST_VIEW + 1;
    }

    display_channel_list(m_start_pos, _sub_menu_to);
}

void OSD_Edit_Channel::move_sub_selection(uint16_t /*_from*/, uint16_t _to)
{
    lv_timer_pause(m_tmr_channel_preview);
    set_sub_menu_selection(_to);
    update_channel_info();
    lv_timer_reset(m_tmr_channel_preview);
    lv_timer_resume(m_tmr_channel_preview);
}

void OSD_Edit_Channel::add_menu_fade()
{
    for (auto idx = 0u; idx < m_menu_data.size(); idx++)
    {
        auto &menu = m_menu_data[idx];

        if (idx == m_selected_item)
        {
            Osd_Breadcrumb::s_instance.add_name(lv_label_get_text(menu.lbl));
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_GREY, 0);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_ORANGE_DARK, 0);
        }
        else
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_GREY, 0);
        }
    }

    if (m_current_channel_list == 0)
    {
        lv_obj_remove_flag(m_instructions, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(m_instructions_1, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Edit_Channel::remove_menu_fade()
{
    Osd_Breadcrumb::s_instance.remove_name();

    for (auto idx = 0u; idx < m_menu_data.size(); idx++)
    {
        auto &menu = m_menu_data[idx];

        if (idx == m_selected_item)
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_WHITE, 0);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_ORANGE, 0);
        }
        else
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_WHITE, 0);
        }
    }

    if (m_current_channel_list == 0)
    {
        lv_obj_add_flag(m_instructions, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(m_instructions_1, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Edit_Channel::set_menu_selection(uint8_t _menu, bool _selected)
{
    if (_menu < m_menu_data.size())
    {
        auto &menu = m_menu_data[_menu];

        if (_selected)
        {
            lv_obj_set_style_text_font(menu.lbl, font_semi_25, 0);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_ORANGE, 0);
            lv_obj_set_style_bg_opa(menu.sel, LV_OPA_100, 0);
        }
        else
        {
            lv_obj_set_style_text_font(menu.lbl, font_25, 0);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(menu.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Edit_Channel::move_selection(uint8_t _from, uint8_t _to)
{
    update_channel_view_list();
    set_menu_selection(_from, false);
    set_menu_selection(_to, true);
}

void OSD_Edit_Channel::add_sub_menu(std::string_view _icon_fav, std::string_view _icon_move_up, std::string_view _icon_move_down)
{
    uint16_t size = MAX_CHANNEL_LIST_VIEW;
    m_center_box = create_rect(m_main_menu, m_center_box_x, m_subMenu_top, m_center_box_w, m_center_box_h, OSD_COLOR_BLACK);
    m_line_left = create_rect(m_center_box, 0, 0, 3, m_center_box_h, OSD_COLOR_ORANGE);
    m_line_right = create_rect(m_center_box, m_center_box_w - 3, 0, 3, m_center_box_h, OSD_COLOR_GREY);

    for (size_t idx = 0; idx < size; idx++)
    {
        auto y = (MENU_SUB_MENU_SPACING * idx);
        auto sel = create_rect(m_center_box, 15, y, 460, 50, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(sel, 25, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_opa(sel, LV_OPA_TRANSP, 0);
        auto mv_box = create_rect(sel, 0, 0, 60, 48, OSD_COLOR_BLACK);
        lv_obj_align(mv_box, LV_ALIGN_RIGHT_MID, -20, 0);
        lv_obj_set_style_bg_opa(mv_box, LV_OPA_TRANSP, 0);
        auto img_move_up = load_image(mv_box, _icon_move_up, 0, 0, 27, 27);
        lv_obj_align(img_move_up, LV_ALIGN_LEFT_MID, 0, 0);
        auto img_move_down = load_image(mv_box, _icon_move_down, 0, 0, 27, 27);
        lv_obj_align(img_move_down, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_add_flag(mv_box, LV_OBJ_FLAG_HIDDEN);
        auto img_fav = load_image(sel, _icon_fav, 0, 0, 27, 25);
        lv_obj_align(img_fav, LV_ALIGN_LEFT_MID, 10, 0);
        auto lbl = set_label_text(sel, "", 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 50, 0);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
        lv_obj_set_width(lbl, 400);
        lv_obj_set_height(lbl, 35);
        //if (m_channel_data.size() > 0 and m_channel_data[idx].favorite == false)
        {
            lv_obj_add_flag(img_fav, LV_OBJ_FLAG_HIDDEN);
        }

        m_sub_menu_data.emplace_back(std::move(sel), std::move(img_fav), nullptr, std::move(lbl));
    }
}

int OSD_Edit_Channel::add_menu(std::string_view _title)
{
    auto pos = m_menu_data.size();
    auto y = m_mn_top + (MENU_SPACING * pos);
    lv_obj_t *img { nullptr };
    lv_obj_t *img_sel { nullptr };
    auto sel = create_rect(m_main_menu, 90, y, 260, 50, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(sel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(sel, 25, DEFAULT_SELECTOR);
    auto lbl = set_label_text_static(sel, _title, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
    m_menu_data.emplace_back(std::move(sel), std::move(img), std::move(img_sel), std::move(lbl));
    return pos;
}

void OSD_Edit_Channel::update_channel_info()
{
    char buffer[200];

    if (m_selected_item < MAX_MENU_ITEM and m_sub_selected_item < m_channel_data.size())
    {
        const auto &ch_info = m_channel_data[m_sub_selected_item];
        auto sat_name = Lineup_Mutex_Ref::get_current_lineup()->get_sattelite_name(ch_info.transponder_id.satellite_id());
        auto config = Config::get_config();
        auto frequency = ch_info.transponder_id.frequency() / 1000;
        auto polarity = OSD_Translate::translate(ch_info.transponder_id.polarity());
        auto tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(ch_info.transponder_id);
        auto sr = tp ? tp->symbol_rate : 0;
        auto band = OSD_Translate::translate(config->band());
        snprintf(buffer, sizeof(buffer) - 1, "%dMHz/%dKbps %s", frequency, sr, polarity.data());
        //lv_label_set_text(m_lbl_freq, buffer);
        snprintf(buffer, sizeof(buffer) - 1, "%s: %s %s", tr(__Satelite).data(), sat_name.c_str(), band.data());
        lv_label_set_text(m_lbl_channel_name, ch_info.channel_name.data());
        lv_label_set_text_fmt(m_lbl_satellite_v, "%s %s", sat_name.c_str(), band.data());
        lv_label_set_text_fmt(m_lbl_frequency_v, "%dMHz", frequency);
        lv_label_set_text_fmt(m_lbl_symbol_rate_v, "%dKbps", sr);
        lv_label_set_text(m_lbl_polarity_v, polarity.data());
        lv_label_set_text(m_lbl_scrambled, tr(__Aberto).data());
    }
    else
    {
        lv_label_set_text(m_lbl_channel_name, "");
        lv_label_set_text(m_lbl_satellite_v, "");
        lv_label_set_text(m_lbl_frequency_v, "");
        lv_label_set_text(m_lbl_symbol_rate_v, "");
        lv_label_set_text(m_lbl_polarity_v, "");
        lv_label_set_text(m_lbl_scrambled, tr(__Sem_informacoes).data());
    }
}

void OSD_Edit_Channel::set_current_channel_list_view()
{
    auto size = m_channel_data.size();
    m_sub_selected_item = 0;
    m_current_channel = 0;
    auto lineup = Lineup_Mutex_Ref::get_current_lineup();
    m_current_service = lineup->get_current_service();

    if (m_current_service)
    {
        for (size_t idx = 0; idx < size; idx++)
        {
            const auto &ch_data = m_channel_data[idx];

            if (m_current_service->service_id() == ch_data.service_id and m_current_service->transponder_id() == ch_data.transponder_id)
            {
                DEBUG_MSG(OSD, DEBUG, "m_current_service: " << m_current_service->service_id() << "\n");
                DEBUG_MSG(OSD, DEBUG, "srv: " << ch_data.service_id << "\n");
                m_sub_selected_item = idx;
                m_current_channel = idx;
                break;
            }
        }
    }
}

void OSD_Edit_Channel::update_channel_table_list()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

    if (m_current_channel_list == 0)
    {
        if (m_channel_list_type == OSD_Channels_List_Type::TV_Channels)
        {
            current_lineup->set_channel_list_type(Channel_List_Type::MY_TV_CHANNELS);
            m_channel_data = current_lineup->get_list(Channel_List_Type::MY_TV_CHANNELS);
        }
        else
        {
            current_lineup->set_channel_list_type(Channel_List_Type::MY_RADIO_CHANNELS);
            m_channel_data = current_lineup->get_list(Channel_List_Type::MY_RADIO_CHANNELS);
        }
    }
    else
    {
        if (m_channel_list_type == OSD_Channels_List_Type::TV_Channels)
        {
            current_lineup->set_channel_list_type(Channel_List_Type::ALL_TV_CHANNELS);
            m_channel_data = current_lineup->get_list(Channel_List_Type::ALL_TV_CHANNELS);
        }
        else
        {
            current_lineup->set_channel_list_type(Channel_List_Type::ALL_RADIO_CHANNELS);
            m_channel_data = current_lineup->get_list(Channel_List_Type::ALL_RADIO_CHANNELS);
        }
    }

    if (m_sub_selected_item >= m_channel_data.size())
    {
        m_sub_selected_item = m_channel_data.size() - 1;
    }
}

void OSD_Edit_Channel::enter_sub_menu()
{
    if (m_channel_data.size() > 0)
    {
        add_menu_fade();
        m_start_pos = m_sub_selected_item;
        move_sub_selection(0, m_sub_selected_item);
        m_is_sub_menu = true;
    }
}

static void channel_preview_cb(lv_timer_t *timer)
{
    auto thiz = static_cast<OSD_Edit_Channel *>(lv_timer_get_user_data(timer));
    thiz->change_channel();
    lv_timer_pause(timer);
}

void OSD_Edit_Channel::show_menu_edit_channel(OSD_Edit_Channel_CB_t _callback, OSD_Channels_List_Type _channel_list_type)
{
    set_focus();
    m_callback = _callback;
    m_channel_list_type = _channel_list_type;

    if (!m_main_menu)
    {
        //MainMenu
        m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd);
        lv_obj_set_style_bg_opa(m_bgd, LV_OPA_TRANSP, 0);
        m_main_menu = create_rect(m_bgd, 0, offset_y, width, height, OSD_COLOR_BLACK);
        m_box_info = create_rect(m_bgd, width, 300, 420, 430, OSD_COLOR_BLACK);
        create_rect(m_bgd, 1210, 120, 90, 200, OSD_COLOR_BLACK);
        auto bgd_bottom = create_rect(m_bgd, 0, 630, DISPLAY_WIDTH, 90, OSD_COLOR_BLACK);

        if (m_channel_list_type == OSD_Channels_List_Type::TV_Channels)
        {
            add_menu(tr(__Meus_canais_TV));
            add_menu(tr(__Todos_canais_TV));
        }
        else
        {
            add_menu(tr(__Minhas_radios));
            add_menu(tr(__Todas_Radios));
        }

        m_lbl_channel_name = set_label_text(m_box_info, "", m_box_info_x, 15, font_semi_25, OSD_COLOR_WHITE);
        lv_label_set_long_mode(m_lbl_channel_name, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_lbl_channel_name, 350);
        lv_obj_set_height(m_lbl_channel_name, 30);
        auto y = m_box_info_y;
        m_lbl_satellite = set_label_text(m_box_info, "", m_box_info_x, m_box_info_y, font_semi_20, OSD_COLOR_WHITE);
        lv_label_set_text_fmt(m_lbl_satellite, "%s: ", tr(__Satelite).data());
        y += m_box_info_spacing;
        m_lbl_frequency = set_label_text(m_box_info, "", m_box_info_x, y, font_semi_20, OSD_COLOR_WHITE);
        lv_label_set_text_fmt(m_lbl_frequency, "%s: ", tr(__Frequencia).data());
        y += m_box_info_spacing;
        m_lbl_symbol_rate = set_label_text(m_box_info, "S/R: ", m_box_info_x, y, font_semi_20, OSD_COLOR_WHITE);
        y += m_box_info_spacing;
        m_lbl_polarity = set_label_text_static(m_box_info, "", m_box_info_x, y, font_semi_20, OSD_COLOR_WHITE);
        lv_label_set_text_fmt(m_lbl_polarity, "%s: ", tr(__Polaridade).data());
        y += m_box_info_spacing;
        m_lbl_scrambled = set_label_text_static(m_box_info, "", m_box_info_x, y, font_semi_20, OSD_COLOR_WHITE);
        y += m_box_info_spacing;
        y = m_box_info_y;
        m_lbl_satellite_v = set_label_text(m_box_info, "", m_box_info_x + 80, m_box_info_y, font_20, OSD_COLOR_WHITE);
        y += m_box_info_spacing;
        m_lbl_frequency_v = set_label_text(m_box_info, "", m_box_info_x + 110, y, font_20, OSD_COLOR_WHITE);
        y += m_box_info_spacing;
        m_lbl_symbol_rate_v = set_label_text(m_box_info, "", m_box_info_x + 45, y, font_20, OSD_COLOR_WHITE);
        y += m_box_info_spacing;
        m_lbl_polarity_v = set_label_text_static(m_box_info, "", m_box_info_x + 110, y, font_20, OSD_COLOR_WHITE);
        y += m_box_info_spacing;
        y += m_box_info_spacing;
        // Cria rodapé
        m_instructions = MB_OSD_Footer::draw(bgd_bottom, tr(__Pressione_ok_para_selecionar_o_canal_mova_as_teclas), -40);
        lv_obj_null_on_delete(&m_instructions);
        m_instructions_1 = MB_OSD_Footer::draw(bgd_bottom, tr(__Pressione_mais_para_favoritar_desfavoritar), -40);
        lv_obj_null_on_delete(&m_instructions_1);
        lv_obj_add_flag(m_instructions_1, LV_OBJ_FLAG_HIDDEN);
        m_current_channel_list = 0;
        update_channel_table_list();
        set_current_channel_list_view();
        m_tmr_channel_preview = lv_timer_create(channel_preview_cb, 3000, this);
        lv_timer_pause(m_tmr_channel_preview);

        if (m_channel_list_type == OSD_Channels_List_Type::Radio_Channels)
        {
            m_radio_preview = create_rect(m_bgd, 890, offset_y, 317, radio_height, OSD_COLOR_BLACK);
            draw_radio_box();
        }

        if (m_current_service)
        {
            add_sub_menu(LOGO_FAVORITOS_BRANCO_17x15, LOGO_MOVER_CIMA_27x27, LOGO_MOVER_BAIXO_27x27);
            reset_menu_selection();
            move_selection(0, m_selected_item);
            enter_sub_menu();
            update_channel_info();
        }
    }
}

void OSD_Edit_Channel::update_channel_view_list()
{
    std::string str;
    auto size = std::min<uint16_t>(MAX_CHANNEL_LIST_VIEW, m_channel_data.size());

    for (size_t idx = 0; idx < size; idx++)
    {
        auto &sub_menu = m_sub_menu_data[idx];
        str = std::to_string(m_channel_data[idx].viewer_channel) + std::string(" - ") + std::string{m_channel_data[idx].channel_name};

        if (is_move_channel_pos_enabled)
        {
            auto mv_box = lv_obj_get_child(m_sub_menu_data[idx].sel, 0);
            lv_obj_add_flag(mv_box, LV_OBJ_FLAG_HIDDEN);

            if (m_channel_data[idx].viewer_channel == m_channel_data[m_sub_selected_item].viewer_channel)
            {
                lv_obj_remove_flag(mv_box, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else
        {
            auto mv_box = lv_obj_get_child(m_sub_menu_data[idx].sel, 0);
            lv_obj_add_flag(mv_box, LV_OBJ_FLAG_HIDDEN);
        }

        if (m_channel_data[idx].favorite)
        {
            lv_obj_remove_flag(sub_menu.img, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(sub_menu.img, LV_OBJ_FLAG_HIDDEN);
        }

        lv_label_set_text(sub_menu.lbl, str.c_str());
    }

    if (size < MAX_CHANNEL_LIST_VIEW)
    {
        const auto max_idx = std::min<size_t>(MAX_CHANNEL_LIST_VIEW, m_sub_menu_data.size());

        for (size_t idx = size; idx < max_idx; idx++)
        {
            auto &sub_menu = m_sub_menu_data[idx];
            str = "";
            lv_obj_add_flag(sub_menu.img, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(sub_menu.lbl, str.c_str());
            lv_obj_set_style_bg_color(sub_menu.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Edit_Channel::move_menu_up()
{
    if (m_is_sub_menu == false)
    {
        m_sub_selected_item = 0;
        auto currentPos = m_selected_item;
        auto count = MAX_MENU_ITEM;
        m_selected_item = (m_selected_item + count - 1) % count;
        m_current_channel_list = m_selected_item;
        update_channel_table_list();
        move_selection(currentPos, m_selected_item);
    }
    else
    {
        auto currentPos = m_sub_selected_item;
        auto count = m_channel_data.size();
        m_sub_selected_item = (m_sub_selected_item + count - 1) % count;
        move_sub_selection(currentPos, m_sub_selected_item);
    }
}

void OSD_Edit_Channel::move_menu_down()
{
    if (m_is_sub_menu == false)
    {
        m_sub_selected_item = 0;
        auto currentPos = m_selected_item;
        auto count = MAX_MENU_ITEM;
        m_selected_item = (m_selected_item + count + 1) % count;
        m_current_channel_list = m_selected_item;
        update_channel_table_list();
        move_selection(currentPos, m_selected_item);
    }
    else
    {
        auto current_pos = m_sub_selected_item;
        auto count = m_channel_data.size();
        m_sub_selected_item = (m_sub_selected_item + count + 1) % count;
        move_sub_selection(current_pos, m_sub_selected_item);
    }
}

void OSD_Edit_Channel::exit_sub_menu()
{
    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        Task::post_event_update_channel_list(current_lineup->get_channel_list_type(), m_channel_data);
    }

    m_is_sub_menu = false;
    m_start_pos = m_sub_selected_item;
    move_sub_selection(0, 0);
    reset_sub_menu_selection();
}

void OSD_Edit_Channel::swap_channel_up()
{
    if (m_sub_selected_item > 0)
    {
        swap_services(m_channel_data[m_sub_selected_item].viewer_channel, m_channel_data[m_sub_selected_item - 1].viewer_channel);
        std::swap(m_channel_data[m_sub_selected_item], m_channel_data[m_sub_selected_item - 1]);
        m_sub_selected_item--;
    }
}

void OSD_Edit_Channel::swap_channel_down()
{
    if (m_sub_selected_item < m_channel_data.size() - 1)
    {
        swap_services(m_channel_data[m_sub_selected_item].viewer_channel, m_channel_data[m_sub_selected_item + 1].viewer_channel);
        std::swap(m_channel_data[m_sub_selected_item], m_channel_data[m_sub_selected_item + 1]);
        m_sub_selected_item++;
    }
}

bool OSD_Edit_Channel::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            if (m_is_sub_menu == false)
            {
                Task::post_event_lineup_save();
                Task::post_event(std::bind(m_callback, false));
            }
            else
            {
                if (is_move_channel_pos_enabled == false)
                {
                    exit_sub_menu();
                }
            }

            return true;

        case Remote_Control_Key::KEY_OK:
            if (m_is_sub_menu && m_channel_data.size() > 0)
            {
                //Somente permite alteração na lista se for meus canais de tv ou minhas radios
                if (m_current_channel_list == 0)
                {
                    is_move_channel_pos_enabled = !is_move_channel_pos_enabled;
                    set_sub_menu_selection(m_sub_selected_item);
                }
            }
            else if (m_is_sub_menu == false)
            {
                enter_sub_menu();
                update_channel_info();
            }

            return true;

        case Remote_Control_Key::KEY_CHUP:
            if (is_move_channel_pos_enabled)
            {
                swap_channel_up();
                set_sub_menu_selection(m_sub_selected_item);
                //update_channel_view_list();
            }
            else
            {
                move_menu_up();
            }

            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            if (is_move_channel_pos_enabled)
            {
                swap_channel_down();
                set_sub_menu_selection(m_sub_selected_item);
                //update_channel_view_list();
            }
            else
            {
                move_menu_down();
            }

            return true;

        case Remote_Control_Key::KEY_VOLUP:
            if (m_is_sub_menu == false and m_channel_data.size() > 0)
            {
                enter_sub_menu();
                update_channel_info();
            }

            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            if (m_is_sub_menu == true && is_move_channel_pos_enabled == false)
            {
                exit_sub_menu();
            }

            return true;

        case Remote_Control_Key::KEY_PLUS:
        {
            if (m_is_sub_menu == true && is_move_channel_pos_enabled == false)
            {
                if (m_sub_selected_item < m_channel_data.size())
                {
                    auto toggle_favorite = !m_channel_data[m_sub_selected_item].favorite;
                    {
                        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                        current_lineup->set_favorite_by_viewer_channel(m_channel_data[m_sub_selected_item].viewer_channel, toggle_favorite);
                    }

                    update_channel_table_list();
                    set_sub_menu_selection(m_sub_selected_item);
                }
            }

            return true;
        }

        default:
            return true;
    }

    return false;
}

void OSD_Edit_Channel::swap_services(int _from, int _to)
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    int idx_from = -1, idx_to = -1;
    auto &services = current_lineup->services;

    for (size_t i = 0; i < services.size(); ++i)
    {
        if (services[i].viewer_channel() == _from)
        {
            idx_from = i;
        }
        else if (services[i].viewer_channel() == _to)
        {
            idx_to = i;
        }
    }

    if (idx_from != -1 && idx_to != -1)
    {
        if (m_current_channel_list == 0)
        {
            auto oifav_a = services[idx_from].get_order_in_favorite();
            auto oifav_b = services[idx_to].get_order_in_favorite();
            services[idx_from].set_order_in_favorite(oifav_b);
            services[idx_to].set_order_in_favorite(oifav_a);
        }
        // Swap the order in fulll
        else
        {
            auto oifull_a = services[idx_from].get_order_in_full();
            auto oifull_b = services[idx_to].get_order_in_full();
            services[idx_from].set_order_in_full(oifull_b);
            services[idx_to].set_order_in_full(oifull_a);
        }
    }
}

void OSD_Edit_Channel::got_focus()
{
    m_start_pos = 0;
    reset_menu_selection();
    move_selection(0, m_selected_item);
}

} // namespace mb
