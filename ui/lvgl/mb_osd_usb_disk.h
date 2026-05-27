#pragma once

#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_events.h"

#include <memory>
#include <map>

namespace mb {

class OSD_USB_Disk: public OSD
{
private:
    enum class USB_Func
    {
        Inserted,
        Removed,
        COUNT
    };
    USB_Func m_selected = USB_Func::Inserted;

    std::array<std::string_view, static_cast<size_t>(USB_Func::COUNT)> m_usb_names =
    {
        tr(__USB_inserido),
        tr(__USB_removido)
    };

    typedef std::function<void(void)> USB_Disk_CB_t;

    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT;

    // Variáveis desta classe
    int m_sleep_timer_value = 0;
    lv_obj_t    *m_main_box { nullptr };
    lv_obj_t    *m_lbl { nullptr };
    lv_timer_t *m_exit_timer{ nullptr };

    static constexpr auto main_box_x = 925;
    static constexpr auto main_box_y = 150;
    static constexpr auto main_box_w = 360;
    static constexpr auto main_box_h = 50;

    static void process_exit_cb(lv_timer_t *_tm);
    USB_Disk_CB_t m_callback;

public:
    OSD_USB_Disk(OSD *_parent);
    virtual ~OSD_USB_Disk();

    void show_usb_disk(USB_Disk_CB_t _callback, Event_USB_Plug _event);
};

} // namespace mb
