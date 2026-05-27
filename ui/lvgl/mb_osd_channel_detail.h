#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_diagnosis.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task_cas.h"
#include <memory>

namespace mb {

class Fade_Canvas;

class Channel_Detail : public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Channel_Detail_CB_t;

private:
    std::unique_ptr<OSD_Diagnosis> mb_osd_diagnostics;

    static constexpr auto width = 1280;
    static constexpr auto height = 720;

    static constexpr auto width_main = 8 * width / 10;
    static constexpr auto height_main = 8 * height / 10;
    static constexpr auto width_event = 9 * width_main / 10;
    static constexpr auto height_event = 4 * height_main / 10;
    static constexpr uint16_t curr_parental_rating_x = 90;
    static constexpr uint16_t curr_parental_rating_y = 276;
    static constexpr uint16_t next_parental_rating_x = 496;
    static constexpr uint16_t next_parental_rating_y = 276;
    static constexpr uint16_t instructions_offset = 450;
    static constexpr auto START_POS_X = 90;

    static constexpr auto NULL_TIME = "--:--";
    static constexpr auto NULL_CHANNEL = "Sem informações"; //tr(__Sem_informacoes);

    lv_obj_t *m_main_board { nullptr };
    lv_obj_t *m_bg_cover { nullptr };
    lv_obj_t *m_channel_title { nullptr };
    lv_obj_t *m_fav_top { nullptr };
    lv_obj_t *m_satellite_line { nullptr };
    lv_obj_t *m_service_line { nullptr };

    lv_obj_t *m_event_board { nullptr };
    lv_obj_t *m_current_event_title { nullptr };
    lv_obj_t *m_current_event_time { nullptr };
    lv_obj_t *m_current_event_subtitle { nullptr };
    lv_obj_t *m_current_class_indic { nullptr };

    lv_obj_t *m_next_event_title { nullptr };
    lv_obj_t *m_next_event_time { nullptr };
    lv_obj_t *m_next_event_subtitle { nullptr };
    lv_obj_t *m_next_class_indic { nullptr };
    lv_obj_t *m_center_line { nullptr };

    lv_obj_t *m_quality_line { nullptr };
    lv_obj_t *m_quality_slider { nullptr };
    lv_style_t m_quality_main;
    lv_style_t m_quality_indicator;

    lv_obj_t *m_strength_line { nullptr };
    lv_obj_t *m_strength_slider { nullptr };
    lv_style_t m_strength_main;
    lv_style_t m_strength_indicator;

    lv_style_transition_dsc_t m_transition_dsc;

    lv_obj_t *m_snr_box { nullptr };
    lv_obj_t *m_snr_title { nullptr };
    lv_obj_t *m_snr_value { nullptr };

    lv_obj_t *m_bgd_back { nullptr };

    lv_timer_t  *m_refresh_timer { nullptr };

    lv_obj_t *m_zone_id_box { nullptr };
    lv_obj_t *m_zone_id_label { nullptr };

    static void refresh_ts_signal_cb(lv_timer_t *tm);
    void channel_detail_frame();
    void parse_events();
    void create_quality_slider();
    void create_strength_slider();

    typedef struct
    {
        std::string name;
        std::string number;
        std::string program;
        std::string detail;
        std::string start_str;
        uint8_t start_hour;
        uint8_t start_min;
        std::string end_str;
        uint8_t end_hour;
        uint8_t end_min;
        Parental_Control parental_rating;
    } Channel_Detail_t;

    Channel_Detail_t m_current;
    Channel_Detail_t m_next;
    Channel_Detail_CB_t m_callback;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    void verify_sequence();
    void show_new_zone_id(int result);
    void duplicate_channel();
    void delete_channel();
    void new_zone_id();

    void show_diagnostics();
    void show_diagnostics_callback();

    bool m_block_cr = false;

    std::vector<Remote_Control_Key> m_incomming_keys = {};
    static constexpr size_t ARRAY_LENGTH = 4;
    static constexpr std::array<Remote_Control_Key, ARRAY_LENGTH> s_reference_keys =
    {
        Remote_Control_Key::KEY_MENU,
        Remote_Control_Key::KEY_VOLDOWN,
        Remote_Control_Key::KEY_VOLUP,
        Remote_Control_Key::KEY_VOLDOWN,
    };

    static constexpr std::array<Remote_Control_Key, ARRAY_LENGTH> s_channel_add_keys =
    {
        Remote_Control_Key::KEY_CHUP,
        Remote_Control_Key::KEY_CHDOWN,
        Remote_Control_Key::KEY_CHUP,
        Remote_Control_Key::KEY_1,
    };

    static constexpr std::array<Remote_Control_Key, ARRAY_LENGTH> s_channel_delete_keys =
    {
        Remote_Control_Key::KEY_CHUP,
        Remote_Control_Key::KEY_CHDOWN,
        Remote_Control_Key::KEY_CHUP,
        Remote_Control_Key::KEY_0,
    };

public:
    Channel_Detail(OSD *_parent);
    virtual ~Channel_Detail();

    void refresh_channel_detail();
    void show_channel_detail(Channel_Detail_CB_t _callback);
    void start_refresh_timer();
};

} // namespace mb

