#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"

#include "tasks/mb_task_database.h"
#include "mb_osd_delete_satellite.h"
#include "mb_osd_edit_satellite.h"

#include <memory>
#include <array>

namespace mb {

class Fade_Canvas;

class OSD_Edit_Delete_Satellite :  public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> OSD_Edit_Delete_Satellite_CB_t;

private:
    std::unique_ptr<OSD_Edit_Satellite> m_editSatellite;
    std::unique_ptr<OSD_Delete_Satellite> m_deleteSatellite;

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

    lv_obj_t *m_edit_btn { nullptr };
    static constexpr auto edit_width = 220;
    static constexpr auto edit_heigth = 50;
    static constexpr auto edit_x = 409;
    static constexpr auto edit_y = 382;
    lv_obj_t *m_edit_label { nullptr };

    lv_obj_t *m_delete_btn { nullptr };
    static constexpr auto delete_width = edit_width;
    static constexpr auto delete_heigth = edit_heigth;
    static constexpr auto delete_x = 652;
    static constexpr auto delete_y = edit_y;
    lv_obj_t *m_delete_label { nullptr };

    lv_obj_t *m_return_btn { nullptr };
    static constexpr auto return_width = edit_width;
    static constexpr auto return_heigth = edit_heigth;
    static constexpr auto return_x = 530;
    static constexpr auto return_y = 570;
    lv_obj_t *m_return_label { nullptr };

    enum class Func_Active
    {
        Edit,
        Delete,
        COUNT
    };
    Func_Active m_selected_button = Func_Active::Edit;

    void select();

    OSD_Edit_Delete_Satellite_CB_t m_callback;
    Satellite m_satellite;

protected:
    static OSD_Edit_Delete_Satellite *s_instance;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Edit_Delete_Satellite(OSD *_parent);
    virtual ~OSD_Edit_Delete_Satellite();

    void edit_delete_satellite(OSD_Edit_Delete_Satellite_CB_t _callback, Satellite _sat);
};

} // namespace mb
