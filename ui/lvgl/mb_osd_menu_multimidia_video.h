#pragma once

#include "mb_osd.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task.h"
#include "mb_events.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "mb_osd_keys.h"
#include "hal/mb_sound.h"
#include "mb_osd_media_player.h"
#include "mb_osd_photo_player.h"
#include "mb_osd_subtitle_configuration.h"
#include "mb_osd_photo_configuration.h"

#include <filesystem>
#include <map>

namespace mb {

class OSD_Translate;
class OSD_Photo_Configuration;
class OSD_Video_Configuration;
class OSD_Subtitle_Configuration;

class OSD_Menu_Multimidia_Video : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Osd_Menu_Multimidia_Video_CB_t;
    Osd_Menu_Multimidia_Video_CB_t m_callback;

private:
    std::unique_ptr<OSD_Media_Player> m_play_video;
    std::unique_ptr<OSD_Photo_Player> m_play_photo;
    std::unique_ptr<OSD_Photo_Configuration> m_photo_configuration;
    std::unique_ptr<OSD_Subtitle_Configuration> m_subtitle_configuration;

    lv_obj_t *m_bgd                       { nullptr };
    lv_obj_t *m_main_window               { nullptr };
    lv_obj_t *m_footer                    { nullptr };
    lv_obj_t *m_cover_page                { nullptr };

    static constexpr auto main_window_x = 0;
    static constexpr auto main_window_y = 120;
    static constexpr auto width = DISPLAY_WIDTH - main_window_x;
    static constexpr auto height = DISPLAY_HEIGHT - main_window_y;

    lv_obj_t *m_main               { nullptr };
    static constexpr auto main_x = 0;
    static constexpr auto main_y = 120;
    static constexpr auto main_w = DISPLAY_WIDTH - main_x;
    static constexpr auto main_h = DISPLAY_HEIGHT - main_y;

    lv_obj_t *m_bar              { nullptr };
    static constexpr auto bar_x = 1170;
    static constexpr auto bar_y = 0;
    static constexpr auto bar_w = 20;
    static constexpr auto bar_h = 425;
    static constexpr auto SCROLL_PAGE_Y = 300;
    static constexpr auto maxVisibleItems = 15;

    lv_obj_t *m_message_box        { nullptr };
    lv_obj_t *m_message_text       { nullptr };
    static constexpr auto message_box_w = main_w / 2;
    static constexpr auto message_box_h = main_h / 4;

    std::vector<std::filesystem::path> m_usb_files = {};

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    // virtual void got_focus() override;
    static OSD_Menu_Multimidia_Video *s_instance;


#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

    static constexpr auto max_columns = 5;
    static constexpr auto x_offset = 60;
    static constexpr auto y_offset = 40;
    static constexpr auto box_h = 150;
    static constexpr auto box_w = 170;
    static constexpr auto icon_w = 68;
    static constexpr auto icon_h = 80;
    static constexpr auto x_spacing = box_w + 40;
    static constexpr auto y_spacing = box_h + 30;

    size_t m_selected = 0;
    std::vector<lv_obj_t *> m_icon_box = {};
    std::vector<lv_obj_t *> m_icon = {};
    std::vector<lv_obj_t *> m_text_box = {};

    std::string m_logo = LOGO_FILE_VIDEO_68X80;
    std::string m_logo_selected = LOGO_FILE_VIDEO_SELECTED_68X80;

    enum class Midia_Type
    {
        Audio,
        Video,
        Photo,
        Recording,
    };
    Midia_Type m_midia_type = Midia_Type::Video;

    int m_active_line = 0;
    int m_first_line = 0;
    int m_last_line = 0;
    int m_total_lines = 0;
    static constexpr auto m_display_lines = 3;

    void draw_video_files();
    void select();
    void unselect();
    void draw_nav_bar();
    void goto_previous_line();
    void goto_next_line();
    void reposition_box();

    bool is_video(const std::filesystem::path &_file);
    bool is_recording(const std::filesystem::path &_file);
    bool is_audio(const std::filesystem::path &_file);
    bool is_image(const std::filesystem::path &_file);
    void detect_midia_type();
    void open_settings_menu();

public:
    OSD_Menu_Multimidia_Video(OSD *_parent);
    virtual ~OSD_Menu_Multimidia_Video();
    virtual void show_menu_videos(Osd_Menu_Multimidia_Video_CB_t _callback, lv_obj_t *_bgd, std::vector<std::filesystem::path> _usb_files);

    void video_player_start();
    void video_player_callback();
    void photo_player_start();
    void photo_player_callback();
    //void pvr_player_start();
};

} // namespace mb
