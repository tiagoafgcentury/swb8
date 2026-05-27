#pragma once

#include "mb_osd.h"

#include "common/mb_lineup.h"
#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_breadcrumb.h"
#include "tasks/mb_task_eit_events.h"

namespace mb {

class OSD_Translate;

class OSD_Guide_Channel_Info : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> OSD_Guide_Channel_Info_CB_t;

private:
    static constexpr auto offset_x = 90;
    static constexpr auto offset_y = 120;
    static constexpr auto width = 890;
    static constexpr auto heigth = 180;

    static constexpr uint16_t curr_parental_rating_x = 90;
    static constexpr uint16_t curr_parental_rating_y = 0;
    static constexpr uint16_t next_parental_rating_x = 496;
    static constexpr uint16_t next_parental_rating_y = 0;
    static constexpr uint16_t instructions_offset = 450;
    static constexpr auto START_POS_X = 90;

    static constexpr auto NULL_TIME = "--:--";
    static constexpr auto NULL_CHANNEL = "Sem informações"; //tr(__Sem_informacoes);

    const Service *m_srv;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_box_info { nullptr };
    lv_obj_t *m_box_main { nullptr };
    lv_obj_t *m_center_box { nullptr };

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

    lv_timer_t  *m_refresh_timer { nullptr };

    void channel_detail_frame();
    void print_channel_info();
    void start_refresh_timer();
    static void refresh_ts_signal_cb(lv_timer_t *tm);
    void refresh_channel_detail();
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

    /*
        lv_obj_t *m_bgd { nullptr };
        lv_obj_t *m_box_info { nullptr };
        lv_obj_t *m_box_guide { nullptr };
        lv_obj_t *m_center_box { nullptr };
        lv_obj_t *m_main_sub_menu { nullptr };

        lv_obj_t *m_channel_name { nullptr };
        lv_obj_t *m_channel_desc { nullptr };
        lv_obj_t *m_program { nullptr };
        lv_obj_t *m_subtitle { nullptr };
        lv_obj_t *m_class_indic { nullptr };
        lv_obj_t *m_img_fav { nullptr };

        lv_obj_t *m_logo_midia { nullptr };

        char m_channel_number[10] = {0};
        Service *m_atual_srv { nullptr };
        static constexpr uint INVALID_CHANNEL = std::numeric_limits<uint>::max();
        uint m_current_channel { INVALID_CHANNEL };
    */
    OSD_Guide_Channel_Info_CB_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Guide_Channel_Info(OSD *_parent);
    virtual ~OSD_Guide_Channel_Info();

    virtual void show_menu_info(OSD_Guide_Channel_Info_CB_t _callback);

    void process();
};

} // namespace mb
