#pragma once

#include "mb_events.h"
#include "mb_remote_control_handler.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"

#include "common/mb_globals.h"

#include <memory>
#include <filesystem>

namespace mb {

class Fade_Canvas;

class OSD_Media_Player : public OSD, public OSD_Clock, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Osd_Media_Player_CB_t;

    enum class Player_Type
    {
        AUDIO,
        VIDEO,
        TS
    };

private:
    static constexpr auto AREA_WIDTH = DISPLAY_WIDTH;
    static constexpr auto AREA_HEIGHT = DISPLAY_HEIGHT / 4;
    static constexpr auto AREA_X = 0;
    static constexpr auto TOPAREA_Y1 = 0;
    static constexpr auto BOTTOMAREA_Y1 = 3 * DISPLAY_HEIGHT / 4;
    static constexpr auto START_POS_X = 90;

    Player_Type m_player_type;

    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_logo_century { nullptr };
    lv_obj_t *m_filename { nullptr };
    lv_obj_t *m_filename_previous { nullptr };
    lv_obj_t *m_filename_next { nullptr };
    lv_obj_t *m_file_time { nullptr };
    lv_obj_t *m_file_time_end { nullptr };
    lv_obj_t *m_footer { nullptr };

    std::unique_ptr<Fade_Canvas> m_bottom_rect;
    std::unique_ptr<Fade_Canvas> m_top_rect;

    lv_obj_t *m_back_box { nullptr };
    lv_obj_t *m_box_transp { nullptr };
    lv_obj_t *m_front_box { nullptr };
    lv_obj_t *m_button_box { nullptr };

    lv_obj_t *m_bottom_mask { nullptr };
    lv_obj_t *m_slider_obj { nullptr };
    lv_obj_t *m_fav_top { nullptr };
    lv_obj_t *m_current_footer { nullptr };
    lv_obj_t *m_next_footer { nullptr };
    lv_timer_t *m_hide_timer { nullptr };
    lv_timer_t *m_player_info_timer { nullptr };
#ifdef MBGUI_USE_RLOTTIE
    lv_obj_t *m_soundwave { nullptr };
    lv_obj_t *m_music_logo { nullptr };
#endif

    lv_style_t m_style_indicator = {};
    lv_style_t m_style_main = {};
    lv_style_t m_label_style = {};
    lv_style_t m_back_style = {};
    lv_style_t m_front_style = {};

    lv_style_transition_dsc_t m_transition_dsc = {};
    lv_anim_t m_animation_template = {};

    bool m_hide_screen_enabled = false;
    int  m_current_midia = 0;
    bool m_repeat_video = false;

    void show_horizontal_bar();
    void update_horizontal_bar(int _progress);
    void audio_player_frame();
    void resize_audio_player_screen(bool _hidden);
    void video_player_frame();
    lv_obj_t *create_filename_label(lv_obj_t *_bgd);
    void update_filename();
    void update_midia_informations();
    void start_hide_timer();
    void start_player_info_timer();
    void del_player_info_timer();

    void show_screen();
    static void hide_screen_cb(lv_timer_t *_tm);
    static void player_info_cb(lv_timer_t *_tm);
    void reset_menu_selection();
    void set_menu_selection(uint8_t _menu, bool _selected);
    void move_selection(uint8_t _from, uint8_t _to);

    void play();
    void pause();
    void resume();
    void next();
    void previous();
    void setRepeat(bool _state);
    void playRandom();
    void update_forward_icon();
    void reset_rewind_forward_icon();
    void reset_play_pause_icon();
    void forward();
    void update_rewind_icon();
    void rewind();
    void config();
    void midia_player_finish();

    void pvr_player_finish();

    enum class Btn_Play_State
    {
        PLAY,
        PAUSE,
        MAX_PLAY_PAUSE_STATE
    };
    Btn_Play_State m_btn_play_state = Btn_Play_State::PLAY;

    uint16_t m_forward_speed = 0;
    uint16_t m_rewind_speed = 0;

    enum class Player_Functions
    {
        PLAYER_PREVIOUS = 0,
        PLAYER_REWIND,
        PLAYER_PLAY_PAUSE,
        PLAYER_STOP,
        PLAYER_FORWARD,
        PLAYER_NEXT,
        //PLAYER_REPEAT,
        //PLAYER_RANDOM,
        //PLAYER_CONFIG,
        MAX_PLAYER_FUNCTIONS,
    };

    uint16_t m_selected_item = static_cast<uint16_t>(Player_Functions::PLAYER_PLAY_PAUSE);
    std::array<lv_obj_t *, static_cast<uint8_t>(Player_Functions::MAX_PLAYER_FUNCTIONS)> m_mp_functions;
    std::array<lv_obj_t *, static_cast<uint8_t>(Player_Functions::MAX_PLAYER_FUNCTIONS)> m_mp_functions_sel;
    std::array<lv_obj_t *, static_cast<uint8_t>(Btn_Play_State::MAX_PLAY_PAUSE_STATE)> m_mp_play_pause;
    std::array<lv_obj_t *, static_cast<uint8_t>(Btn_Play_State::MAX_PLAY_PAUSE_STATE)> m_mp_play_pause_sel;

    lv_obj_t *m_play_ptr { nullptr };
    lv_obj_t *m_play_sel_ptr { nullptr };

    void update_play_pause_btn(uint8_t _menu, bool _selected);
    void reset_play_pause_btn();

    enum class Speed_State
    {
        NORMAL,
        SPEED_2X,
        SPEED_4X,
        SPEED_8X,
        SPEED_16X,
        SPEED_24X,
        MAX_SPEED_STATE
    };
    Speed_State m_foward_speed_state = Speed_State::NORMAL;
    Speed_State m_rewind_speed_state = Speed_State::NORMAL;
    std::array<lv_obj_t *, static_cast<uint8_t>(Speed_State::MAX_SPEED_STATE)> m_mp_ff_speed;
    std::array<lv_obj_t *, static_cast<uint8_t>(Speed_State::MAX_SPEED_STATE)> m_mp_fr_speed;
    lv_obj_t *m_rewind_ptr { nullptr };
    lv_obj_t *m_forward_ptr { nullptr };

    lv_obj_t *m_label_speed = nullptr;
    void update_label_speed(Speed_State _speed);

    void move_right();
    void move_left();

    Osd_Media_Player_CB_t m_callback = {};

    long int m_current_time = {};
    long int m_total_time = {};

    std::vector<std::filesystem::path> m_playlist = {};

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Media_Player(OSD *_parent);
    virtual ~OSD_Media_Player();
    void show_media_player(Osd_Media_Player_CB_t _callback, Player_Type _type, std::vector<std::filesystem::path> _playlist, uint _selected);

};

} // namespace mb
