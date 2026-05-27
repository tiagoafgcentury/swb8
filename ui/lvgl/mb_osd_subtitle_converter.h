#pragma once

#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"
#include "common/mb_types.h"

namespace mb {

class Subtitle_Converter {
public:
    static lv_font_t* to_font(int idx) {
        switch (idx) {
            case 0: return font_20;
            case 1: return font_semi_25;
            case 2: return font_semi_40;
            default: return font_semi_25;
        }
    }

    static lv_font_t* to_font(Subtitle_Font_Size size) {
        return to_font(static_cast<int>(size));
    }

    static lv_color_t to_font_color(int idx) {
        switch (idx) {
            case 0: return OSD_COLOR_WHITE;
            case 1: return OSD_COLOR_YELLOW;
            case 2: return OSD_COLOR_BLACK;
            default: return OSD_COLOR_WHITE;
        }
    }

    static lv_color_t to_background_color(int idx) {
        switch (idx) {
            case 0: return OSD_COLOR_WHITE;
            case 1: return OSD_COLOR_GREY;
            case 2: return OSD_COLOR_YELLOW;
            default: return OSD_COLOR_WHITE;
        }
    }
};

} // namespace mb
