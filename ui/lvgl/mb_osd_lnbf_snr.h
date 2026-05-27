#pragma once

#include "mb_osd.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task.h"
#include "mb_events.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "mb_osd_keys.h"
#include "hal/mb_sound.h"
#include "mb_lnbf_detection.h"
#include "common/mb_satellites.h"

#include <map>

namespace mb {

class OSD_Translate;

class OSD_Lnbf_Snr : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> osd_lnbf_snr_callback_t;
    osd_lnbf_snr_callback_t m_callback;

private:
    std::unique_ptr<MB_Detect_Lnbf> m_detect_lnbf;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main                { nullptr };
    lv_obj_t *m_title               { nullptr };
    static constexpr auto title_y = 0;
    static constexpr auto title_h = 53;

    lv_obj_t *m_bgd_subtitle        { nullptr };
    lv_obj_t *m_subtitle            { nullptr };
    static constexpr auto subtitle_x = 214;
    static constexpr auto subtitle_y = 260 - offset_y;
    static constexpr auto subtitle_h = 66;
    static constexpr auto subtitle_w = 852;

    lv_obj_t *m_snr                 { nullptr };
    static constexpr auto snr_x = 929;
    static constexpr auto snr_y = 440 - offset_y;
    static constexpr auto snr_h = 33;
    static constexpr auto snr_w = 180;

    lv_obj_t *m_info                { nullptr };
    lv_obj_t *m_info_box            { nullptr };
    static constexpr auto info_y =  380 - offset_y;
    static constexpr auto info_h =  27;

    lv_obj_t *m_footer              { nullptr };
    static constexpr auto footer_y =      643 - offset_y;
    static constexpr auto footer_h =      info_h;

    // Barra de progresso
    lv_obj_t *m_slider_label        { nullptr };
    static constexpr auto slider_label_x = 507;
    static constexpr auto slider_label_y = 440 - offset_y;

    lv_obj_t *m_slider              { nullptr };
    static constexpr auto slider_y =      418 - offset_y;
    static constexpr auto slider_w =      910;
    static constexpr auto slider_x = (width - slider_w) / 2;
    static constexpr auto slider_h =      20;
    lv_obj_t *m_progress_line { nullptr };
    lv_obj_t *m_progress_slider { nullptr };
    lv_style_t m_progress_main;
    lv_style_t m_progress_indicator;
    lv_style_t m_progress_knob;
    static constexpr auto width_event = 728;
    lv_style_transition_dsc_t m_transition_dsc;
    lv_timer_t  *m_refresh_timer { nullptr };
    int m_current_progress = 0;
    static void refresh_cb(lv_timer_t *_tm);

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto btn_y = 530;
    static constexpr auto btn_w = 220;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_s = 0;
    static constexpr auto btn_x = ( width / 2 ) - ( btn_w / 2 );

    bool m_signal_found = false;
    
    void draw_buttons();
    void create_progress_bar();
    void to_point();
    void populate();
    void update_subtitle();
    void next_transponder();
    static void next_transponder_callback(const std::string &result);
    void refresh_info_box(const std::string &result);

    static void beep_cb(lv_timer_t* tm);
    void process_beep();
    lv_timer_t* m_beep_timer { nullptr };
    double m_last_snr { 0.0 };
    int m_last_range { -1 };
    bool m_beep_on { false };
    uint32_t m_beep_counter { 0 };



protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Lnbf_Snr *s_instance;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Lnbf_Snr(OSD *_parent);
    virtual ~OSD_Lnbf_Snr();

    virtual void show_lnbf_snr(osd_lnbf_snr_callback_t _callback);
    virtual void hide_menu();
    void refresh_progress();
    void update_info(Event_Transponder_data _progress);
};

} // namespace mb
