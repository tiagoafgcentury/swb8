#pragma once

#include "mb_osd.h"
#include "mb_osd_translate.h"
#include <array>
#include <vector>

namespace mb {

template <uint16_t MAX_ROWS = 10>
class MB_OSD_Service_Table_Progress : public OSD
{
public:
    MB_OSD_Service_Table_Progress(OSD* _parent) : OSD(_parent)
    {
        lv_style_init(&m_style_main);
        lv_style_init(&m_style_indicator);
    }
    virtual ~MB_OSD_Service_Table_Progress()
    {
        lv_style_reset(&m_style_main);
        lv_style_reset(&m_style_indicator);
        DELETE_OBJ(m_bgd_table);
    }

    void create_services_table(lv_obj_t* parent, lv_coord_t x = 0, lv_coord_t y = 0)
    {
        // Background for the progress info
        m_bgd_table = create_rect(parent, x, y, bgd_table_w, 500, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_table);
        lv_obj_align(m_bgd_table, LV_ALIGN_TOP_MID, 0, 55);

        // Create a title for the table at the top
        m_table_title = create_rect(m_bgd_table, 0, 0, bgd_table_w, table_title_h, OSD_COLOR_GREY_DARK);
        lv_obj_null_on_delete(&m_table_title);
        lv_obj_align(m_table_title, LV_ALIGN_TOP_MID, 0, 0);

        m_table_title_label = set_label_text(m_table_title, "", 0, 0, font_semi_20, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_table_title_label);
        lv_obj_align(m_table_title_label, LV_ALIGN_CENTER, 0, 0);

        // History container (to allow scrolling if history grows too much, though here we just use fixed labels)
        for (int i = 0; i < MAX_STATUS_LINES; ++i)
        {
            m_status_labels[i] = set_label_text(m_bgd_table, "", 0, table_title_h + 10 + (i * 25), font_20, OSD_COLOR_WHITE);
            lv_obj_null_on_delete(&m_status_labels[i]);
            lv_obj_align(m_status_labels[i], LV_ALIGN_TOP_LEFT, 30, table_title_h + 10 + (i * 25));
        }
        create_progress_bar();
    }

    void create_progress_bar()
    {
        lv_style_set_bg_opa(&m_style_main, LV_OPA_COVER);
        lv_style_set_bg_color(&m_style_main, OSD_COLOR_GREY_MEDIUM);
        lv_style_set_bg_color(&m_style_indicator, OSD_COLOR_BLUE);
        m_bgd_slider = create_rect(m_bgd_table, m_bgd_slider_x, m_bgd_slider_y, m_bgd_slider_w, m_bgd_slider_h, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_slider);
        m_slider = lv_bar_create(m_bgd_slider);
        lv_obj_null_on_delete(&m_slider);
        lv_obj_add_style(m_slider, &m_style_main, LV_PART_MAIN);
        lv_obj_add_style(m_slider, &m_style_indicator, LV_PART_INDICATOR);
        lv_obj_set_size(m_slider, slider_w - 4, slider_h);
        lv_obj_align(m_slider, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_anim_duration(m_slider, 0, 0);
        lv_obj_set_style_bg_color(m_slider, OSD_COLOR_YELLOW, LV_PART_INDICATOR);
        /*Create a label below the slider*/
        m_slider_label = set_label_text(m_bgd_slider, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_slider_label);
        lv_obj_align(m_slider_label, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_text(m_slider_label, "0%");
    }

    void set_animation_duration(uint32_t duration)
    {
        lv_obj_set_style_anim_duration(m_slider, duration, 0);
    }

    void set_progress(uint8_t percentage)
    {
        if (percentage > 100)
        {
            percentage = 100;
        }

        lv_slider_set_value(m_slider, percentage, LV_ANIM_ON);
        lv_label_set_text_fmt(m_slider_label, "%d%%", percentage);
    }

    void add_status(std::string_view text)
    {
        if (m_status_count < MAX_STATUS_LINES)
        {
            if (m_status_labels[m_status_count])
            {
                lv_label_set_text(m_status_labels[m_status_count], text.data());
                m_status_count++;
            }
        }
        else
        {
            // Scroll history: move all texts up
            for (int i = 0; i < MAX_STATUS_LINES - 1; ++i)
            {
                lv_label_set_text(m_status_labels[i], lv_label_get_text(m_status_labels[i+1]));
            }
            lv_label_set_text(m_status_labels[MAX_STATUS_LINES - 1], text.data());
        }
    }

    void set_title(std::string_view text)
    {
        if (m_table_title_label) {
            lv_label_set_text(m_table_title_label, text.data());
        }
        else if (m_table_title)
        {
            m_table_title_label = set_label_text(m_table_title, text.data(), 0, 0, font_semi_20, OSD_COLOR_WHITE);
            lv_obj_align(m_table_title_label, LV_ALIGN_CENTER, 0, 0);
        }
    }

    void reset()
    {
        lv_slider_set_value(m_slider, 0, LV_ANIM_ON);
        lv_label_set_text(m_slider_label, "0%");
        DELETE_OBJ(m_table_title);
        DELETE_OBJ(m_table_title_label);
        for (int i = 0; i < MAX_STATUS_LINES; ++i)
        {
            DELETE_OBJ(m_status_labels[i]);
        }
        DELETE_OBJ(m_status_label);
        DELETE_OBJ(m_bgd_table);
    }

private:
    lv_obj_t* m_bgd_table{nullptr};
    
    // Barra de progresso
    lv_obj_t* m_bgd_slider{nullptr};
    static constexpr auto m_bgd_slider_x = 0;
    static constexpr auto m_bgd_slider_y = 400;
    static constexpr auto m_bgd_slider_w = 940;
    static constexpr auto m_bgd_slider_h = 55;

    // Progress UI
    lv_obj_t* m_slider_label{nullptr};
    lv_obj_t* m_slider{nullptr};
    static constexpr auto slider_w = 910;
    static constexpr auto slider_h = 20;
    lv_style_t m_style_main;
    lv_style_t m_style_indicator;
	lv_obj_t* m_table_title{nullptr};
	lv_obj_t* m_table_title_label{nullptr};
    lv_obj_t* m_status_label{nullptr};

    static constexpr int MAX_STATUS_LINES = 14;
    std::array<lv_obj_t*, MAX_STATUS_LINES> m_status_labels{nullptr};
    uint8_t m_status_count = 0;

	static constexpr auto table_title_h = 40;
    static constexpr auto bgd_table_w = 940;

	Service m_last_service;
};

} // namespace mb
