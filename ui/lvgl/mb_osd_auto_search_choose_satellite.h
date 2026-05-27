#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task_database.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_terms_of_use.h"
#include "mb_osd_auto_search_channel_list.h"

#include <memory>
#include <array>

namespace mb {

class OSD_Auto_Search_Choose_Satellite :  public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(void)> Auto_Search_Choose_Satellite_CB_t;

private:
    std::unique_ptr<OSD_Terms_of_Use> m_osd_terms_of_use;
    std::unique_ptr<OSD_Auto_Search_Channel_List> m_osd_as_channel_list;

    static constexpr auto start_y = 147;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - 147;

    lv_obj_t *m_mainscreen { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };

    lv_obj_t *m_title_box { nullptr };
    static constexpr auto title_width = 400;
    static constexpr auto title_heigth = 53;
    static constexpr auto title_x = (width - title_width) / 2;
    static constexpr auto title_y = 0;
    lv_obj_t *m_title_label { nullptr };

    lv_obj_t *m_subtitle_box { nullptr };
    static constexpr auto subtitle_width = 700;
    static constexpr auto subtitle_heigth = 53;
    static constexpr auto subtitle_x = (width - subtitle_width) / 2;
    static constexpr auto subtitle_y = 207 - start_y;
    lv_obj_t *m_subtitle_label { nullptr };

    static constexpr auto satellite_num = 12;
    static constexpr auto satellite_width = 220;
    static constexpr auto satellite_heigth = 50;
    std::array<lv_obj_t *, satellite_num> m_satellite_box;
    std::array<lv_obj_t *, satellite_num> m_satellite_label;

    lv_obj_t *m_button_box { nullptr };
    static constexpr auto button_width = 220;
    static constexpr auto button_heigth = 50;
    static constexpr auto button_x = 530;
    static constexpr auto button_y = 570 - start_y;
    lv_obj_t *m_button_label { nullptr };

    enum class Selected_Area
    {
        Satellites,
        Button,
        COUNT
    };
    Selected_Area m_selected_area = Selected_Area::Satellites;

    unsigned int m_selected_satellite { 0 };
    std::vector<Satellite> m_satellites;

    static constexpr auto instruction_w = 1020;
    static constexpr auto instruction_h = 27;
    static constexpr auto instruction_x = 140;
    static constexpr auto instruction_y = 641 - start_y;

    Auto_Search_Choose_Satellite_CB_t m_callback;

    // Seleciona satélite
    void select();
    void unselect();
    void select_satellite();

    void show_menu_terms_of_use_callback(bool _result);
    void auto_search_channel_list_callback();

protected:
    static OSD_Auto_Search_Choose_Satellite *s_instance;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Auto_Search_Choose_Satellite(OSD *_parent);
    virtual ~OSD_Auto_Search_Choose_Satellite();

    void show_select_satellite(Auto_Search_Choose_Satellite_CB_t _callback);
    void process_satellite_list();
};

} // namespace mb
