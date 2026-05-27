#pragma once

#include <array>
#include <memory>

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_remote_control_handler.h"
#include "mb_lnbf_detection.h"

namespace mb {

class Fade_Canvas;

class OSD_Auto_Search_Channel_List : public OSD, public Remote_Control_Handler {
 public:
    typedef std::function<void(bool)> OSD_Auto_Search_Channel_List_CB_t;

 private:
    std::unique_ptr<MB_Detect_Lnbf> m_detect_lnbf;

    std::vector<Transponder> m_detected_transponders;
    uint32_t m_total_transponders_found = 0;

    static constexpr auto offset_x = 90;
    static constexpr auto offset_y = 114;
    static constexpr auto width = DISPLAY_WIDTH - offset_x;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t* table_title{nullptr};
    static constexpr uint16_t MAX_CHANNEL_LIST_VIEW = 10;
    static constexpr uint16_t TABLE_SPACING = 38;
    static constexpr auto table_line_w = 910;
    static constexpr auto table_line_h = 38;
    static constexpr auto table_top = 0;

    std::array<lv_obj_t*, MAX_CHANNEL_LIST_VIEW> m_table_line;
    std::array<lv_obj_t*, MAX_CHANNEL_LIST_VIEW> m_tv_lines_table;
    std::array<lv_obj_t*, MAX_CHANNEL_LIST_VIEW> m_radio_lines_table;

    lv_obj_t* m_bgd{nullptr};
    lv_obj_t* m_mainscreen{nullptr};
    lv_obj_t* m_title_label{nullptr};
    lv_obj_t* m_lbl_footer_back{nullptr};
    lv_obj_t* m_lbl_footer_ok{nullptr};
    lv_obj_t* m_btn_ok{nullptr};
    lv_obj_t* m_info_box{nullptr};

    lv_obj_t* m_bgd_table{nullptr};
    static constexpr auto bgd_table_x = 230 - offset_x;
    static constexpr auto bgd_table_y = 169 - offset_y;
    static constexpr auto bgd_table_w = 910;
    static constexpr auto bgd_table_h = MAX_CHANNEL_LIST_VIEW * table_line_h;

    // Barra de progresso
    lv_obj_t* m_bgd_slider{nullptr};
    static constexpr auto m_bgd_slider_x = 230 - offset_x;
    static constexpr auto m_bgd_slider_y = 570 - offset_y;
    static constexpr auto m_bgd_slider_w = 910;
    static constexpr auto m_bgd_slider_h = 55;

    lv_obj_t* m_slider_label{nullptr};
    lv_obj_t* m_slider{nullptr};
    static constexpr auto slider_w = 910;
    static constexpr auto slider_h = 20;
    lv_style_t m_style_main;
    lv_style_t m_style_indicator;
    lv_timer_t* m_refresh_timer{nullptr};

    OSD_Auto_Search_Channel_List_CB_t m_callback;
    Satellite m_satellite;

    //std::shared_ptr<Event_List_Update> m_lineup_build_event;
    void create_services_table();
    void create_progress_bar();
    void save_config_params();
    void start_blind_scan();
    void finish_blind_scan(std::vector<Transponder> tp_list);
    uint32_t calculate_frequency(uint32_t _frequency, aui_nim_freq_band _band,
                                                             aui_nim_polar _polarity);

    std::shared_ptr<Event_List_Update> m_channel_list_update_callback;
    void channel_list_update_callback(bool _is_done);
    void channel_list_partial_callback(size_t _transponder_seq, const std::vector<Service>& _services);

    void create_info_banner();
    lv_obj_t* m_info_banner{nullptr};
    lv_obj_t* m_info_label{nullptr};
    static constexpr auto info_banner_x = 230 - offset_x;
    static constexpr auto info_banner_y = 0;
    static constexpr auto info_banner_w = 910;
    static constexpr auto info_banner_h = 40;

    int32_t m_current_progress = 0;
    static void refresh_cb(lv_timer_t* tm);
    enum class Status {
        Idle,
        LNBf_Detection,
        To_LNBf_Detection_Success,
        LNBf_Detection_Success,
        To_LNBf_Detection_Fail,
        LNBf_Detection_Fail,
        To_Start_Lineup_Scanning,
        Transponder_Scanning,
        Lineup_Scanning,
        To_Finished_With_Success,
        Finished_With_Success,
        To_Finished_With_Fail,
        Finished_With_Fail,
        COUNT
    };
    Status m_status = Status::Idle;

    std::shared_ptr<Event_Autodetect_LNBf>
            m_post_event_autodetect_lnbf_start_event;
    std::shared_ptr<Event_Blind_Scan_Progress> m_post_event_blind_scan;
    std::weak_ptr<Event_Blind_Scan_Progress> m_event_weak;

 protected:
    static OSD_Auto_Search_Channel_List* s_instance;

    virtual bool handle_event_remote_control(
            const Event_Remote_Control& _event) override;

 public:
    OSD_Auto_Search_Channel_List(OSD* _parent);
    virtual ~OSD_Auto_Search_Channel_List();
    void auto_search_channel_list(OSD_Auto_Search_Channel_List_CB_t _callback,
                                                                Satellite _sat, bool _need_bgd);
    void refresh_progress();
};

}  // namespace mb
