#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_osd.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_keys.h"

#include <memory>
#include <map>

namespace mb {

class OSD_Audio_LR: public OSD, public Remote_Control_Handler
{
private:
    typedef std::function<void(void)> Audio_LR_CB_t;

    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT;

    // Variáveis desta classe
    int m_sleep_timer_value = 0;
    lv_obj_t    *m_main { nullptr };
    lv_obj_t    *m_lbl { nullptr };
    lv_obj_t    *m_left_icon { nullptr };
    lv_obj_t    *m_right_icon { nullptr };
    lv_timer_t *m_exit_timer{ nullptr };

    lv_obj_t    *m_main_box { nullptr };
    static constexpr auto main_box_x = 800;
    static constexpr auto main_box_y = 160;
    static constexpr auto main_box_w = 400;
    static constexpr auto main_box_h = 360;
    lv_obj_t    *m_audio_box { nullptr };
    static constexpr auto audio_box_x = 10;
    static constexpr auto audio_box_y = 10;
    static constexpr auto audio_box_w = main_box_w - 20;
    static constexpr auto audio_box_h = main_box_h - 80;
    std::vector<lv_obj_t *> m_lang_box;
    std::vector<lv_obj_t *> m_lang_label;
    std::vector<std::string> m_lang_str;
    static constexpr auto audio_langs_x = 10;
    static constexpr auto audio_langs_y = 10;
    static constexpr auto audio_langs_w = audio_box_w - 20;
    static constexpr auto audio_langs_h = 50;
    static constexpr auto audio_langs_s = 60;

    static constexpr auto start_x = 915;
    static constexpr auto start_y = 220;
    static constexpr auto start_w = 380;
    static constexpr auto start_h = 50;

    MB_OSD_Keys m_keys;
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 0;
    static constexpr auto button_w = audio_box_w - 20;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 60;
    static constexpr auto button_x = 10;
    static constexpr auto button_y = 10;

    static void process_exit_cb(lv_timer_t *_tm);
    void draw_main_box();
    void draw_audio_box();
    void draw_audio_langs();
    void draw_lr_icons(size_t _index);
    void update_audio_lang();

    Audio_LR_CB_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Audio_LR(OSD *_parent);
    virtual ~OSD_Audio_LR();

    void show_audio_lr(Audio_LR_CB_t _callback);
};

} // namespace mb

