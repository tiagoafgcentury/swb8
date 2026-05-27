#include "mb_osd_draw_closed_caption.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

namespace mb {

OSD_Draw_Closed_Caption::OSD_Draw_Closed_Caption(OSD *_parent):
    OSD(_parent)
{
    MB_ZERO(m_lbl_line);
}

OSD_Draw_Closed_Caption::~OSD_Draw_Closed_Caption()
{
    DELETE_OBJ(m_main_box);
    DELETE_TIMER(m_hide_timer);
}

void OSD_Draw_Closed_Caption::create_closed_caption()
{
    m_main_box = create_rect(get_main_screen(OSD_Layer::CLOSED_CAPTION_LAYER), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_box);
    lv_obj_set_style_bg_opa(m_main_box, LV_OPA_TRANSP, 0);

    for (auto i = 0; i < MBGUI_CC_MAX_LINES; i++)
    {
        DELETE_OBJ(m_lbl_line[i]);
        m_lbl_line[i] = set_label_text(m_main_box, "", 0, 0, font_mono_34, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_line[i]);
        lv_obj_set_size(m_lbl_line[i], LV_SIZE_CONTENT, LV_SIZE_CONTENT); // altura
        lv_obj_set_style_bg_opa(m_lbl_line[i], LV_OPA_100, 0);
        lv_obj_set_style_bg_color(m_lbl_line[i], OSD_COLOR_BLACK, 0);
        lv_obj_set_style_pad_all(m_lbl_line[i], 4, 0);
        lv_obj_set_style_align(m_lbl_line[i], LV_ALIGN_DEFAULT, 0);
    }

    m_hide_timer = lv_timer_create(hide_timer_callback, 6000, this);
    lv_timer_set_repeat_count(m_hide_timer, -1);
}

void OSD_Draw_Closed_Caption::hide_timer_callback(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Draw_Closed_Caption *>(lv_timer_get_user_data(_timer));

    for (auto i = 0; i < MBGUI_CC_MAX_LINES; i++)
    {
        lv_obj_add_flag(thiz->m_lbl_line[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Draw_Closed_Caption::print_closed_caption(const Event_CC &_event)
{
    for (auto i = 0; i < MBGUI_CC_MAX_LINES; i++)
    {
        const auto line = m_lbl_line[i];

        if (_event.lines[i].text.size() > 0)
        {
            const auto x = _event.lines[i].x;
            const auto y = _event.lines[i].y;
            const auto text = _event.lines[i].text.data();
            lv_label_set_text(line, text);
            lv_obj_remove_flag(line, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_y(line, (y * 43) + 5);
            lv_obj_set_x(line, (x * 10) + 40);
        }
        else
        {
            lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_timer_reset(m_hide_timer);
}

} // namespace mb
