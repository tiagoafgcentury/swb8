#pragma once

#include "mb_osd.h"
#include "mb_osd_fonts.h"

#include <string>

namespace mb {

namespace MB_OSD_Footer {
lv_obj_t *draw(lv_obj_t *_background, std::string_view _text, int _offset_y,
               lv_font_t *_font = font_20);
};

}
