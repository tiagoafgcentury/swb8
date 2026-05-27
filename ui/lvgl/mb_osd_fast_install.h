#pragma once

#include "mb_osd.h"
#include "mb_menu_data.h"

#include "common/mb_lineup.h"
#include "common/mb_config.h"
#include "mb_remote_control_handler.h"

namespace mb {

class OSD_Translate;

class OSD_Fast_Install : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> OSD_Fast_Install_CB_t;

private:
    static constexpr auto width = 1280;
    static constexpr auto height = 720;

    static constexpr auto width_main = 8 * width / 10;
    static constexpr auto height_main = 8 * height / 10;
    static constexpr auto width_event = 9 * width_main / 10;
    static constexpr auto height_event = 4 * height_main / 10;

    Signal_Data m_signal_data;

    lv_style_t m_quality_main;
    lv_style_t m_quality_indicator;
    lv_style_t m_quality_knob;
    lv_style_t m_strength_main;
    lv_style_t m_strength_indicator;
    lv_style_t m_strength_knob;
    lv_style_transition_dsc_t m_transition_dsc;

    lv_obj_t *m_quality_line { nullptr };

    lv_obj_t *m_strength_line { nullptr };

    lv_obj_t *m_bgd_main { nullptr };
    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_satellite_line { nullptr };
    lv_obj_t *m_service_line { nullptr };
    lv_obj_t *m_btn_back { nullptr };
    lv_obj_t *m_lbl_back { nullptr };
    lv_obj_t *m_btn_next { nullptr };
    lv_obj_t *m_lbl_next { nullptr };
    lv_obj_t *m_lbl_title { nullptr };
    lv_obj_t *m_lbl_message { nullptr };
    lv_timer_t *m_tmr_signal_data { nullptr };

    uint8_t m_selected_item = 0;

    void set_menu_selection();
    void move_right();
    void move_left();

    OSD_Fast_Install_CB_t m_callback;
    void create_strength_slider();
    void create_quality_slider();
    void hide_menu();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Fast_Install(OSD *_parent);
    virtual ~OSD_Fast_Install();

    void show_menu_fast_install(lv_obj_t *m_main_menu, OSD_Fast_Install_CB_t _callback);
    void process();
};

} // namespace mb
