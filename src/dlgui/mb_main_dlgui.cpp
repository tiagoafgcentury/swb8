#include "mb_main_dlgui.h"

#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <lvgl/lvgl.h>


#include <aui_dis.h>
#include "hal/ALi/mb_ali_globals.h"

using namespace std::chrono_literals;

constexpr auto LOOP_TIME = 10ns;

lv_display_t *g_main_display = nullptr;

#include <aui_hdmi.h>

namespace mb {

void *g_hnd_display { nullptr };
void *g_hnd_analog { nullptr };

}

aui_hdl s_hdl_hdmi = nullptr;
void*  s_hdl_analog = nullptr;

void hdmi_output_on()
{
    ALI_EXEC(aui_hdmi_init(nullptr));
    aui_attr_hdmi attr_hdmi;
    MB_ZERO(attr_hdmi);
    ALI_EXEC(aui_hdmi_open(&attr_hdmi, &s_hdl_hdmi));

    //if (is_hdmi_connected())
    {
        unsigned int on = 0;
        ALI_EXEC(aui_hdmi_on(s_hdl_hdmi));
        ALI_EXEC(aui_hdmi_audio_on(s_hdl_hdmi));
        ALI_EXEC(aui_hdmi_set(s_hdl_hdmi, AUI_HDMI_AV_UNMUTE_SET, NULL, NULL));
        ALI_EXEC(aui_hdmi_set(s_hdl_hdmi, AUI_HDMI_IOCT_SET_AV_BLANK, NULL, &on));
    }
}

void set_cvbs_on()
{
    ALI_EXEC(aui_dis_init(nullptr));
    aui_attr_dis attr_dis;
    MB_ZERO(attr_dis);
    attr_dis.uc_dev_idx = AUI_DIS_SD;
    ALI_EXEC(aui_dis_open(&attr_dis, &s_hdl_analog));


    unsigned int ui_ary_num = 4;
    unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_CVBS};
    ALI_EXEC(aui_dis_dac_unreg(s_hdl_analog, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_dac_reg(s_hdl_analog, ui_dac_attr, ui_ary_num));
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
    // Inicia resolução do HDMI
    aui_dis_tvsys tvsys = AUI_DIS_TVSYS_PAL;
    unsigned char progressive = 99;

    if(aui_dis_tv_system_get(g_hnd_display, &tvsys, &progressive) == 0
            and (tvsys != AUI_DIS_TVSYS_LINE_1080_30 or progressive != 0))
    {
        ALI_EXEC(aui_dis_tv_system_set(g_hnd_display, AUI_DIS_TVSYS_LINE_1080_30, 0));
        attr_dis.uc_dev_idx = AUI_DIS_SD;
        unsigned int ui_ary_num = 4;
        unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_CVBS};
        ALI_EXEC(aui_dis_open(&attr_dis, &g_hnd_analog));
        ALI_EXEC(aui_dis_dac_unreg(g_hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
        ALI_EXEC(aui_dis_dac_reg(g_hnd_analog, ui_dac_attr, ui_ary_num));
        tvsys = AUI_DIS_TVSYS_PAL_M;
        //ALI_EXEC(aui_dis_dac_unreg(g_hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
        ALI_EXEC(aui_dis_tv_system_set(g_hnd_analog, tvsys, 1));
        unsigned int tve_src = AUI_DIS_SD;
        ALI_EXEC(aui_dis_set(g_hnd_analog, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
        ALI_EXEC(aui_dis_dac_reg(g_hnd_analog, ui_dac_attr, ui_ary_num));
    }

    hdmi_output_on();
    set_cvbs_on();
}

void select_font(int _size)
{
    DEBUG_MSG(COMMON, DEBUG, "Selecting font size " << _size << "\n");

    switch(_size)
    {
        case 0:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_40_4bpp;
            break;

        case 1:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_30_4bpp;
            break;

        case 2:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_25_4bpp;
            break;

        case 3:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_20_4bpp;
            break;

        case 4:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_bold_40_4bpp;
            break;

        case 5:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_bold_30_4bpp;
            break;

        case 6:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_bold_25_4bpp;
            break;

        case 7:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_bold_20_4bpp;
            break;

        case 8:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_semi_bold_40_4bpp;
            break;

        case 9:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_semi_bold_30_4bpp;
            break;

        case 10:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_semi_bold_25_4bpp;
            break;

        case 11:
        default:
            g_selected_font = (lv_font_t *)&lv_font_segoeui_semi_bold_20_4bpp;
            break;
    }
}

void draw_label(const char *_text)
{
    static int line = 20;
    auto label = set_label_text_base(m_main, 0, line, g_selected_font, OSD_COLOR_WHITE);
    lv_label_set_text(label, _text);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, line);
    DEBUG_MSG(COMMON, DEBUG, "Line " << line << ": " << _text << "\n");
    lv_point_t pos;
    lv_text_get_size(&pos, _text, g_selected_font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    line += (pos.y + 10);
    DEBUG_MSG(COMMON, DEBUG, "Line " << line << ": " << _text << "\n");
}

lv_obj_t *get_main_screen()
{
    return lv_screen_active();
}

lv_obj_t *create_rect(lv_obj_t *_bgd, int _x, int _y, int _w, int _h, lv_color_t _color)
{
    auto rect = lv_obj_create(_bgd);
    lv_obj_set_size(rect, _w, _h);
    lv_obj_set_scrollbar_mode(rect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect, _color, 0);
    lv_obj_set_style_radius(rect, 0, DEFAULT_SELECTOR);
    lv_obj_align(rect, LV_ALIGN_DEFAULT, _x, _y);
    lv_obj_set_style_pad_all(rect, 0, LV_PART_MAIN);
    return rect;
}

lv_obj_t *set_label_text_base(lv_obj_t *_bgd, int _x, int _y, lv_font_t *_font, lv_color_t _font_color)
{
    auto label = lv_label_create(_bgd);
    lv_obj_set_style_text_font(label,  _font, 0);
    lv_obj_set_style_text_color(label, _font_color, DEFAULT_SELECTOR);
    lv_obj_align(label, LV_ALIGN_DEFAULT, _x, _y);
    return label;
}

int main(int _argc, char **_argv)
{
    select_font(0);
    init_display_ali();
    lv_init();
    g_main_display = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(g_main_display, "/dev/fb0");
    lv_display_trigger_activity(nullptr);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
    lv_display_set_color_format(g_main_display, LV_COLOR_FORMAT_ARGB8888);
    m_main = create_rect(get_main_screen(), main_x, main_y, main_w, main_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_radius(m_main, 10, LV_PART_MAIN);

    // Verifica se existe mais de 1 argumento
    if(_argc < 2)
    {
        std::cerr << "Usage: " << _argv[0] << " [<font num>] <text> [[<font num>] <text> ...]" << std::endl;
        return 1;
    }

    for(int i = 1; i < _argc; i++)
    {
        // Testa se o argv[i] é um número, se for, seta a fonte
        char *endptr = nullptr;
        auto size = strtol(_argv[i], &endptr, 10);

        if(endptr and * endptr == 0)
        {
            // arv[i] é um número
            select_font(size);
            continue;
        }

        // Se não for, desenha o texto
        draw_label(_argv[i]);
    }

    for(int i = 0; i < 3; i++)
    {
        lv_timer_handler();
        std::this_thread::sleep_for(LOOP_TIME);
    }

    lv_deinit();
    return 0;
}
