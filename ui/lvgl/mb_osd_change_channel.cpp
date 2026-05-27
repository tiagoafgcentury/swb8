#include "mb_osd_change_channel.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd_fonts.h"
#include "tasks/mb_task_application.h"
#include "tasks/mb_task_player.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"

#include <lvgl.h>

namespace mb {

OSD_Change_Channel::OSD_Change_Channel(OSD *_parent):
    OSD(_parent)
{
}

OSD_Change_Channel::~OSD_Change_Channel()
{
    m_remote_control_key_buffer.clear();
    m_channel_data.channel_list.clear();
    m_menu_data.clear();
    is_pressing_key = false;
    //DELETE_TIMER(m_tmr_message);
    DELETE_TIMER(m_tmr_key_pressed);
    DELETE_OBJ(m_main_menu);
    remove_focus();
}

void OSD_Change_Channel::display_channel_list(uint16_t start_idx, uint16_t selected_item)
{
    auto size = std::min<uint16_t>(MAX_CHANNEL_LIST_VIEW, m_channel_data.channel_list.size());

    for (uint16_t idx = 0; idx < MAX_CHANNEL_LIST_VIEW; idx++)
    {
        auto &sub_menu = m_menu_data[idx];
        lv_label_set_text(sub_menu.lbl, "");
    }

    for (uint16_t idx = start_idx; idx < start_idx + size; idx++)
    {
        auto &sub_menu = m_menu_data[(idx - start_idx)];
        auto ch_data = m_channel_data.channel_list;
        auto str = ch_data[idx].channel_number + " - " + ch_data[idx].channel_name;
        lv_label_set_text(sub_menu.lbl, str.c_str());

        if (idx == selected_item)
        {
            m_sel_channel_number = ch_data[idx].channel_number;
            //set_channel(ch_data[idx].channel_number);
            lv_obj_set_style_bg_color(sub_menu.sel, OSD_COLOR_ORANGE, 0);
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_COVER, 0);
        }
        else
        {
            lv_obj_set_style_bg_color(sub_menu.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(sub_menu.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Change_Channel::add_channel_info()
{
    //auto size = std::min<uint16_t>(MAX_CHANNEL_LIST_VIEW, m_channel_data.channel_list.size());
    auto size = MAX_CHANNEL_LIST_VIEW;

    for (size_t idx = 0; idx < size; idx++)
    {
        auto y = CHANNEL_LIST_TOP + (CHANNEL_LIST_SPACING * idx);
        auto sel = create_rect(m_bgd_channel, main_x, y, 430, 50, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(sel, 25, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_opa(sel, LV_OPA_TRANSP, 0);
        auto sel_box = create_rect(sel, 0, 0, 320, 50, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(sel_box, LV_OPA_TRANSP, 0);
        lv_obj_align(sel_box, LV_ALIGN_RIGHT_MID, -95, 0);
        //auto str = m_channel_data.channel_list[idx].channel_number + " - " + m_channel_data.channel_list[idx].channel_name;
        auto lbl = set_label_text(sel_box, ""/*str.c_str()*/, 0, 0, font_semi_20, OSD_COLOR_WHITE);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
        lv_obj_set_width(lbl, 320);
        lv_obj_set_height(lbl, 25);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_RIGHT, 0);
        m_menu_data.emplace_back(std::move(sel), nullptr, nullptr, std::move(lbl));
    }
}

void OSD_Change_Channel::update_channel_table_list(std::string _channel_number)
{
    m_channel_data.channel_list.clear();
    uint16_t _number = atoi(_channel_number.c_str());

    if (_number > 0)
    {
        DEBUG_MSG(OSD, DEBUG, "Searching for channel: " << _number << "\n");
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

        for (auto &srv : current_lineup->services)
        {
            snprintf(m_channel_number, sizeof(m_channel_number), "%d", srv.viewer_channel());
            std::string str(m_channel_number);
            std::string new_number = std::to_string(_number);

            if (!str.empty() && str.find(new_number) == 0)
            {
                m_channel_data.channel_list.emplace_back(srv.name().data(), str, srv.is_favorite(), srv.service_id(), srv.transponder_id());
            }
        }
    }
}

void OSD_Change_Channel::create_change_channel(lv_obj_t *_parent)
{
    m_main_menu = create_rect(_parent, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_menu);
    lv_obj_set_style_bg_opa(m_main_menu, LV_OPA_TRANSP, 0);
    m_bgd_channel = create_rect(m_main_menu, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_channel);
    lv_obj_set_style_bg_opa(m_bgd_channel, LV_OPA_80, 0);
    m_box_channel = create_rect(m_bgd_channel, main_x + 220, 132, 115, 53, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_channel);
    lv_obj_set_style_bg_opa(m_box_channel, LV_OPA_TRANSP, 0);
    m_lbl_channel = set_label_text_static(m_box_channel, "", 0, 0, font_bold_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_lbl_channel);
    lv_obj_set_width(m_lbl_channel, 115);
    lv_obj_set_style_text_align(m_lbl_channel, LV_TEXT_ALIGN_RIGHT, 0);
    add_channel_info();
}

void OSD_Change_Channel::set_channel(const std::string &str)
{
    std::string txt = str + "-";
    lv_label_set_text(m_lbl_channel, txt.c_str());
}

void OSD_Change_Channel::show_menu_digit(lv_obj_t *_parent, char _digit, Viewer_Channel_CB_t _viewer_channel_cb)
{
    set_focus();
    m_vc_callback = _viewer_channel_cb;

    if (m_bgd_channel == nullptr)
    {
        update_channel_table_list(std::to_string(_digit));
        create_change_channel(_parent);
    }

    remote_control_process_key_digit(_digit);
}

void OSD_Change_Channel::hide_menu()
{
}

void OSD_Change_Channel::channel_digit_pressed()
{
    if (m_remote_control_key_buffer.size())
    {
        char str[m_remote_control_key_buffer.size() + 1];
        auto d = m_remote_control_key_buffer.data();

        for (auto i = 0u; i < m_remote_control_key_buffer.size(); i++, d++)
        {
            str[i] = '0' + *d;
        }

        str[static_cast<int>(m_remote_control_key_buffer.size())] = 0;
        set_channel(str);
        update_channel_table_list(str);
        move_selection(0);
    }
}

bool OSD_Change_Channel::handle_event_remote_control(const Event_Remote_Control &_event)
{
    char key = get_digit(_event.key);

    if (key != '\n')
    {
        remote_control_process_key_digit(key);
    }

    switch (_event.key)
    {

        case Remote_Control_Key::KEY_OK:
            if (!m_remote_control_key_buffer.empty())
            {
                remote_control_process_key_buffer();
            }
            return true;

        case Remote_Control_Key::KEY_CHUP:
            move_up();
            lv_timer_reset(m_tmr_key_pressed);
            return true;

        case Remote_Control_Key::KEY_CHDOWN:
            move_down();
            lv_timer_reset(m_tmr_key_pressed);
            return true;

        case Remote_Control_Key::KEY_VOLTAR:
            m_vc_callback(0);
            return true;

        default:
            return true;
    }
}

static void wait_remote_control_key_pressed_cb(lv_timer_t *timer)
{
    OSD_Change_Channel *thiz = static_cast<OSD_Change_Channel *>(lv_timer_get_user_data(timer));
    thiz->remote_control_process_key_buffer();
}

void OSD_Change_Channel::remote_control_process_key_digit(char _digit)
{
    is_pressing_key = true;

    if (m_remote_control_key_buffer.size() < MAX_DIGIT_KEY_PRESSED)
    {
        m_remote_control_key_buffer.push_back(_digit);
        channel_digit_pressed();
    }

    if (m_tmr_key_pressed == nullptr)
    {
        m_tmr_key_pressed = lv_timer_create(wait_remote_control_key_pressed_cb, 5000, this);
    }
    else
    {
        lv_timer_reset(m_tmr_key_pressed);
    }
}

void OSD_Change_Channel::remote_control_process_key_buffer()
{
    Viewer_Channel_t viewer_channel = 0;

    if (!m_sel_channel_number.empty() && m_channel_data.channel_list.size() > 0)
    {
        viewer_channel = static_cast<Viewer_Channel_t>(std::strtoul(m_sel_channel_number.c_str(), nullptr, 10));
    }
    else
    {
        for (auto key : m_remote_control_key_buffer)
        {
            viewer_channel = (viewer_channel * 10) + key;
        }

        m_remote_control_key_buffer.clear();
        channel_digit_pressed();
    }

    m_vc_callback(viewer_channel);
}

void OSD_Change_Channel::delete_message_info_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    OSD_Change_Channel *thiz = static_cast<OSD_Change_Channel *>(lv_timer_get_user_data(tm));
    thiz->delete_message_info();
}

void OSD_Change_Channel::delete_message_info()
{
    //hide_menu();
    remove_clock();
    Task::post_event(m_callback);
}

void OSD_Change_Channel::show_message_box(OSD_Change_Channel_CB_t _callback)
{
    m_callback = _callback;

    if (m_bgd_channel)
    {
        lv_obj_add_flag(m_bgd_channel, LV_OBJ_FLAG_HIDDEN);
    }

    m_bgd_message = create_rect(m_main_menu, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_message);
    lv_obj_align(m_bgd_message, LV_ALIGN_CENTER, 0, 0);
    set_label_text_static(m_bgd_message, tr(__Sem_informacoes), 90, 28, font_bold_40, OSD_COLOR_GREY_MEDIUM);
    set_label_text_static(m_bgd_message, tr(__Sem_informacoes), 90, 78, font_20, OSD_COLOR_GREY_MEDIUM);
    set_label_text_static(m_bgd_message, tr(__Sem_informacoes), 97, 568, font_25, OSD_COLOR_GREY_MEDIUM);
    set_label_text_static(m_bgd_message, tr(__Sem_informacoes), 97, 628, font_25, OSD_COLOR_GREY_MEDIUM);
    create_rect(m_bgd_message, 90, 614, 1100, 3, OSD_COLOR_GREY_MEDIUM);
    load_image(m_bgd_message, LOGO_MIDIABOX_BRANCO_208x30, 982, 575, 208, 40);
    load_image(m_bgd_message, LOGO_INFO_34x34, 1024, 628, 34, 34);
    load_image(m_bgd_message, LOGO_AUDIO_LR_34x34, 1068, 628, 34, 34);
    load_image(m_bgd_message, LOGO_CC_34x34, 1112, 628, 34, 34);
    load_image(m_bgd_message, LOGO_MAIS_34x34, 1156, 628, 34, 34);
    add_clock(m_bgd_message, 1090, 28);
    m_message_label = set_label_text_static(m_bgd_message, tr(__Canal_invalido), 0, 0, font_bold_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_message_label);
    lv_obj_align(m_message_label, LV_ALIGN_CENTER, 0, 0);
    m_tmr_message = lv_timer_create(delete_message_info_cb, 3000, this);
    lv_timer_set_repeat_count(m_tmr_message, 1);
}

void OSD_Change_Channel::set_menu_selection(uint16_t _to)
{
    if (_to < m_start_pos)
    {
        m_start_pos = _to;
    }
    else if (_to >= (m_start_pos + MAX_CHANNEL_LIST_VIEW))
    {
        m_start_pos = _to - MAX_CHANNEL_LIST_VIEW + 1;
    }

    display_channel_list(m_start_pos, _to);
}

void OSD_Change_Channel::move_selection(uint16_t _to)
{
    set_menu_selection(_to);
    //update_channel_info();
}

void OSD_Change_Channel::move_up()
{
    if (m_selected_item > 0)
    {
        m_selected_item -= 1;
    }

    move_selection(m_selected_item);
}

void OSD_Change_Channel::move_down()
{
    if (++m_selected_item >= m_channel_data.channel_list.size())
    {
        m_selected_item = m_channel_data.channel_list.size() - 1;
    }

    move_selection(m_selected_item);
}

void OSD_Change_Channel::got_focus()
{
}

} // namespace mb
