#pragma once

#include "mb_osd.h"
#include "mb_menu_data.h"

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"

namespace mb {

class OSD_Translate;

class OSD_Channel_List : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> OSD_Channel_List_CB_t;

private:
    static constexpr uint16_t MAX_MENU_ITEM = 4;
    static constexpr uint16_t MAX_CHANNEL_LIST_VIEW = 9;
    static constexpr uint16_t MENU_SPACING = 73;
    static constexpr uint16_t MENU_SUB_MENU_SPACING = 50;
    static constexpr auto logo_x = 90;
    static constexpr auto logo_y = 42;
    static constexpr auto logo_width = 50;
    static constexpr auto logo_heigth = 50;
    static constexpr uint16_t m_instructions_offset = 400;

    //Menu
    uint16_t m_mn_top = 132;

    Channel_List_Type m_current_channel_list { Channel_List_Type::MY_TV_CHANNELS };
    Channel_List_Type m_last_channel_list { Channel_List_Type::MY_TV_CHANNELS };

    Channel_List_Type m_selected_item { Channel_List_Type::MY_TV_CHANNELS };
    bool m_is_sub_menu = false;

    //subMenu_Options
    uint16_t m_subMenu_top = 132;

    uint16_t m_sub_selected_item = 0;
    uint16_t m_start_pos = 0;

    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_main_sub_menu { nullptr };
    lv_obj_t *m_bgd_channel_list { nullptr };
    lv_obj_t *m_lbl_freq { nullptr };
    lv_obj_t *m_lbl_satelite { nullptr };
    lv_obj_t *m_bgd_title { nullptr };
    lv_obj_t *m_img { nullptr };
    lv_obj_t *m_logo_midia { nullptr };
    lv_obj_t *m_lbl { nullptr };
    lv_obj_t *m_lbl_msg { nullptr };
    lv_obj_t *m_bgd_channel_info { nullptr };
    lv_obj_t *m_line_left { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };

    lv_obj_t *m_instructions { nullptr };;
    lv_obj_t *m_instructions_lbl { nullptr };;

    char m_channel_number[10] = {0};
    Service *m_current_service;
    static constexpr uint INVALID_CHANNEL = std::numeric_limits<uint>::max();
    uint m_current_channel { INVALID_CHANNEL };

    void initStyles();

    std::vector<Menu_Data> m_menu_data;
    std::vector<Menu_Data> m_sub_menu_data;
    std::vector<Lineup::Channel_Info> m_channel_data;

    void change_channel();
    int add_menu(std::string_view _title);
    void add_sub_menu(std::string_view _icon);
    static void move_select_left(void *var, int32_t v);
    void set_menu_selection(uint8_t _menu,  bool _selected);
    void move_selection(uint8_t _from, uint8_t _to);
    void reset_menu_selection();
    void update_channel_view_list();
    void set_current_channel_list_view();
    void update_channel_table_list();
    void move_sub_selection(uint16_t _from, uint16_t _to);
    void set_sub_menu_selection(uint16_t _sub_menu_to);
    void reset_sub_menu_selection();
    void display_channel_list(uint16_t start_idx, uint16_t selected_item);
    //void update_channel_info(Lineup *_lineup);
    void update_channel_info();
    void set_favorite_channel(Menu_Data &ch_info, bool _selected);
    void enter_sub_menu();

    void add_menu_fade();
    void remove_menu_fade();

    OSD_Channel_List_CB_t m_callback;

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
    OSD_Channel_List(OSD *_parent);
    virtual ~OSD_Channel_List();

    virtual void show_menu(lv_obj_t *_bgd, OSD_Channel_List_CB_t _callback);

    void move_menu_up();
    void move_menu_down();

    void process();
};

} // namespace mb

