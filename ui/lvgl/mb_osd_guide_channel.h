#pragma once

#include "mb_osd.h"

#include "common/mb_lineup.h"
#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_breadcrumb.h"
#include "tasks/mb_task_eit_events.h"

#include <array>

namespace mb {

class OSD_Translate;
class OSD_Guide_Channel_Info;
class OSD_Scheduled_Edit;

class OSD_Guide_Channel : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> OSD_Guide_Channel_CB_t;

private:
    std::unique_ptr<OSD_Guide_Channel_Info> m_osd_guide_channel_info;
    std::unique_ptr<OSD_Scheduled_Edit> m_osd_scheduled_edit;

    static constexpr auto offset_x = 90;
    static constexpr auto offset_y = 120;
    static constexpr auto width = 890;//DISPLAY_WIDTH;
    static constexpr auto height = 180;
    static constexpr uint16_t MAX_CHANNEL_LIST_VIEW = 7;
    static constexpr uint16_t MENU_SPACING = 73;
    static constexpr uint16_t MENU_SUB_MENU_SPACING = 50;
    static constexpr uint16_t m_instructions_offset = 400;

    static constexpr uint16_t MENU_MAX_TEXT_WIDTH = 800;
    static constexpr uint16_t MENU_PARENTAL_RATING_X = 90;
    static constexpr uint16_t MENU_PARENTAL_RATING_Y = 78;

    static constexpr auto m_center_box_x = 380;
    static constexpr auto m_center_box_y = 0;
    static constexpr auto m_center_box_w = 495;
    static constexpr auto m_center_box_h = 500;

    static constexpr auto m_box_info_x = 0;
    static constexpr auto m_box_info_y = 44;
    static constexpr auto m_box_info_spacing = 26;
    static constexpr auto m_minimum_recording_time = 5;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_box_info { nullptr };
    lv_obj_t *m_box_guide { nullptr };
    lv_obj_t *m_center_box { nullptr };
    lv_obj_t *m_main_sub_menu { nullptr };
    lv_obj_t *m_radio_preview { nullptr };
    lv_style_t m_front_style = {};

    lv_obj_t *m_channel_name { nullptr };
    lv_obj_t *m_channel_desc { nullptr };
    lv_obj_t *m_program { nullptr };
    lv_obj_t *m_subtitle { nullptr };
    lv_obj_t *m_class_indic { nullptr };
    lv_obj_t *m_img_fav { nullptr };
    lv_obj_t *m_error_message { nullptr };

    lv_obj_t *m_logo_midia { nullptr };

    char m_channel_number[10] = {0};
    Service *m_atual_srv { nullptr };
    static constexpr uint INVALID_CHANNEL = std::numeric_limits<uint>::max();
    uint m_current_channel { INVALID_CHANNEL };
    ScheduleEntry m_schedule_entry;

    struct
    {
        std::string_view channel_name;
        Viewer_Channel_t channel_number {};
        std::string_view channel_category;
        std::string_view channel_program;
        std::string_view program_description;
        uint8_t parental_rating;
        bool favorite;
    } m_channel_text_info;

    int m_selected_item = 0;
    int m_first_visible_item = 0;
    int m_last_visible_item = 0;
    bool m_init_selected_item = true;

    lv_obj_t *m_tm_dt_line { nullptr };
    lv_obj_t *m_tm_dt_box { nullptr };
    static constexpr auto m_tm_dt_box_x = 90;
    static constexpr auto m_tm_dt_box_y = 0;
    static constexpr auto m_tm_dt_box_w = 207;
    static constexpr auto m_tm_dt_box_h = 40;
    lv_obj_t *m_tm_dt_clock { nullptr };
    lv_obj_t *m_left_arrow { nullptr };
    lv_obj_t *m_right_arrow { nullptr };
    lv_timer_t *m_timer_arrow { nullptr };
    lv_timer_t *m_tmr_channel_preview { nullptr };

    UTC_MJD m_start_time;
    UTC_MJD m_current_time;

    lv_obj_t *m_tm_box { nullptr };
    static constexpr auto m_tm_box_x = 298;
    static constexpr auto m_tm_box_y = 0;
    static constexpr auto m_tm_box_w = 985;
    static constexpr auto m_tm_box_h = 40;

