#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_confirm_save.h"
#include "mb_osd_confirm_default.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_keys.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_footer.h"
#include "mb_osd_set_clock.h"
#include "mb_osd_set_timezone.h"
#include "mb_osd_password.h"
#include "mb_osd_change_password.h"
#include "mb_osd_parental_control.h"

#include <map>
#include <string>
#include <memory>
#include <functional>

namespace mb {

class OSD_Menu_Config: public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Config_CB_t;

private:
    std::unique_ptr<OSD_Confirm_Save> m_confirm_save;
    std::unique_ptr<OSD_Confirm_Default> m_confirm_default;
    std::unique_ptr<Osd_Breadcrumb> m_breadCrumb;
    std::unique_ptr<OSD_Set_Clock> m_set_clock;
    std::unique_ptr<OSD_Set_Timezone> m_set_timezone;
    std::unique_ptr<OSD_Password> m_osd_password;
    std::unique_ptr<OSD_Change_Password> m_osd_change_password;
    std::unique_ptr<OSD_Parental_Control> m_osd_parental_control;

    Event_System_Settings m_param_settings;

    // Menu principal
    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };
    static constexpr auto offset_x = 400;
    static constexpr auto offset_y = 114;
    static constexpr auto width = 260 + 23 + 23;
    static constexpr auto height = 520;

    lv_obj_t *m_bgd_options { nullptr };

    // Linha vertical
    lv_obj_t *m_left_line = nullptr;
    lv_obj_t *m_center_line = nullptr;
    lv_obj_t *m_right_line = nullptr;

    lv_obj_t *m_passwd { nullptr };

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_x = 429;
    static constexpr auto button_y = 114;
    static constexpr auto button_w = 260;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 189 - 114;

    MB_OSD_Keys m_options;
    static constexpr auto option_x = 735;
    static constexpr auto option_y = 114;
    static constexpr auto option_w = 220;
    static constexpr auto option_h = 50;
    static constexpr auto option_s = 180 - 114;
    std::vector<std::string_view> m_option_names;

    lv_obj_t *m_footer { nullptr };
    static constexpr auto footer_y = -40;

    // Areas de cobertura de tela
    lv_obj_t *m_save_bckg { nullptr };
    static constexpr auto save_x1 = 735 - 23 + 3;
    static constexpr auto save_y1 = 100;
    static constexpr auto save_x2 = save_x1 + (3 * width) / 2;
    static constexpr auto save_y2 = height + 14;

    lv_obj_t *m_config_area { nullptr };
    lv_obj_t *m_config_fade { nullptr };

    // Coluna sendo processada
    enum class Column_Active
    {
        First,
        Second,
        Third,
    };
    Column_Active m_column_active = Column_Active::First;

    //Menu_Options
    lv_obj_t *m_main_screen { nullptr };

    lv_obj_t *m_tmzone { nullptr };

    lv_obj_t *m_clock { nullptr };
    lv_obj_t *m_clock_text { nullptr };
    lv_obj_t *m_clock_ta { nullptr };
    lv_obj_t *m_clock_box { nullptr };

    lv_obj_t *m_tmzone_2 { nullptr };
    lv_obj_t *m_tmzone_3 { nullptr };
    lv_obj_t *m_tmzone_4 { nullptr };
    lv_obj_t *m_tmzone_5 { nullptr };
    lv_obj_t *m_tmzone_2_text { nullptr };
    lv_obj_t *m_tmzone_3_text { nullptr };
    lv_obj_t *m_tmzone_4_text { nullptr };
    lv_obj_t *m_tmzone_5_text { nullptr };

    lv_obj_t *m_aspect { nullptr };
    lv_obj_t *m_aspect_box { nullptr };

    lv_obj_t *m_color { nullptr };
    lv_obj_t *m_color_box { nullptr };

    lv_obj_t *m_default { nullptr };
    lv_obj_t *m_default_box { nullptr };

    lv_obj_t *m_zip { nullptr };
    lv_obj_t *m_zip_box { nullptr };
    lv_obj_t *m_zip_ta { nullptr };
    lv_obj_t *m_ta { nullptr };

    lv_obj_t *m_version_label { nullptr };
    lv_obj_t *m_parental_title { nullptr };

    lv_obj_t *m_star { nullptr };

    // Função ativada
    enum class Func_Index
    {
        Language,
        Resolution,
        Color,
        Aspect,
        Clock,
        Parental_Control,
        Default,
    };

    // Variáveis estáticas da classe
    Func_Index m_active = Func_Index::Language;

    Zone_ID_t m_local_zone_id = 0;

    // Constantes de posicioamento de menu
    static constexpr auto text_width = DISPLAY_WIDTH / 4;
    static constexpr auto text_height = 60;
    static constexpr auto text_padding = 10;
    static constexpr auto box_space = (DISPLAY_HEIGHT - 8 * text_height - 7 * text_padding) / 2;
    static constexpr auto x_pos = 120;
    static constexpr auto y_language = box_space;
    static constexpr auto y_clock = box_space + text_height + text_padding;
    static constexpr auto y_resolution = box_space + 2 * (text_height + text_padding);
    static constexpr auto y_tmzone = box_space + 3 * (text_height + text_padding);
    static constexpr auto y_aspect = box_space + 4 * (text_height + text_padding);
    static constexpr auto y_color = box_space + 5 * (text_height + text_padding);
    static constexpr auto y_default = box_space + 6 * (text_height + text_padding);
    static constexpr auto y_zip = box_space + 7 * (text_height + text_padding);

    static constexpr auto tmzone_width = (DISPLAY_WIDTH / 8) - 5;
    static constexpr auto tmzone_2_x = x_pos + text_width + 10;
    static constexpr auto tmzone_3_x = tmzone_2_x + tmzone_width + 10;
    static constexpr auto tmzone_4_x = tmzone_3_x + tmzone_width + 10;
    static constexpr auto tmzone_5_x = tmzone_4_x + tmzone_width + 10;

    std::vector<Menu_Data> m_menu_data;

    void process_factory_default(Remote_Control_Key _key);
    void process_zip(Remote_Control_Key _key);

    void update_breadcrumb();
    void process_options();
    void create_bgd_options(bool _large);
    void create_fade();
    void draw_options();
    void draw_language();
    void draw_resolution();
    void draw_color_pattern();
    void draw_aspect_mode();
    void draw_time_and_date();
    void draw_parental_control();
    void draw_factory_default();
    void draw_save_background();

    void process_ok();
    void process_language();
    void process_resolution();
    void process_color_pattern();
    void process_aspect_mode();
    void process_time_and_date();
    void process_factory_default();
    void create_zip_area();
    void create_timezone_area();
    void create_clock_area();
    void get_time_date(char *_buffer, int _len);
    void create_config_area();

    void parental_control_password_callback(bool _ok);
    void config_parental_control();
    void config_parental_control_callback(bool _ok);
    void process_parental_control(bool _ok);
    void process_parental_control_callback(bool _ok);
    void factory_default_password_callback(bool _ok);
    void factory_default_confirm_callback(bool _ok);
    void factory_default_callback();

    Config_CB_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Config(OSD *_parent);
    virtual ~OSD_Menu_Config();
    void show_menu_config(Config_CB_t _callback, lv_obj_t *_bgd);
};

} // namespace mb

