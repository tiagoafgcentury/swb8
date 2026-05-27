#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_footer.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "mb_menu_data.h"

#include "common/mb_lineup.h"
#include <memory>
#include <chrono>
#include <vector>

namespace mb {

class OSD_Translate;
class OSD_Scheduled_Edit;
class OSD_Message_Box;

class OSD_Scheduled_List : public OSD, public OSD_Clock, public Remote_Control_Handler
{
private:
    typedef std::function<void(bool)> Scheduled_List_Callback_CB_t;

    std::unique_ptr<OSD_Scheduled_Edit> m_scheduled_edit;
    std::unique_ptr<OSD_Message_Box> m_message_box;

    static constexpr auto offset_x = 90;
    static constexpr auto offset_y = 90;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;
    static constexpr uint16_t MAX_SCHEDULE_LIST_VIEW = 9;

    static constexpr auto table_w = 1100;
    static constexpr auto table_h = 400;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bgd_table { nullptr };

    static constexpr auto max_num_cols = 6;
    static constexpr uint16_t col_widths[max_num_cols] = {353, 130, 130, 145, 195, 145};
    const int row_height = 40;

    uint16_t m_selected_item = 0;
    uint16_t m_start_pos = 0;

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

    Schedule m_headers =
    {
        .channel = std::string(tr(__Canal)),
        .date = std::string(tr(__Data)),
        .time = std::string(tr(__Horario)),
        .operation = std::string(tr(__Funcao)),
        .repeat = std::string(tr(__Repetir)),
        .status = "Status"
    };

    int m_selected_index = 0;
    std::vector<Schedule> m_entries;
    std::vector<Menu_Data> m_table_data;
    std::vector<ScheduleEntry> m_schedule;

    void reset_selection();
    void create_table();
    void create_line(lv_obj_t *_bgd, Schedule &_data, bool _visible, bool _save);
    void display_channel_list(uint16_t _start_idx, uint16_t _selected_item);
    void set_selection(uint16_t _list_to);
    void move_selection(uint16_t _from, uint16_t _to);
    void move_menu_up();
    void move_menu_down();
    void load_schedule_list();
    void load_schedule_list_callback(std::vector<ScheduleEntry> _schedule);
    void show_menu_schedule_edit_callback();
    void remove_item_schedule_list_callback(bool _result);
    void empty_schedule_list_callback();

    Scheduled_List_Callback_CB_t m_callback;
protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Scheduled_List(OSD *_parent);
    virtual ~OSD_Scheduled_List();

    virtual void show_scheduled_list(Scheduled_List_Callback_CB_t _callback);
};

} // namespace mb
