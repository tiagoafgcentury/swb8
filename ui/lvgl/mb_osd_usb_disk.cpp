#include <iostream>
#include <map>
#include <sstream>

#include "mb_osd_usb_disk.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "hal/mb_system.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "tasks/mb_task.h"

namespace mb {

OSD_USB_Disk::OSD_USB_Disk(OSD *_parent):
    OSD(_parent)
{
}

OSD_USB_Disk::~OSD_USB_Disk()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_USB_Disk::~OSD_USB_Disk\n");
    DELETE_OBJ(m_main_box);
    DELETE_TIMER(m_exit_timer);
}

void OSD_USB_Disk::show_usb_disk(USB_Disk_CB_t _callback, Event_USB_Plug _event)
{
    DEBUG_MSG(OSD, DEBUG,  "show_usb_disk\n");
    DELETE_OBJ(m_main_box);
    DELETE_TIMER(m_exit_timer);
    m_callback = _callback;
    m_main_box = create_rect(get_main_screen(OSD_Layer::USB_DEVICE_LAYER), main_box_x, main_box_y, main_box_w, main_box_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_box);
    lv_obj_set_style_bg_opa(m_main_box, OSD_TRANPARENCY_LIVE, 0);
    lv_obj_set_style_radius(m_main_box, 25, DEFAULT_SELECTOR);
#ifdef MBGUI_USE_RLOTTIE
    auto usb_img = lv_rlottie_create_from_file(m_main_box, 100, 100, ANIM_USB);
    lv_image_set_rotation(usb_img, 2700);
    lv_obj_align(usb_img, LV_ALIGN_LEFT_MID, 15, 0);
    lv_rlottie_set_play_mode(usb_img, LV_RLOTTIE_CTRL_LOOP);
#endif
    auto usb_lbl = set_label_text(m_main_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_label_set_text(usb_lbl, _event.type == Event_USB_Plug::Plugged_In ? tr(__USB_inserido).data() : tr(__USB_removido).data());
    lv_obj_align(usb_lbl, LV_ALIGN_LEFT_MID, 110, 0);
    m_exit_timer = lv_timer_create(process_exit_cb, 5000, this);
}

void OSD_USB_Disk::process_exit_cb(lv_timer_t *_tm)
{
    OSD_USB_Disk *thiz = static_cast<OSD_USB_Disk *>(lv_timer_get_user_data(_tm));
    Task::post_event(std::bind(thiz->m_callback));
}

} // namespace mb
