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
#include "mb_osd_menu_multimidia_video.h"

namespace mb {

class OSD_Translate;
class OSD_Menu_Multimidia_Video;

class OSD_Menu_Multimidia : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Osd_Menu_Multimidia_CB_t;
    Osd_Menu_Multimidia_CB_t m_callback;

private:
    std::unique_ptr<OSD_Menu_Multimidia_Video> m_osd_menu_video;

    // Main Menu
    lv_obj_t *m_bgd                { nullptr };
    lv_obj_t *m_main               { nullptr };
    static constexpr auto main_x = 150;
    static constexpr auto main_y = 0;
    static constexpr auto main_w = DISPLAY_WIDTH - main_x;
    static constexpr auto main_h = DISPLAY_HEIGHT - main_y;

    lv_obj_t *m_message_box        { nullptr };
    lv_obj_t *m_message_text       { nullptr };
    static constexpr auto message_box_w = main_w / 2;
    static constexpr auto message_box_h = main_h / 4;
    lv_timer_t  *m_timer        = { nullptr };
    lv_timer_t  *m_usb_check_timer        = { nullptr };

    enum class Status
    {
        Detection,
        Detected,
        Failed,
        Listing,
        Waiting,
        Finished,
        Empty,
        Device_not_found,
        Leaving,
        COUNT
    };
    Status m_status = Status::Detection;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto btn_x = 10;
    static constexpr auto btn_y = 198;
    static constexpr auto btn_w = 220;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_s = 271 - 198;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 0;

    enum class MultimediaOption
    {
        Recordings,
        Videos,
        Photos,
        Music,
        COUNT
    };
    MultimediaOption m_option = MultimediaOption::Recordings;

    struct MultimediaOptionText
    {
        MultimediaOption index;
        std::string_view text;
    };

    std::array<MultimediaOptionText, (int)MultimediaOption::COUNT> m_options =
    {
        {
            { MultimediaOption::Recordings, tr(__Gravacoes) },
            { MultimediaOption::Videos, tr(__Videos) },
            { MultimediaOption::Photos, tr(__Fotos) },
            { MultimediaOption::Music, tr(__Musicas) }
        }
    };

    void disable_keys();
    void enable_keys();

    std::vector<std::filesystem::path> m_video_files = {};
    std::vector<std::filesystem::path> m_photo_files = {};
    std::vector<std::filesystem::path> m_music_files = {};
    std::vector<std::filesystem::path> m_ts_files = {};
    void list_usb_files();
    void fill_message_box(std::string_view _text);
    void detect_flash_disk();

    static void process_timer_cb(lv_timer_t *_tm);
    void process_timer();

    static void verify_usb_mounted_timer_cb(lv_timer_t *_tm);
    void verify_usb_mounted_timer();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    //virtual void handle_event_usb_plug_event(Event_USB_Plug _event) override;

    static OSD_Menu_Multimidia *s_instance;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Menu_Multimidia(OSD *_parent);
    virtual ~OSD_Menu_Multimidia();
    virtual void show_menu_multimidia(Osd_Menu_Multimidia_CB_t _callback, lv_obj_t *_bgd);



    void menu_callback();
    void enter_menu(std::vector<std::filesystem::path> _usb_files);

    void handle_event_usb_plug_event(Event_USB_Plug _event);
};

} // namespace mb
