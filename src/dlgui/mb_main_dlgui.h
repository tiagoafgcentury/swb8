#pragma once

#include <atomic>
#include <lvgl.h>

#include "common/mb_globals.h"

constexpr auto DEFAULT_SELECTOR = static_cast<uint32_t>(LV_PART_MAIN) | static_cast<uint32_t>(LV_STATE_DEFAULT);

#define OSD_COLOR_WHITE                 lv_color_hex(0xF1F1F1)
#define OSD_COLOR_BLACK                 lv_color_hex(0x000000)
#define OSD_COLOR_BLACK_LIGHT           lv_color_hex(0x373435)
#define OSD_COLOR_GREY                  lv_color_hex(0x737373)
#define OSD_COLOR_GREY_MEDIUM           lv_color_hex(0x484848)
#define OSD_COLOR_GREY_DARK             lv_color_hex(0x232323)
#define OSD_COLOR_BLUE                  lv_color_hex(0x098DE9)
#define OSD_COLOR_RED                   lv_color_hex(0xFF0000)
#define OSD_COLOR_GREEN                 lv_color_hex(0x008000)
#define OSD_COLOR_ORANGE                lv_color_hex(0xFF6D1E)
#define OSD_COLOR_ORANGE_DARK           lv_color_hex(0xFF4000)
#define OSD_COLOR_YELLOW                lv_color_hex(0xFFFF00)

static constexpr uint16_t main_w = DISPLAY_WIDTH;
static constexpr uint16_t main_h = DISPLAY_HEIGHT;
static constexpr uint16_t main_x = (DISPLAY_WIDTH - main_w) / 2;
static constexpr uint16_t main_y = (DISPLAY_HEIGHT - main_h) / 2;

lv_font_t *font_20 = (lv_font_t *) &lv_font_segoeui_20_4bpp;
lv_font_t *g_selected_font = (lv_font_t *) &lv_font_segoeui_40_4bpp;

lv_obj_t *m_main = nullptr;

lv_obj_t *create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color);
lv_obj_t *get_main_screen();

lv_obj_t *set_label_text_base(lv_obj_t *bgd, int x, int y, lv_font_t *font, lv_color_t fontColor);
lv_obj_t *set_label_text(lv_obj_t *bgd, const char *text, int x, int y, lv_font_t *font, lv_color_t fontColor);

void draw_label(const char *text);
void select_font(int _size);
