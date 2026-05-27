#pragma once

#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"

#include <memory>
#include <map>

namespace mb {

class OSD_Closed_Caption: public OSD, public Remote_Control_Handler
{
private:
    CC_Type m_selected = CC_Type::Disabled;

    std::array<std::string_view, static_cast<size_t>(CC_Type::COUNT)> m_caption_names =
    {
        tr(__Desativado),
        "Digital",
        "Subtitle"
    };

    typedef std::function<void(void)> osd_closed_caption_cb;

    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT;

    // Variáveis desta classe
    int m_sleep_timer_value = 0;
    lv_obj_t    *m_main_box { nullptr };
    lv_obj_t    *m_lbl { nullptr };
    lv_timer_t *m_exit_timer{ nullptr };

#ifdef MBGUI_USE_RLOTTIE
    static constexpr auto ANI_MARGIN = 50;
#else
    static constexpr auto ANI_MARGIN = 0;
#endif

    static constexpr auto main_box_x = 865 - ANI_MARGIN;
    static constexpr auto main_box_y = 150;
    static constexpr auto main_box_w = 420 + ANI_MARGIN;
    static constexpr auto main_box_h = 50;

    uint8_t m_value = 0;

    Service *m_current_srv = nullptr;

    static void process_exit_cb(lv_timer_t *tm);
    void draw_main_box();
    void next();
    void update_captions();

    osd_closed_caption_cb m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Closed_Caption(OSD *_parent);
    virtual ~OSD_Closed_Caption();

    void show_closed_caption(osd_closed_caption_cb _callback);
};

} // namespace mb

