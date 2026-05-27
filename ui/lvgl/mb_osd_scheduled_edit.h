#pragma once

#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_footer.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "mb_menu_data.h"
#include "mb_osd_keys.h"

#include "common/mb_lineup.h"
#include <memory>
#include <chrono>
#include <vector>

namespace mb {

class OSD_Translate;

class OSD_Scheduled_Edit : public OSD, public OSD_Clock, public Remote_Control_Handler
{
private:
    typedef std::function<void(bool)> Scheduled_Edit_CB_t;

    static constexpr auto offset = 124;
    static constexpr auto offset_x = 90;
    static constexpr auto offset_y = 120;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;
    static constexpr uint16_t MAX_SCHEDULE_LIST_VIEW = 9;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bgd_option { nullptr };
    lv_obj_t *m_bgd_bottom { nullptr };
    lv_obj_t *m_footer { nullptr };

    lv_obj_t *m_btn_save { nullptr };
    static constexpr auto btn_save_width = 220;
    static constexpr auto btn_save_heigth = 50;
    static constexpr auto btn_save_x = (width - btn_save_width) / 2;
    static constexpr auto btn_save_y = 570 - offset;
    lv_obj_t *m_btn_save_label { nullptr };

    enum class Func_Active
    {
        Channel,
        Date,
        Time,
        Operation,
        Repeat,
        Status,
        Save,
        COUNT
    };
    Func_Active m_selected_button = Func_Active::Channel;

    std::array<lv_obj_t *, static_cast<size_t>(Func_Active::COUNT)> m_titles;
    std::array<lv_obj_t *, static_cast<size_t>(Func_Active::COUNT)> m_labels;
    std::array<std::string_view, static_cast<size_t>(Func_Active::COUNT)> m_title_names =
    {
        tr(__Canal),
        tr(__Data),
        tr(__Horario),
        tr(__Funcao),
        tr(__Repetir),
        "Status"
    };

    static constexpr auto title_width = 165;
    static constexpr auto title_heigth = 50;
    static constexpr auto title_x = 90;
    static constexpr auto title_y = 5;
    static constexpr auto title_spacing = 75;

    MB_OSD_Keys m_keys_main;
    static constexpr auto button_width = 260;
    static constexpr auto button_heigth = 50;
    static constexpr auto button_x = 290;
    static constexpr auto button_y = 5;
    static constexpr auto button_spacing = title_spacing;

    lv_obj_t *m_cover_area { nullptr };
    static constexpr auto cover_w = 565;
    static constexpr auto cover_h = 460;
    static constexpr auto cover_x = 0;
    static constexpr auto cover_y = 0;

    lv_area_t m_area = {};

    lv_obj_t *m_line { nullptr };
    static constexpr auto line_width = 3;
    static constexpr auto line_heigth = 460;
    static constexpr auto line_x = 270;
    static constexpr auto line_y = 0;

    MB_OSD_Keys m_keys_options;
    static constexpr auto btn_x = 580;
    static constexpr auto btn_y = 270;
    static constexpr auto btn_w = 190;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_spacing = 204;

    struct Schedule
    {
        uint16_t id { 0 };
        std::string channel;
        std::string date;
        std::string time;
        std::string operation;
        std::string repeat;
        std::string status;
    };

    Schedule m_entry_text = {};
    ScheduleEntry m_entry = {};

    std::vector<Lineup::Channel_Info> m_channel_data;
    std::vector<Menu_Data> m_list_data;
    uint16_t m_selected_item = 0;
    uint16_t m_start_pos = 0;
    bool m_enable_sched_screen = false;

    static constexpr uint16_t MAX_CHANNEL_LIST_VIEW = 9;
    static constexpr uint16_t LIST_SPACING = 50;

    lv_obj_t *m_textarea { nullptr };
    lv_obj_t *m_wrong_date { nullptr };
    lv_timer_t *m_refresh_timer { nullptr };
    lv_style_t  m_style;

    enum class Mode_Active
    {
        Browsing,
        Editing_Date,
        Editing_Time,
    };
    Mode_Active m_mode_active = Mode_Active::Browsing;

    std::string m_final_date = {};
    std::string m_final_start_time = {};
    std::string m_final_end_time = {};

    bool m_edit_btn_time = false;

    void select();
    void unselect();
    void reset_sat_options();
    void draw_schedule_options(bool _create_buttons);
    void create_textarea(uint16_t _idx);
    void edit_schedule_functions();
    void fill_table();
    void draw_titles();
    void draw_buttons();

    void set_selection(uint16_t _to);
    void move_menu_up();
    void move_menu_down();
    void update_channel_table_list();
    void add_channel_list();
    void display_channel_list(uint16_t _start_idx, uint16_t _selected_item);
    void set_current_channel_list_view();
    void process_date_time(Remote_Control_Key _key);
    static void refresh_cb(lv_timer_t *_tm);
    void refresh_progress();

    Scheduled_Edit_CB_t m_callback;
protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Scheduled_Edit(OSD *_parent);
    virtual ~OSD_Scheduled_Edit();

    virtual void show_scheduled_edit(Scheduled_Edit_CB_t _callback, ScheduleEntry _s_entry);
};

} // namespace mb
