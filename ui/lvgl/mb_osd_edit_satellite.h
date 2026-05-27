#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_keyboard.h"
#include "mb_osd_keys.h"
#include "mb_osd_breadcrumb.h"
#include "mb_lnbf_detection.h"
#include "mb_lnbf_detection.h"
#include "mb_osd_lnbf_detection.h"

#include "tasks/mb_task_database.h"

#include <memory>
#include <array>

namespace mb {

class Fade_Canvas;

class OSD_Edit_Satellite :  public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> osd_edit_satellite_callback_t;

private:
    std::unique_ptr<OSD_Keyboard> m_osd_keyboard;
    std::shared_ptr<MB_Detect_Lnbf> m_detect_lnbf;
    std::shared_ptr<OSD_Lnbf_Detection> m_osd_lnbf_detection;
    std::unique_ptr<Osd_Breadcrumb> mb_osd_breadcrumb;

    static constexpr auto offset = 124;
    static constexpr auto offset_x = 400;
    static constexpr auto offset_y = 116;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT;

    lv_obj_t *m_mainscreen { nullptr };
    lv_obj_t *m_bgd_sat { nullptr };

    enum class Func_Active
    {
        LNBF_Detection,
        Name,
        Band,
        LNBF_Type,
        Polarity,
        Switch,
        Save,
        COUNT
    };

    std::array<lv_obj_t *, static_cast<size_t>(Func_Active::COUNT)> m_titles;
    std::array<lv_obj_t *, static_cast<size_t>(Func_Active::COUNT)> m_labels;
    std::array<std::string_view, static_cast<size_t>(Func_Active::COUNT)> m_title_names =
    {
        "LNBF",
        tr(__Nome_do_satelite),
        tr(__Banda),
        tr(__Tipo_de_LNBF),
        tr(__Polaridade),
        tr(__Chave),
    };

    static constexpr auto title_width = 230;
    static constexpr auto title_heigth = 50;
    static constexpr auto title_x = 90;
    static constexpr auto title_y = 3;
    static constexpr auto title_spacing = 203 - 132;

    MB_OSD_Keys m_keys_main;
    static constexpr auto button_width = title_width;
    static constexpr auto button_heigth = title_heigth;
    static constexpr auto button_x = 348;
    static constexpr auto button_y = title_y;
    static constexpr auto button_spacing = title_spacing;

    lv_obj_t *m_cover_area { nullptr };
    static constexpr auto cover_w = width - 680;
    static constexpr auto cover_h = 6 * title_spacing;
    static constexpr auto cover_x = 0;
    static constexpr auto cover_y = 0;

    lv_obj_t *m_line { nullptr };
    static constexpr auto line_width = 3;
    static constexpr auto line_heigth = 6 * title_spacing;
    static constexpr auto line_x = width - 959;
    static constexpr auto line_y = 0;

    MB_OSD_Keys m_keys_options;
    static constexpr auto btn_x = 618;
    static constexpr auto btn_y = 270;
    static constexpr auto btn_w = 220;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_spacing = 917 - 683;

    MB_OSD_Keys m_key_save;
    static constexpr auto btn_save_width = 220;
    static constexpr auto btn_save_heigth = 50;
    static constexpr auto btn_save_x = (width - btn_save_width) / 2;
    static constexpr auto btn_save_y = 570 - offset;
    bool m_save_active = false;

    MB_OSD_Keys m_key_voltar;
    MB_OSD_Keys m_key_diseq_c;
    static constexpr auto btn_diseq_c_x = btn_x+20;
    static constexpr auto btn_diseq_c_y = 330;
    static constexpr auto btn_diseq_c_w = 90;
    static constexpr auto btn_diseq_c_h = 40;
    static constexpr auto btn_diseq_c_s = btn_diseq_c_w + 10;
    bool m_diseq_c_active = false;
    int m_diseq_c_selected = 0;
 
    // Barra de progresso
    lv_obj_t *m_slider_label;
    static constexpr auto slider_label_x = 0;
    static constexpr auto slider_label_y = -40;
    lv_obj_t *m_slider;
    static constexpr auto slider_y = 0;
    static constexpr auto slider_w = cover_w-100;
    static constexpr auto slider_x = 0;
    static constexpr auto slider_h = 20;

    lv_timer_t *m_refresh_timer { nullptr };
    int m_progress = 0 ;
    std::vector<Satellite> m_satellites = {};

    void draw_buttons();
    void draw_titles();
    void edit_name();
    void reset_sat_options();
    void draw_sat_options();
    void lnbf_detection();
    void create_progress_bar();
    void update_progress_bar();
    static void update_progress_bar_cb(lv_timer_t *_timer);
    void lnbf_detection_failed();
    void lnbf_detection_success(Transponder_Id tp);
    void draw_voltar_button();
    void process_changes(const Event_Remote_Control &_event);
    void save_changes();
    void discard_changes();
    void process_ok_button();
    void update_satellite(const Satellite &s);
    void draw_diseq_c_buttons(DiseqC_Type _current_type);
    void process_diseq_c(const Event_Remote_Control &_event);
    void disable_diseq_c();

    osd_edit_satellite_callback_t m_callback;
    Satellite m_current_satellite;
    Satellite m_previous_satellite;

protected:
    static OSD_Edit_Satellite *s_instance;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Edit_Satellite(OSD *_parent);
    virtual ~OSD_Edit_Satellite();
    void edit_satellite(osd_edit_satellite_callback_t _callback, Satellite _sat);
};

} // namespace mb
