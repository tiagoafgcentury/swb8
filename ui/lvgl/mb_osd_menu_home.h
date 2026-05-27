#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"

#include <memory>
#include <chrono>
#include <vector>

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"

namespace mb {

class Fade_Canvas;
class OSD_Translate;

class OSD_Menu_Home : public OSD, public OSD_Clock, public Remote_Control_Handler
{
private:
    typedef std::function<void(bool)> Menu_Home_CB_t;
    Menu_Home_CB_t m_callback;

    Menu_Home m_menu_home;
    std::shared_ptr<Fade_Canvas> m_bgd_video;

    lv_style_t m_menu_style;
    lv_style_t m_menu_sel_style;
    lv_style_t m_box_style;
    lv_style_t m_class_style;
    int m_menu_index = 0;

    //Menu_Favoritos
    uint16_t m_menu_num_categories = 0;
    uint8_t m_selected_item = 0;
    uint8_t m_selected_item_hor = 0;
    uint8_t m_selected_item_ver = 0;

    static constexpr uint16_t MENU_SPACING = 10;
    static constexpr uint16_t MENU_SPACING_SUBTITLE = 275;
    static constexpr uint16_t MENU_TEXT_SPACING = 310;
    static constexpr uint16_t MENU_MAX_TEXT_WIDTH = 545;
    static constexpr uint16_t MENU_PARENTAL_RATING_X = 20;
    static constexpr uint16_t MENU_PARENTAL_RATING_Y = 123;
    static constexpr uint16_t MAX_SUBMENU_VIEW = 8;
    static constexpr uint16_t MAX_SUBMENU_VIEW_LINE = 4;

    //static constexpr auto NO_EPG_DATA = tr(__Sem_informacoes);

    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bgd_sub_menu { nullptr };
    lv_obj_t *m_bgd_categories { nullptr };
    lv_obj_t *m_menu_title { nullptr };
    lv_obj_t *m_menu_title_next { nullptr };
    lv_obj_t *m_menu_title_previous { nullptr };
    uint16_t m_menu_current_ver_pos = m_menu_home.MAX_CATEGORIES;
    uint16_t m_menu_title_idx = 0;

    lv_obj_t *m_channel_name { nullptr };
    lv_obj_t *m_channel_desc { nullptr };
    lv_obj_t *m_program { nullptr };
    lv_obj_t *m_subtitle { nullptr };
    lv_obj_t *m_class_indic { nullptr };
    lv_obj_t *m_img_fav { nullptr };
    lv_timer_t *m_tmr_wait_change_channel { nullptr };
    lv_timer_t *m_tmr_channel_preview { nullptr };

    Service *m_atual_srv { nullptr };
    Service *m_last_srv { nullptr };
    Service_ID_t m_service_id { 0 };
    Transponder_Id m_transponder_id;

    void init_styles();
    static void move_select_left(void *var, int32_t v);

    struct
    {
        std::string channel_name;
        std::string channel_number;
        std::string channel_category;
        std::string channel_program;
        std::string program_description;
        Parental_Control parental_rating;
        bool favorite;
    } m_channel_text_info;

    std::string_view get_category_title(uint8_t index);
    void set_menu_selection(Sub_Menu_Data _sub_menu, Sub_Menu_View &_sub_menu_view, bool _selected, bool _hidden);
    void set_menu_selection(uint16_t _menu, uint16_t _menu_ver, uint16_t _menu_hor, bool _selected);
    void move_selection(uint16_t _ver, uint16_t _hor);

    void create_menu(int _cat_idx, int _sub_menu_index);
    void add_menu_title(std::string_view _category_name, std::string_view _logo);
    int add_menu_option(std::string_view _text, std::string_view _icon);
    void return_main_menu();

    void print_channel_info();

protected:
    bool handle_event_remote_control(const Event_Remote_Control &_event);
    void got_focus();

public:
    OSD_Menu_Home(OSD *_parent);
    virtual ~OSD_Menu_Home();

    void show_menu_home(Menu_Home_CB_t _callback, lv_obj_t *_bgd);

    void update_channel_info(bool _save_channel_info, Service *_srv = nullptr);
    void change_channel(bool _save_channel_info);
    void move_menu_up();
    void move_menu_down();
    void move_menu_right();
    void move_menu_left();
};

} // namespace mb
