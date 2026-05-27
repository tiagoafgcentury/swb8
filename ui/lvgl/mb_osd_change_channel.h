#pragma once

#include "mb_osd.h"
#include "mb_osd_clock.h"

#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"

#include "common/mb_lineup.h"

#define MAX_DIGIT_KEY_PRESSED 5

namespace mb {

class OSD_Translate;

class OSD_Change_Channel : public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(Viewer_Channel_t)> Viewer_Channel_CB_t;
    typedef std::function<void(void)> OSD_Change_Channel_CB_t;

private:
    Viewer_Channel_CB_t m_vc_callback;
    OSD_Change_Channel_CB_t m_callback;

    static constexpr auto main_x = 880;
    static constexpr auto main_y = 0;
    static constexpr auto main_w = 400;
    static constexpr auto main_h = DISPLAY_HEIGHT;
    static constexpr auto MAX_CHANNEL_LIST_VIEW = 9u;
    static constexpr auto CHANNEL_LIST_SPACING = 50;
    static constexpr auto CHANNEL_LIST_TOP = 212;

    lv_timer_t *m_tmr_message { nullptr };
    lv_timer_t *m_tmr_key_pressed { nullptr };
    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bgd_channel { nullptr };
    lv_obj_t *m_box_channel { nullptr };
    lv_obj_t *m_lbl_channel { nullptr };
    lv_obj_t *m_lbl_message { nullptr };
    lv_obj_t *m_bgd_message { nullptr };
    lv_obj_t *m_message_label { nullptr };

    const Service *m_srv { nullptr };
    All_Channel_Data m_channel_data;
    char m_channel_number[10] = {0};
    std::vector<Menu_Data> m_menu_data;
    uint16_t m_selected_item = 0;
    uint16_t m_start_pos = 0;


#ifdef MBGUI_ANIMATION
    lv_anim_t m_anim;
    static void anim_delete_callback(lv_anim_t *_anim);
#endif

    void create_change_channel(lv_obj_t *_parent);
    void remote_control_process_key_digit(char _digit);
    void channel_digit_pressed();

    bool is_pressing_key { false };
    std::vector<char> m_remote_control_key_buffer;
    std::string m_sel_channel_number;
    static void delete_message_info_cb(lv_timer_t *tm);

    void update_channel_table_list(std::string _channel_number);
    void add_channel_info();
    void display_channel_list(uint16_t start_idx, uint16_t selected_item);
    void set_menu_selection(uint16_t _to);
    void move_selection(uint16_t _to);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Change_Channel(OSD *_parent);
    virtual ~OSD_Change_Channel();

    void set_channel(const std::string &str);
    void show_menu_digit(lv_obj_t *_parent, char _digit, Viewer_Channel_CB_t _viewer_channel_cb);
    void show_message_box(OSD_Change_Channel_CB_t _callback);
    void delete_message_info();
    void remote_control_process_key_buffer();

    void move_up();
    void move_down();

    virtual void hide_menu();

    void process();
};

} // namespace mb

