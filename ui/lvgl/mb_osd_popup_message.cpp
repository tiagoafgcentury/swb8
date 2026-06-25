#pragma once

#include "mb_osd.h"
#include "mb_osd_popup_message.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include <lvgl.h>

#include <cstddef>
#include <iostream>
#include <cstring>
#include <array>

#include <common/mb_config.h>

#if __has_include(<cJSON.h>)
    #include <cJSON.h>
#else
    #if __has_include(<cjson/cJSON.h>)
        #include <cjson/cJSON.h>
    #endif
#endif

namespace mb {

std::shared_ptr<OSD_Popup_Message> OSD_Popup_Message::s_instance;

std::shared_ptr<OSD_Popup_Message> OSD_Popup_Message::get_instance()
{
    if (s_instance)
    {
        return s_instance;
    }

    return std::make_shared<OSD_Popup_Message>();
}

OSD_Popup_Message::OSD_Popup_Message():
    OSD(nullptr)
{
    memset(m_messages.data(), 0, sizeof(lv_obj_t *) * m_messages.size());
    memset(m_popup_timers.data(), 0, sizeof(lv_timer_t *) * m_popup_timers.size());
}

OSD_Popup_Message::~OSD_Popup_Message()
{
    for (auto ptr : m_messages)
    {
        if (ptr)
        {
            lv_obj_delete(ptr);
        }
    }

    for (auto timer : m_popup_timers)
    {
        if (timer)
        {
            lv_timer_del(timer);
        }
    }

    memset(m_messages.data(), 0, sizeof(lv_obj_t *) * m_messages.size());
    memset(m_popup_timers.data(), 0, sizeof(lv_timer_t *) * m_popup_timers.size());
}

void OSD_Popup_Message::hide_popup_message(Message_Categories _category, bool _do_cleanup)
{
    auto idx = static_cast<int>(_category);
    auto ptr = m_messages[idx];

    if (ptr)
    {
        lv_obj_delete(ptr);
        m_messages[idx] = nullptr;
    }

    auto timer = m_popup_timers[idx];

    if (timer)
    {
        lv_timer_del(timer);
        m_popup_timers[idx] = nullptr;
    }

    if (_do_cleanup)
    {
        for (auto obj : m_messages)
        {
            if (obj)
            {
                return;
            }
        }

        s_instance.reset();
    }
}

static lv_color_t parse_color_from_json(cJSON *_root, const char *_key)
{
    cJSON *obj = cJSON_GetObjectItemCaseSensitive(_root, _key);

    if (!obj || !cJSON_IsString(obj))
    {
        return lv_color_hex(0x000000);
    }

    std::string value = obj->valuestring;

    if (value.empty())
    {
        return lv_color_hex(0x000000);
    }

    int decimal = std::stoi(value, nullptr, 16);

    return lv_color_hex(decimal);
}

void OSD_Popup_Message::show_popup_message(const Event_Display_Message &_message)
{
    auto category = _message.category;

    hide_popup_message(category, false);

    if (_message.message.empty())
    {
        return;
    }

    if (!s_instance)
    {
        s_instance = shared_from_this();
    }

    lv_color_t text_color = lv_color_hex(0xFFFFFF);
    lv_color_t bg_color   = lv_color_hex(0x000000);
    int timeout_ms = _message.timeout.count();
    std::string display_message = _message.message;
    DEBUG_MSG(OSD, DEBUG, TERM_BLUE_BOLD "Showing popup message:\n" << display_message.c_str() << "\n" TERM_RESET);
    if (category != Message_Categories::Event_Finger_Print)
    {
        process_nagra_message(display_message);
    }
    DEBUG_MSG(OSD, DEBUG, TERM_BLUE_BOLD "Showing popup message:\n" << display_message.c_str() << "\n" TERM_RESET);

    cJSON *root = cJSON_Parse(_message.message.c_str());
    bool fp_blink = false;
    bool fp_scroll = false;
    int fp_y = DISPLAY_HEIGHT - 140;

    if (root)
    {
        text_color = parse_color_from_json(root, "text_color");
        bg_color   = parse_color_from_json(root, "bg_color");

        auto *msg = cJSON_GetObjectItemCaseSensitive(root, "message");

        if (msg && cJSON_IsString(msg))
        {
            display_message = msg->valuestring;
        }

        auto *timeout = cJSON_GetObjectItemCaseSensitive(root, "timeout");

        if (timeout && cJSON_IsNumber(timeout))
        {
            timeout_ms = timeout->valueint;
        }

        if(category == Message_Categories::Event_Finger_Print)
        {
            auto *blink = cJSON_GetObjectItemCaseSensitive(root, "blink");

            if(blink && cJSON_IsBool(blink))
            {
                fp_blink = cJSON_IsTrue(blink);
            }

            auto *scroll = cJSON_GetObjectItemCaseSensitive(root, "scroll");

            if(scroll && cJSON_IsBool(scroll))
            {
                fp_scroll = cJSON_IsTrue(scroll);
            }

            auto *ypos = cJSON_GetObjectItemCaseSensitive(root, "y_position");

            if(ypos && cJSON_IsNumber(ypos))
            {
                fp_y = ypos->valueint;
                fp_y = std::max(0, fp_y);
                fp_y = std::min(fp_y, DISPLAY_HEIGHT - 80);
            }
        }
    }

    lv_obj_t *main_box = nullptr;

    if(category == Message_Categories::Event_Finger_Print && root)
    {

        main_box = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 60, fp_y, DISPLAY_WIDTH - 120, 100, bg_color);
        lv_obj_set_style_bg_opa(main_box, LV_OPA_40, 0);
        lv_obj_set_style_border_width(main_box, 1, 0);
        lv_obj_set_style_border_color(main_box, text_color, 0);
        lv_obj_set_style_radius(main_box, 8, 0);
        lv_obj_move_foreground(main_box);
    }
    else
    {
        main_box = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, bg_color);
        lv_obj_set_style_bg_opa(main_box, LV_OPA_90, 0);
        lv_obj_set_style_radius(main_box, 25, 0);
    }

    lv_font_t *font = (category == Message_Categories::Event_Finger_Print) ? const_cast<lv_font_t *>(font_25) : const_cast<lv_font_t *>(font_30);
    int text_y_offset = 0;
    if (category == Message_Categories::Event_Finger_Print)
    {
        m_caid = set_label_text_static(main_box, "CAID: ---", 0, 0, font_20, text_color);
        lv_obj_null_on_delete(&m_caid);
        lv_obj_set_style_text_align(m_caid, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(m_caid, LV_ALIGN_TOP_MID, 0, 6);

        m_scua = set_label_text_static(main_box, "SCUA: ---", 0, 0, font_20, text_color);
        lv_obj_null_on_delete(&m_scua);
        lv_obj_set_style_text_align(m_scua, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(m_scua, LV_ALIGN_TOP_MID, 0, 34);

        using namespace std::placeholders;

        m_process_scua_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Popup_Message::set_cas_fingerprint,                      this, _2, _3));
        Task_CAS::get_instance()->get_cas_fingerprint(m_process_scua_cb);
        text_y_offset = 24;
    }

    auto text_label = set_label_text(main_box, "", 0, 0, font, text_color);
    if(fp_scroll)
    {
        lv_label_set_long_mode(text_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    }
    else
    {
        lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);
    }

    if (category == Message_Categories::Event_Finger_Print)
    {
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, text_y_offset);
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
    }
    else
    {
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
    }

    DEBUG_MSG(OSD, DEBUG, TERM_BLUE_BOLD "Showing popup message:\n" << display_message.c_str() << "\n" TERM_RESET);    
    lv_label_set_text(text_label, display_message.c_str());
    if(fp_blink)
    {
        static auto blink_cb = [](void *obj, int32_t v)
        {
            lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), v, 0);
        };

        lv_anim_t anim;

        lv_anim_init(&anim);
        lv_anim_set_var(&anim, main_box);
        lv_anim_set_values(&anim, LV_OPA_20, LV_OPA_100);
        lv_anim_set_time(&anim, 500);
        lv_anim_set_playback_time(&anim, 500);
        lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&anim, blink_cb);
        lv_anim_start(&anim);
    }

    auto network_id = Config::get_config()->selected_satellite_config().network_id;
    if (network_id == Network_Id_Sky && category != Message_Categories::Event_Finger_Print)
    {
        load_image(main_box, LOGO_SKY_125X50, 90, 90, 125, 50);
    }

    switch (category)
    {
        case Message_Categories::Event_Finger_Print:
            lv_obj_move_foreground(main_box);
            break;

        case Message_Categories::Event_Popup:
            lv_obj_move_to_index(main_box, 1);
            break;

        default:
            lv_obj_move_to_index(main_box, 0);
            break;
    }

    if (timeout_ms <= 0)
    {
        if (category == Message_Categories::Event_Finger_Print)
        {
            timeout_ms = 15000;
        }
        else
        {
            timeout_ms = 10000;
        }
    }

    auto timer = lv_timer_create(timer_cb, timeout_ms, reinterpret_cast<void *>(category));
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_set_auto_delete(timer, true);

    m_popup_timers[static_cast<int>(category)] = timer;
    m_messages[static_cast<int>(category)] = main_box;

    if (root)
    {
        cJSON_Delete(root);
    }
}

