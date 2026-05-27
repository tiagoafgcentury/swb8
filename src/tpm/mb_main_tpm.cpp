#include "mb_main_tpm.h"
#include "mb_task_http_server.h"
#include "mb_tpm.h"

#include <chrono> 
#include <thread>
#include <lvgl/lvgl.h>

#include <aui_dis.h>
#include "hal/ALi/mb_ali_globals.h"

using namespace std::chrono_literals;

constexpr auto LOOP_TIME = 150ns;

std::atomic<bool> g_mbgui_keep_running = true;

namespace mb {

lv_display_t *g_main_display = nullptr;

void *g_hnd_display { nullptr };
void *g_hnd_analog { nullptr };

}

void init_display_ali()
{
    using mb::ali_error_msg;
    using mb::g_hnd_display;
    using mb::g_hnd_analog;
    ALI_EXEC(aui_dis_init(nullptr));
    aui_attr_dis attr_dis;
    MB_ZERO(attr_dis);
    attr_dis.uc_dev_idx = AUI_DIS_HD;
    ALI_EXEC(aui_dis_open(&attr_dis, &g_hnd_display));
    MB_ZERO(attr_dis);
    ALI_EXEC(aui_dis_tv_system_set(g_hnd_display, AUI_DIS_TVSYS_LINE_1080_30, 0));
    attr_dis.uc_dev_idx = AUI_DIS_SD;
    unsigned int ui_ary_num = 4;
    unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_CVBS};
    ALI_EXEC(aui_dis_open(&attr_dis, &g_hnd_analog));
    ALI_EXEC(aui_dis_dac_unreg(g_hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_dac_reg(g_hnd_analog, ui_dac_attr, ui_ary_num));
    aui_dis_tvsys tvsys;
    tvsys = AUI_DIS_TVSYS_PAL_M;
    //ALI_EXEC(aui_dis_dac_unreg(g_hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_tv_system_set(g_hnd_analog, tvsys, 1));
    unsigned int tve_src = AUI_DIS_SD;
    ALI_EXEC(aui_dis_set(g_hnd_analog, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
    ALI_EXEC(aui_dis_dac_reg(g_hnd_analog, ui_dac_attr, ui_ary_num));
}


int main()
{
    lv_init();
    mb::g_main_display = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(mb::g_main_display, "/dev/fb0");
    lv_display_trigger_activity(NULL);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
    lv_display_set_color_format(mb::g_main_display, LV_COLOR_FORMAT_ARGB8888);
    init_display_ali();
    mb::tpm::create_background();
    mb::Task_HTTP_Server task_http_server;

    while(g_mbgui_keep_running.load(std::memory_order_relaxed))
    {
        mb::Task::run_processes();
        lv_timer_handler();
        std::this_thread::sleep_for(LOOP_TIME);
    }

    lv_deinit();
    return 0;
}

