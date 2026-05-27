#pragma once

#include "mb_events.h"
#include "mb_remote_control_handler.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include "mb_osd_fade_canvas.h"

#include "common/mb_globals.h"

#include <filesystem>
#include <memory>

namespace mb {

class Fade_Canvas;

class OSD_Photo_Player : public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> photo_player_callback_t;

private:
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto START_POS_X = 90;
    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 5;
    static constexpr auto BOTTOMAREA_Y1 = DISPLAY_HEIGHT - AREA_HEIGHT;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_screen { nullptr };
    std::unique_ptr<Fade_Canvas> m_bottom_rect;
    std::unique_ptr<Fade_Canvas> m_top_rect;
    lv_obj_t *m_bottom_mask { nullptr };
    lv_obj_t *m_top_mask { nullptr };
    lv_obj_t *m_filename { nullptr };
    lv_obj_t *m_logo_midiabox { nullptr };
    lv_obj_t *m_bottom { nullptr };

    lv_obj_t *m_next_logo { nullptr };
    lv_obj_t *m_play_stop_logo { nullptr };
    lv_obj_t *m_previous_logo { nullptr };

    static constexpr auto base_line = 90;
    static constexpr auto m_logo_spacing = 50;
    static constexpr auto next_x = START_POS_X;
    static constexpr auto play_stop_x = START_POS_X + m_logo_spacing;
    static constexpr auto previous_x = START_POS_X + 2 * m_logo_spacing;
    static constexpr auto m_logo_width = 34;
    static constexpr auto m_logo_height = 34;

    enum class Play_State
    {
        Play,
        Pause
    };
    Play_State m_play_state = Play_State::Pause;

    std::vector<std::filesystem::path> m_playlist = {};
    uint m_selected = 0;
    std::string m_current_path;
    lv_timer_t *m_timer { nullptr };
    lv_obj_t *m_image { nullptr };
    lv_obj_t *m_label_warning { nullptr };
    uint8_t m_slide_show_enabled;
    uint8_t m_display_time;
    uint8_t m_transition_effect;
    uint8_t m_aspect_ratio;


    photo_player_callback_t m_callback;

    void init_slide_show();
    void draw_image();
    void draw_footer();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    static void process_timer_callback(lv_timer_t *_tm);
    void process_timer();

public:
    OSD_Photo_Player(OSD *_parent);
    virtual ~OSD_Photo_Player();
    void show_photo_player(photo_player_callback_t _callback, lv_obj_t *_bgd, std::vector<std::filesystem::path> _playlist, uint _selected);
};

} // namespace mb
