#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_events.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"

#include <memory>

namespace mb {

class Fade_Canvas;

class Channel_Info : public OSD, public OSD_Clock
{
public:
    typedef std::function<void(void)> Channel_Info_CB_t;

    struct Event_Info_t
    {
        std::string name;
        std::string number;
        std::string program;
        Parental_Control parental_rating;
        std::string start_str;
        std::string end_str;
        time_t start = 0;
        double duration = 0;
    };

    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto NULL_TIME = "--:--";
    static constexpr auto NULL_CHANNEL = "N/A";
    static constexpr auto START_POS_X = 90;

private:
    lv_obj_t *m_main_screen { nullptr };

    lv_obj_t *m_ch_name { nullptr };
    lv_obj_t *m_ch_info { nullptr };
    lv_obj_t *m_ch_date { nullptr };
    lv_obj_t *m_ch_hour { nullptr };
    lv_obj_t *m_ch_current { nullptr };
    lv_obj_t *m_ch_current_time { nullptr };
    lv_obj_t *m_ch_next_time { nullptr };
    lv_obj_t *m_ch_next { nullptr };

    lv_obj_t *m_footer { nullptr };


    std::unique_ptr<Fade_Canvas> m_bottom_rect;
    std::unique_ptr<Fade_Canvas> m_top_rect;

    lv_obj_t *m_bottom_mask { nullptr };
    lv_obj_t *m_slider_obj { nullptr };
    lv_obj_t *m_fav_top { nullptr };
    lv_obj_t *m_current_footer { nullptr };
    lv_obj_t *m_next_footer { nullptr };
    lv_timer_t *m_hide_timer { nullptr };

    lv_style_t m_style_indic_pr;
    lv_style_t m_style_indicator;
    lv_style_t m_style_main;
    lv_style_t m_style_top;
    lv_style_t m_label_style;

    lv_style_transition_dsc_t m_transition_dsc;

    lv_anim_t m_animation_template;

    void draw_footer(const Event_Info_t &_current_program, const Event_Info_t &_next_program);
    static void delete_channel_info_cb(lv_timer_t *tm);
    void show_horizontal_bar();
    void update_horizontal_bar(int progress);
    void channel_info_frame();

    Channel_Info(OSD *_parent);
    static std::unique_ptr<Channel_Info> s_instance;
    void delete_channel_info();
    void start_hide_timer();

public:
    static Channel_Info *get_instance(OSD *_parent);
    virtual ~Channel_Info();

    void show_channel_info(const Service *_srv);
    static void hide_channel_info();
    static void reset_hide_timer();
    static bool has_menuInfo();
};

} // namespace mb

