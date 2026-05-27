#pragma once
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "hal/mb_remote_control_keys.h"
#include <memory>

namespace mb {

class OSD_Volume: public OSD
{
public:
    typedef std::function<void(bool)> Volume_CB_t;

private:
    lv_timer_t  *m_tmr_volume_on { nullptr };

    lv_style_t  m_style_volume;
    lv_style_t  m_style_prog_bar;

    lv_obj_t    *m_bgd_main { nullptr };
    lv_obj_t    *m_bgd_volume { nullptr };
    lv_obj_t    *m_bgd_volume_off { nullptr };
    lv_obj_t    *m_bgd_volume_off_grey { nullptr };
    lv_obj_t    *m_lbl_volume { nullptr };
    lv_obj_t    *m_prgbar_volume { nullptr };
    lv_obj_t    *m_logo_volume { nullptr };
    lv_obj_t    *m_logo_volume_off { nullptr };
    lv_obj_t    *m_logo_volume_off_sel { nullptr };
    lv_obj_t    *m_lbl_volume_off_sel { nullptr };
    lv_obj_t    *m_img_volume_off { nullptr };

#ifdef MBGUI_ANIMATION
    lv_anim_t m_anim {};
    static void anim_delete_callback(lv_anim_t *_anim);
    void show_volume_bar();
    void hide_volume_bar();

    static constexpr auto START_POS_X = -45;
    static constexpr auto END_POS_X = 90;
#else
    static constexpr auto START_POS_X = 90;
#endif

    static constexpr auto vol_start_x = START_POS_X;
    static constexpr auto vol_start_y = 148;
    static constexpr auto vol_start_w = 50;
    static constexpr auto vol_start_h = 424;

    static constexpr auto mute_start_x = 915;
    static constexpr auto mute_start_y = 148;
    static constexpr auto mute_start_w = 380;
    static constexpr auto mute_start_h = 50;

    static void update_volume_cb(lv_timer_t *_tm);
    void create_volume(uint16_t _volume);
    Volume_CB_t m_callback;
    bool m_muted = false;

public:
    OSD_Volume();
    virtual ~OSD_Volume();
    void delete_volume();
    void set_volume(uint16_t _volume, bool _mute, Volume_CB_t _callback);
};

}