    static constexpr auto NULL_CHANNEL = "N/A";
    static constexpr auto NULL_TIME = 0;
    struct Event_Info_t
    {
        std::string program;
        UTC_MJD start;
        std::chrono::minutes duration { 0 };
    };

    struct Channel_Info_t
    {
        std::string_view name;
        Viewer_Channel_t viewer_channel;
        int service_id;
        std::vector<Event_Info_t> events;
    };

    std::vector<Channel_Info_t> m_all_channel_info = {};

    static constexpr auto EVENT_LINES = 7;
    static constexpr auto EVENT_COLUMNS = 10;
    static constexpr auto EVENT_CELL_COUNT = EVENT_LINES * EVENT_COLUMNS;
    std::array<lv_obj_t *, EVENT_CELL_COUNT> m_event_boxes = {};
    std::vector<lv_obj_t *> m_channel_boxes;
    static constexpr auto m_channel_box_x = 90;
    static constexpr auto m_channel_box_y = 42;
    static constexpr auto m_channel_box_w = 207;
    static constexpr auto m_channel_box_h = 40;
    static constexpr auto m_channel_box_s = 42;

    void show_menu_info_callback();
    void draw_radio_box();
    void draw_timedate_box();
    void draw_timedate_clock(UTC_MJD current_time);
    void change_channel();
    void update_channel_info(bool save_channel_info, Service *_srv);
    void print_channel_info();
    void move_right();
    void move_left();
    void draw_arrow_right(const char *text);
    void draw_arrow_left(const char *text);
    void init_arrow_timer();
    void load_all_epg();
    void load_epg();
    void draw_channel_list();
    void move_menu_up();
    void move_menu_down();
    void move_menu_right();
    void move_menu_left();
    void draw_event_list();
    bool is_valid_event(Event_Info_t &event);
    lv_obj_t *draw_event_box(Event_Info_t event, int line = 0);
    void process();

    UTC_MJD next_30_minutes(UTC_MJD _ct) const
    {
        using namespace std::chrono;
        // Calculate the number of minutes to add
        long long minutes = _ct.minute();
        long long remainder = minutes % 30;
        long long minutes_to_add = 30 - remainder;
        minutes_to_add = (remainder == 0) ? 30 : minutes_to_add;
        // Add the seconds to the current time using std::chrono::seconds
        UTC_MJD next_30_min_multiple = _ct + std::chrono::seconds(minutes_to_add * 60);
        return next_30_min_multiple;
    }

    int minute_to_x(int minute_offset) const
    {
        return (minute_offset * SLOT_WIDTH) / TIME_SLOT_MINUTES;
    }

    void schedule_event();
    void schedule_event_callback();

    void print_event_info(const Event_Info_t &_event, int _line)
    {
#ifndef NDEBUG
        if (_line != 0)
        {
            DEBUG_MSG(OSD, DEBUG, "[" << std::dec << _line << "]");
        }

        DEBUG_MSG(OSD, DEBUG, " - " << _event.program << ", Start: " << _event.start.time_to_str() << ", Duration: " << std::dec << _event.duration.count() << " minutes\n");
#endif
    }

    OSD_Channels_List_Type m_channel_list_type;
    OSD_Guide_Channel_CB_t m_callback;

    static constexpr auto NUM_VISIBLE_CHANNELS = 7;
    static constexpr auto TIME_SLOT_MINUTES = 30;
    static constexpr auto TOTAL_VISIBLE_MINUTES = 240;  // 4 hours
    static constexpr auto MIN_PROGRAM_DURATION  = 5;     // in minutes
    static constexpr auto CHANNEL_HEIGHT = 50;
    static constexpr auto SLOT_WIDTH = 200;  // width for 30 minutes
    static constexpr auto VISIBLE_MINUTES = 150; // 2h30 = 150 min
    static constexpr auto LEFT_PADDING = 100;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Guide_Channel(OSD *_parent);
    virtual ~OSD_Guide_Channel();

    virtual void show_menu_guide_channel(OSD_Guide_Channel_CB_t _callback, OSD_Channels_List_Type _channel_list_type);
    void change_channel(bool _save_channel_info);
};

} // namespace mb
