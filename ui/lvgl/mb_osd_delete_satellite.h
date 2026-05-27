#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_remote_control_handler.h"

#include "tasks/mb_task_database.h"

#include <array>

namespace mb {

class Fade_Canvas;

class OSD_Delete_Satellite :  public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> OSD_Delete_Satellite_CB_t;

private:
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT;

    lv_obj_t *m_mainscreen { nullptr };

    lv_obj_t *m_logo { nullptr };
    static constexpr auto logo_width = 50;
    static constexpr auto logo_heigth = 50;
    static constexpr auto logo_x = 90;
    static constexpr auto logo_y = 42;

    lv_obj_t *m_breadcrumb_box { nullptr };
    static constexpr auto breadcrumb_width = 663;
    static constexpr auto breadcrumb_heigth = 33;
    static constexpr auto breadcrumb_x = 163;
    static constexpr auto breadcrumb_y = 51;
    lv_obj_t *m_breadcrumb_label { nullptr };

    lv_obj_t *m_item_box { nullptr };
    lv_obj_t *m_item_label { nullptr };

    lv_obj_t *m_title_box { nullptr };
    static constexpr auto title_width = 400;
    static constexpr auto title_heigth = 53;
    static constexpr auto title_x = (width - title_width) / 2;
    static constexpr auto title_y = 147;
    lv_obj_t *m_title_label { nullptr };

    lv_obj_t *m_subtitle_box { nullptr };
    static constexpr auto subtitle_width = 700;
    static constexpr auto subtitle_heigth = 53;
    static constexpr auto subtitle_x = (width - subtitle_width) / 2;
    static constexpr auto subtitle_y = 207;
    lv_obj_t *m_subtitle_label { nullptr };

    lv_obj_t *m_yes_btn { nullptr };
    static constexpr auto yes_width = 220;
    static constexpr auto yes_heigth = 50;
    static constexpr auto yes_x = 409;
    static constexpr auto yes_y = 382;
    lv_obj_t *m_yes_label { nullptr };

    lv_obj_t *m_no_btn { nullptr };
    static constexpr auto no_width = yes_width;
    static constexpr auto no_heigth = yes_heigth;
    static constexpr auto no_x = 652;
    static constexpr auto no_y = yes_y;
    lv_obj_t *m_no_label { nullptr };

    enum class Func_Active
    {
        Yes,
        No,
        COUNT
    };
    Func_Active m_selected_button = Func_Active::No;

    void select();

    OSD_Delete_Satellite_CB_t m_callback;
    Satellite m_satellite;

protected:
    static OSD_Delete_Satellite *s_instance;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Delete_Satellite(OSD *_parent);
    virtual ~OSD_Delete_Satellite();
    void delete_satellite(OSD_Delete_Satellite_CB_t _callback, Satellite _sat);
};

} // namespace mb

