#pragma once

#include "mb_osd.h"

#include "common/mb_lineup.h"
#include "common/mb_config.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_breadcrumb.h"

namespace mb {

class OSD_Translate;

class OSD_Edit_Channel : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> OSD_Edit_Channel_CB_t;

private:
    static constexpr auto offset_y = 120;
    static constexpr auto width = 890;//DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT - offset_y;
    static constexpr auto radio_height = 180;

    static constexpr uint16_t MAX_MENU_ITEM = 2;
    static constexpr uint16_t MAX_CHANNEL_LIST_VIEW = 10;
    static constexpr uint16_t MENU_SPACING = 73;
    static constexpr uint16_t MENU_SUB_MENU_SPACING = 50;
    static constexpr uint16_t m_instructions_offset = 400;

    //Menu
    uint16_t m_mn_top = 50;

    uint8_t m_current_channel_list = 0;

    uint8_t m_selected_item = 0;
    bool m_is_sub_menu = false;

    //subMenu_Options
    uint16_t m_subMenu_top = 0;

    uint16_t m_sub_selected_item = 0;
    uint16_t m_start_pos = 0;

    bool is_move_channel_pos_enabled = false;

    static constexpr auto m_center_box_x = 380;
    static constexpr auto m_center_box_y = 0;
    static constexpr auto m_center_box_w = 495;
    static constexpr auto m_center_box_h = 500;

    static constexpr auto m_box_info_x = 0;
    static constexpr auto m_box_info_y = 54;
    static constexpr auto m_box_info_spacing = 26;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_box_info { nullptr };
    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_center_box { nullptr };
    lv_obj_t *m_main_sub_menu { nullptr };
    lv_obj_t *m_bgd_channel_list { nullptr };
    lv_obj_t *m_bgd_title { nullptr };
    lv_obj_t *m_img { nullptr };
    lv_obj_t *m_lbl_msg { nullptr };
    lv_obj_t *m_line_left { nullptr };
    lv_obj_t *m_line_right { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };
    lv_obj_t *m_instructions { nullptr };
    lv_obj_t *m_instructions_1 { nullptr };

    lv_obj_t *m_lbl_channel_name { nullptr };
    lv_obj_t *m_lbl_satellite { nullptr };
    lv_obj_t *m_lbl_frequency { nullptr };
    lv_obj_t *m_lbl_symbol_rate { nullptr };
    lv_obj_t *m_lbl_polarity { nullptr };
    lv_obj_t *m_lbl_scrambled { nullptr };

    lv_obj_t *m_lbl_satellite_v { nullptr };
    lv_obj_t *m_lbl_frequency_v { nullptr };
    lv_obj_t *m_lbl_symbol_rate_v { nullptr };
    lv_obj_t *m_lbl_polarity_v { nullptr };

    lv_obj_t *m_radio_preview { nullptr };
    lv_style_t m_front_style = {};

    lv_timer_t *m_tmr_channel_preview { nullptr };

    char m_channel_number[10] = {0};
    Service *m_current_service ;
    static constexpr uint INVALID_CHANNEL = std::numeric_limits<uint>::max();
    uint m_current_channel { INVALID_CHANNEL };

    void initStyles();

    std::vector<Menu_Data> m_menu_data;
    std::vector<Menu_Data> m_sub_menu_data;
    std::vector<Lineup::Channel_Info> m_channel_data;

    int add_menu(std::string_view _title);
    void add_sub_menu(std::string_view _icon_fav, std::string_view _icon_move_up, std::string_view _icon_move_down);
    void set_menu_selection(uint8_t _menu,  bool _selected);
    void move_selection(uint8_t _from, uint8_t _to);
    void reset_menu_selection();
    void update_channel_view_list();
    void update_channel_table_list();
    void move_sub_selection(uint16_t _from, uint16_t _to);
    void set_sub_menu_selection(uint16_t _sub_menu_to);
    void reset_sub_menu_selection();
    void display_channel_list(uint16_t start_idx, uint16_t selected_item);
    void update_channel_info();
    void set_favorite_channel(Menu_Data &ch_info, bool _selected);
    void enter_sub_menu();
    void add_menu_fade();
    void remove_menu_fade();
    void draw_radio_box();

    void set_current_channel_list_view();

    void exit_sub_menu();
    void swap_channel_up();
    void swap_channel_down();
    void swap_services(int _from, int _to);

    OSD_Edit_Channel_CB_t m_callback;
    OSD_Channels_List_Type m_channel_list_type;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Edit_Channel(OSD *_parent);
    virtual ~OSD_Edit_Channel();

    virtual void show_menu_edit_channel(OSD_Edit_Channel_CB_t _callback, OSD_Channels_List_Type _channel_list_type);

    void move_menu_up();
    void move_menu_down();
    void change_channel();

    void process();
};

} // namespace mb
