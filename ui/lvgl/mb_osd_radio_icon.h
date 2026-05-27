#pragma once

#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_osd.h"
#include "tasks/mb_task_cas.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"

#include "lvgl.h"
#include <functional>

namespace mb {

class OSD_Radio_Icon: public OSD, public OSD_Clock
{
public:
    typedef std::function<void(void)> Banner_CB_t;

private:
    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;
    static constexpr auto name_width = 400;
    static constexpr auto name_height = 40;

    //Menu_Options
    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_soundwave { nullptr };
    lv_obj_t *m_music_logo { nullptr };
    lv_obj_t *m_front_box { nullptr };
    lv_obj_t *m_back_box { nullptr };

    lv_obj_t *m_previous_radio_label { nullptr };
    lv_obj_t *m_current_radio_label { nullptr };
    lv_obj_t *m_next_radio_label { nullptr };

    lv_style_t m_back_style = {};
    lv_style_t m_front_style = {};

    void create_main_screen();;
    void update_radio_names(std::string_view _current_service_name, std::string_view _next_service_name, std::string_view _previous_service_name);

public:
    OSD_Radio_Icon(OSD *_parent);
    virtual ~OSD_Radio_Icon();

    void hide_radio_icon();
    void show_radio_icon(std::string_view _current_service_name, std::string_view _next_service_name, std::string_view _previous_service_name);
};

} // namespace mb
