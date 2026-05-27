#include "mb_osd_waiting_signal.h"
#include "mb_osd_translate.h"
#include "mb_menu_data.h"
#include "mb_menu_resources.h"
#include "common/mb_globals.h"
#include "mb_osd_fonts.h"

#include <string.h>
#include <map>

#ifndef NDEBUG
    #include <chrono>
    using namespace std::chrono;
#endif

namespace mb {

OSD_Waiting_Signal::OSD_Waiting_Signal(OSD *_parent):
    OSD(_parent)
{
}

OSD_Waiting_Signal::~OSD_Waiting_Signal()
{
}

void OSD_Waiting_Signal::update_waiting_signal()
{
    if (m_bgd_wfs)
    {
        int rndX = rand() % MAX_RANGE_WIDTH; //WIDTH - 150 - 150 - MAX_W_WAITING_SIGNAL
        int rndY = rand() % MAX_RANGE_HEIGHT; //HEIGHT - 100 -100 - MAX_H_WAITING_SIGNAL
        rndX += OFFSET_X;
        rndY += OFFSET_Y;
        lv_obj_align(m_bgd_wfs, LV_ALIGN_DEFAULT, rndX, rndY);
    }
}

void OSD_Waiting_Signal::waiting_signal(lv_obj_t *_bgd_main)
{
    if (m_bgd == nullptr)
    {
        m_bgd = create_rect(_bgd_main, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd);
        lv_obj_align(m_bgd, LV_ALIGN_CENTER, 0, 0);
        load_image(m_bgd, LOGO_MENU_CENTURY, 90, 42, 213, 51);
        add_clock(m_bgd, 1090, 28);
        auto label_wfs = set_label_text_static(m_bgd, tr(__Sem_sinal_da_antena), 0, 0, font_bold_40, OSD_COLOR_WHITE);
        lv_obj_align(label_wfs, LV_ALIGN_CENTER, 0, 0);
#ifdef MBGUI_USE_RLOTTIE
        auto sat_logo = lv_rlottie_create_from_file(m_bgd, 200, 200, ANIM_SAT_SIGNAL);
        lv_rlottie_set_play_mode(sat_logo, LV_RLOTTIE_CTRL_LOOP);
        lv_obj_align(sat_logo, LV_ALIGN_CENTER, 0, -100);
#endif
    }
}

void OSD_Waiting_Signal::cleanup_screen_waiting_signal()
{
    DELETE_TIMER(m_tmr_wait_signal);
    DELETE_OBJ(m_bgd);
}

} //mb
