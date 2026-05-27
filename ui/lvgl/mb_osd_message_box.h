#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_keys.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Message_Box: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> Message_Box_CB_t;

    enum class Message_Type
    {
        OK,
        YES_NO,
        COUNT
    };

private:
    static constexpr auto width = DISPLAY_WIDTH / 2;
    static constexpr auto heigth = DISPLAY_HEIGHT / 2;

    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_box { nullptr };
    lv_obj_t *m_label { nullptr };

    static constexpr auto offset_x = 715;
    static constexpr auto offset_y = 114;

    Message_Type m_message_type = Message_Type::OK;

    MB_OSD_Keys m_options;
    static constexpr auto option_x = 220;
    static constexpr auto option_y = 250;
    static constexpr auto option_w = 160;
    static constexpr auto option_h = 50;
    static constexpr auto option_s = 962 - 789;

    void draw_message_box(std::string_view _text);

    Message_Box_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Message_Box(OSD *_parent);
    virtual ~OSD_Message_Box();
    void show_message_box_yes_no(Message_Box_CB_t _callback, std::string_view _text, bool _yes_select);
    void show_message_box_ok(Message_Box_CB_t _callback, std::string_view _text);
    void show_message_box(std::string_view _text);
    void update_message(std::string_view _text);
};

} // namespace mb
