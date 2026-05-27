#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "lvgl.h"
#include "mb_osd_sleep_timer.h"
#include "mb_osd_clock.h"
#include "mb_osd_keys.h"
#include "mb_osd_message_box.h"
#include "mb_osd_menu_plus_record.h"
#include "mb_osd_menu_plus_sleep.h"
#include "mb_osd_menu_plus_timeshift.h"

#include <memory>
#include <functional>

namespace mb {

class Fade_Canvas;
class OSD_Message_Box;

class OSD_Menu_Plus: public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Plus_CB_t;

private:
    Plus_CB_t m_callback;
    std::unique_ptr<OSD_Message_Box> m_messageBox;
    std::unique_ptr<OSD_Menu_Plus_Record> m_menuPlusRecord;
    std::unique_ptr<OSD_Menu_Plus_Sleep> m_menu_plus_sleep;
    std::unique_ptr<OSD_Menu_Plus_Timeshift> m_menuPlusTimeshift;

    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;

    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_logo_century { nullptr };
    lv_obj_t *m_bottom_mask { nullptr };
    lv_obj_t *m_ch_name { nullptr };
    lv_obj_t *m_ch_info { nullptr };

    lv_obj_t *m_box_main { nullptr };
    lv_obj_t *m_pvr_rec_logo { nullptr };

    const Service *m_srv_atual;
    std::unique_ptr<Fade_Canvas> m_bottom_rect;
    std::unique_ptr<Fade_Canvas> m_top_rect;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 0;
    static constexpr auto button_x = 287;
    static constexpr auto button_y = 50;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = button_w + 23; // 530 - 287

    enum class FunctionOption
    {
        Record,
        Timeshift,
        Sleep,
        COUNT
    };
    FunctionOption m_option = FunctionOption::Record;

    struct Function_Option_Text
    {
        FunctionOption index;
        std::string_view text;
    };

    std::array<Function_Option_Text, (int)FunctionOption::COUNT> m_options =
    {
        {
            { FunctionOption::Record, tr(__Gravar) },
            { FunctionOption::Timeshift, "Timeshift" },
            { FunctionOption::Sleep, "Sleep" }
        }
    };

    std::string mount_point = "";
    bool m_auto_exit = false;

    typedef std::chrono::system_clock::time_point Time_Point;

    void pvr_screen_base_frame(const Service *srv);
    void show_message_box();
    void show_record_screen(std::chrono::time_point<std::chrono::system_clock> _time_to_end);
    void show_sleep_screen();
    void show_timeshift_screen();
    bool check_usb_mounted();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    void message_box_callback(bool _ok);
    void record_menu_callback();
    void timeshift_menu_callback();
    void sleep_menu_callback();

public:
    OSD_Menu_Plus(OSD *_parent);
    virtual ~OSD_Menu_Plus();

    void show_menu_plus(const Service *_srv, Plus_CB_t _callback, bool _call_record_program, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
};

} // namespace mb