void OSD_Popup_Message::timer_cb(lv_timer_t *_timer)
{
    if (!s_instance)
    {
        return;
    }

    auto category = reinterpret_cast<int>(lv_timer_get_user_data(_timer));
    s_instance->hide_popup_message(static_cast<Message_Categories>(category), true);
}

bool OSD_Popup_Message::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        case Remote_Control_Key::KEY_VOLTAR:
        {
            bool has_popup = m_messages[static_cast<int>(Message_Categories::Event_Popup)];
            bool has_fp = m_messages[static_cast<int>(Message_Categories::Event_Finger_Print)];

            if (has_popup || has_fp)
            {
                hide_popup_message(Message_Categories::Event_Popup);
                hide_popup_message(Message_Categories::Event_Finger_Print);
                remove_focus();
                return true;
            }
            break;
        }

        default:
            break;
    }

    return false;
}

void OSD_Popup_Message::process_nagra_message(std::string &_message)
{
    if (_message.find("Código: ") == std::string::npos)
    {
        return;
    }

    auto find_integer_after = [](const std::string &_str, const std::string &_after) -> int
    {
        size_t pos = _str.find(_after);

        if (pos == std::string::npos)
        {
            return -1;
        }

        pos += _after.length();

        int res = 0;
        bool found = false;

        while (pos < _str.size())
        {
            if (isdigit(_str[pos]))
            {
                found = true;
                res = res * 10 + (_str[pos] - '0');
            }
            else if (found)
            {
                break;
            }

            pos++;
        }

        return found ? res : -1;
    };

    auto code = find_integer_after(_message, "Código: ");
    auto it = m_nagra_code_map.find(code);
    if (it == m_nagra_code_map.end())
    {
        return;
    }

    std::string result = it->second;
    size_t tab_pos = result.find('\t');
    if (tab_pos == std::string::npos)
    {
        return;
    }

    int start = OSD_Translate::is_portuguese() ? 0 : tab_pos + 1;
    int end = OSD_Translate::is_portuguese() ? tab_pos : result.size();
    std::string translated = result.substr(start, end - start);
    size_t newline_pos = _message.find('\n');
    if (newline_pos != std::string::npos)
    {
        _message = translated + _message.substr(newline_pos);
    }
    else
    {
        _message = translated;
    }
}

void OSD_Popup_Message::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    if (m_caid)
    {
        lv_label_set_text_fmt(m_caid, "CAID: %s", _caid.c_str());
    }

    if (m_scua)
    {
        lv_label_set_text_fmt(m_scua, "SCUA: %s", _scua.c_str());
    }
}

} // namespace mb

