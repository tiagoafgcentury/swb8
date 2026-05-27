#include "mb_osd_channel_list.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_data.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "tasks/mb_task_application.h"
#include "mb_events.h"

#include <lvgl.h>

namespace mb {

OSD_Channel_List::OSD_Channel_List(OSD *_parent):
    OSD(_parent)
{
}

OSD_Channel_List::~OSD_Channel_List()
{
    m_channel_data.clear();
    m_menu_data.clear();
    m_sub_menu_data.clear();
    DELETE_OBJ(m_main_menu);
    remove_focus();
}

void OSD_Channel_List::change_channel()
{
    Transponder_Id tp_id = m_channel_data[m_sub_selected_item].transponder_id;
    Service_ID_t service_id = m_channel_data[m_sub_selected_item].service_id;
    uint16_t i = 0;

    for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
    {
        if (srv.service_id() == service_id and srv.transponder_id() == tp_id)
        {
            Task::post_event_channel_change(POST_CALLER & srv);
            m_current_channel = i;
            break;
        }

        i += 1;
    }
}

void OSD_Channel_List::set_favorite_channel(Menu_Data &ch_info, bool _selected)
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

void OSD_Channel_List::reset_menu_selection()
{
    m_selected_item = m_current_channel_list;

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

void OSD_Channel_List::reset_sub_menu_selection()
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

void OSD_Channel_List::display_channel_list(uint16_t start_idx, uint16_t selected_item)
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
        auto str = std::to_string(m_channel_data[idx].viewer_channel) + std::string(" - ") + std::string{m_channel_data[idx].channel_name};

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

void OSD_Channel_List::set_sub_menu_selection(uint16_t _sub_menu_to)
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

void OSD_Channel_List::move_sub_selection(uint16_t /*_from*/, uint16_t _to)
{
    set_sub_menu_selection(_to);
    update_channel_info();
}

void OSD_Channel_List::add_menu_fade()
{
    for (auto idx = 0u; idx < m_menu_data.size(); idx++)
    {
        auto &menu = m_menu_data[idx];

        if (idx == static_cast<decltype(idx)>(m_selected_item))
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_GREY, 0);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_ORANGE_DARK, 0);
        }
        else
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_GREY, 0);
        }
    }

    lv_obj_remove_flag(m_bgd_channel_info, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(m_instructions, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Channel_List::remove_menu_fade()
{
    for (auto idx = 0u; idx < m_menu_data.size(); idx++)
    {
        auto &menu = m_menu_data[idx];

        if (idx == static_cast<decltype(idx)>(m_selected_item))
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_WHITE, 0);
            lv_obj_set_style_bg_color(menu.sel, OSD_COLOR_ORANGE, 0);
        }
        else
        {
            lv_obj_set_style_text_color(menu.lbl, OSD_COLOR_WHITE, 0);
        }
    }

    lv_obj_add_flag(m_bgd_channel_info, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_instructions, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Channel_List::set_menu_selection(uint8_t _menu, bool _selected)
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

void OSD_Channel_List::move_selection(uint8_t _from, uint8_t _to)
{
    update_channel_view_list();
    set_menu_selection(_from, false);
    set_menu_selection(_to, true);
}

void OSD_Channel_List::add_sub_menu(std::string_view _icon)
{
    uint16_t size = MAX_CHANNEL_LIST_VIEW;
    // Desenha linha vertical
    m_line_left = create_rect(m_main_menu, 390, m_mn_top, 3, 450, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_left);

    for (size_t idx = 0; idx < size; idx++)
    {
        auto y = m_subMenu_top + (MENU_SUB_MENU_SPACING * idx);
        auto sel = create_rect(m_main_menu, 410, y, 445, 50, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(sel, 25, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_opa(sel, LV_OPA_TRANSP, 0);
        auto img = load_image(sel, _icon, 0, 0, 27, 25);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);
        auto lbl = set_label_text(sel, "", 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 60, 0);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
        lv_obj_set_width(lbl, 330);
        lv_obj_set_height(lbl, 35);
        //if (m_channel_data.size() > 0 and m_channel_data[idx].favorite == false)
        {
            lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
        }

        m_sub_menu_data.emplace_back(sel, img, nullptr, lbl);
    }
}

int OSD_Channel_List::add_menu(std::string_view _title)
{
    auto pos = m_menu_data.size();
    auto y = m_mn_top + (MENU_SPACING * pos);
    lv_obj_t *img { nullptr };
    lv_obj_t *img_sel { nullptr };
    auto sel = create_rect(m_main_menu, 90, y, 260, 50, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(sel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(sel, 25, DEFAULT_SELECTOR);
    auto lbl = set_label_text(sel, _title, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 30, 0);
    m_menu_data.emplace_back(sel, img, img_sel, lbl);
    return pos;
}

void OSD_Channel_List::update_channel_info()
{
    char buffer[200];

    if (static_cast<unsigned int>(m_selected_item) < MAX_MENU_ITEM and m_sub_selected_item < m_channel_data.size())
    {
        const auto &ch_info = m_channel_data[m_sub_selected_item];
        auto sat_name = Lineup_Mutex_Ref::get_current_lineup()->get_sattelite_name(ch_info.transponder_id.satellite_id());
        auto config = Config::get_config();
        auto frequency = ch_info.transponder_id.frequency() / 1000;
        auto polarity = ch_info.transponder_id.polarity();
        auto pol = polarity == Polarity::Horizontal ? "H" : "V";
        auto tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(ch_info.transponder_id);
        auto sr = tp ? tp->symbol_rate : 0;
        snprintf(buffer, sizeof(buffer) - 1, "%dMHz/%dKbps %s", frequency, sr, pol);
        lv_label_set_text(m_lbl_freq, buffer);
        snprintf(buffer, sizeof(buffer) - 1, "%s: %s %s", tr(__Satelite).data(), sat_name.c_str(), config->band() == Band::Ku ? "KU" : "C");
        lv_label_set_text(m_lbl_satelite, buffer);
    }
    else
    {
        lv_label_set_text_static(m_lbl_freq, "");
        lv_label_set_text_static(m_lbl_satelite, "");
    }
}

void OSD_Channel_List::set_current_channel_list_view()
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

void OSD_Channel_List::update_channel_table_list()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    m_channel_data = current_lineup->get_list(static_cast<Channel_List_Type>(m_current_channel_list));

    if (m_sub_selected_item >= m_channel_data.size())
    {
        m_sub_selected_item = m_channel_data.size() - 1;
    }
}

void OSD_Channel_List::enter_sub_menu()
{
    if (m_channel_data.size() > 0)
    {
        add_menu_fade();
        m_start_pos = m_sub_selected_item;
        move_sub_selection(0, m_sub_selected_item);
        m_is_sub_menu = true;
    }
}

void OSD_Channel_List::show_menu(lv_obj_t *m_bgd, OSD_Channel_List_CB_t _callback)
{
    set_focus();
    m_callback = _callback;

    if (!m_main_menu)
    {
        //MainMenu
        m_main_menu = create_rect(m_bgd, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_main_menu);
        lv_obj_set_style_bg_opa(m_main_menu, LV_OPA_90, 0);
        add_menu(std::string(tr(__Meus_canais_TV)));
        add_menu(std::string(tr(__Minhas_radios)));
        add_menu(std::string(tr(__Todos_canais_TV)));
        add_menu(std::string(tr(__Todas_Radios)));
        m_bgd_channel_info = create_rect(m_main_menu, 112, 475, 240, 100, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_channel_info);
        lv_obj_set_style_bg_opa(m_bgd_channel_info, LV_OPA_TRANSP, 0);
        set_label_text(m_bgd_channel_info, tr(__Informacoes_do_canal), 0, 0, font_semi_20, OSD_COLOR_GREY);
        m_lbl_freq = set_label_text(m_bgd_channel_info, "", 0, 30, font_semi_20, OSD_COLOR_GREY);
        lv_obj_null_on_delete(&m_lbl_freq);
        m_lbl_satelite = set_label_text(m_bgd_channel_info, "", 0, 60, font_semi_20, OSD_COLOR_GREY);
        lv_obj_null_on_delete(&m_lbl_satelite);
        m_img =  load_image(m_main_menu, LOGO_MENU_CENTURY_C_CINZA, logo_x, logo_y, logo_width, logo_heigth + 10);
        lv_obj_null_on_delete(&m_img);
        m_lbl = set_label_text_static(m_main_menu, tr(__Lista_de_Canais), 163, 51, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl);
        // Cria rodapé
        m_instructions = MB_OSD_Footer::draw(m_main_menu, tr(__Pressione_mais_para_favoritar_desfavoritar), -40);
        lv_obj_null_on_delete(&m_instructions);
        m_logo_midia = load_image(m_main_menu, LOGO_MIDIABOX_BRANCO_225x33, 965, 620, 225, 33);
        lv_obj_null_on_delete(&m_logo_midia);
        {
            auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
            m_current_channel_list = current_lineup->get_channel_list_type();  
            m_last_channel_list = current_lineup->get_channel_list_type();
        }

        DEBUG_MSG(OSD, DEBUG, "Current channel list type: " << static_cast<int>(m_current_channel_list) << "\n");
        update_channel_table_list();
        set_current_channel_list_view();

        if (m_current_service)
        {
            add_sub_menu(LOGO_FAVORITOS_BRANCO_17x15);
            reset_menu_selection();
            move_selection(0, static_cast<int>(m_selected_item));
            enter_sub_menu();
            update_channel_info();
        }
    }
}

void OSD_Channel_List::update_channel_view_list()
{
    std::string str;
    auto size = std::min<uint16_t>(MAX_CHANNEL_LIST_VIEW, m_channel_data.size());

    for (size_t idx = 0; idx < size; idx++)
    {
        auto &sub_menu = m_sub_menu_data[idx];
        str = std::to_string(m_channel_data[idx].viewer_channel) + std::string(" - ") + std::string{m_channel_data[idx].channel_name};

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

void OSD_Channel_List::move_menu_up()
{
    if (m_is_sub_menu == false)
    {
        m_sub_selected_item = 0;
        auto currentPos = m_selected_item;
        m_selected_item = increment_value(m_selected_item, -1);
        m_current_channel_list = m_selected_item;
        update_channel_table_list();
        move_selection(static_cast<unsigned int>(currentPos), static_cast<unsigned int>(m_selected_item));
    }
    else
    {
        auto currentPos = m_sub_selected_item;
        auto count = m_channel_data.size();
        m_sub_selected_item = (m_sub_selected_item + count - 1) % count;
        move_sub_selection(currentPos, m_sub_selected_item);
    }
}

void OSD_Channel_List::move_menu_down()
{
    if ((m_is_sub_menu) == false)
    {
        m_sub_selected_item = 0;
        auto currentPos = m_selected_item;
        m_selected_item = increment_value(m_selected_item, 1);
        m_current_channel_list = m_selected_item;
        update_channel_table_list();
        move_selection(static_cast<unsigned int>(currentPos), static_cast<unsigned int>(m_selected_item));
    }
    else
    {
        auto current_pos = m_sub_selected_item;
        auto count = m_channel_data.size();
        m_sub_selected_item = (m_sub_selected_item + count + 1) % count;
        move_sub_selection(current_pos, m_sub_selected_item);
    }
}

bool OSD_Channel_List::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            if (m_is_sub_menu == false)
            {
                auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                current_lineup->set_channel_list_type(m_last_channel_list);
                Task::post_event(m_callback);
            }
            else
            {
                reset_sub_menu_selection();
                m_is_sub_menu = false;
            }

            return true;

        case Remote_Control_Key::KEY_OK:
            if (m_is_sub_menu && m_channel_data.size() > 0)
            {
                {
                    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                    current_lineup->set_channel_list_type(m_current_channel_list);
                }

                change_channel();
                Task::post_event(m_callback);
            }

            return true;

        case Remote_Control_Key::KEY_CHUP:
            move_menu_up();
            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            move_menu_down();
            return true;

        case Remote_Control_Key::KEY_VOLUP:
            if (m_channel_data.size() > 0)
            {
                enter_sub_menu();
                update_channel_info();
            }

            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            m_is_sub_menu = false;
            m_start_pos = m_sub_selected_item;
            move_sub_selection(0, 0);
            reset_sub_menu_selection();
            return true;

        case Remote_Control_Key::KEY_PLUS:
        {
            if (m_is_sub_menu == true)
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

void OSD_Channel_List::got_focus()
{
    m_start_pos = 0;
    reset_menu_selection();
    move_selection(0, static_cast<unsigned int>(m_selected_item));
}

} // namespace mb

