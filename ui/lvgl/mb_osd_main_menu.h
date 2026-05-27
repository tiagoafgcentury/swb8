#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_menu_home.h"
#include "mb_osd_box_info.h"
#include "mb_osd_menu_ajustes.h"
#include "mb_osd_menu_multimidia.h"

#include <memory>

#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"

namespace mb {

class OSD_Menu_Multimidia;
class OSD_Menu_Home;
class OSD_Menu_Info;
class OSD_Menu_Channel_List;
class OSD_Menu_Ajustes;
class OSD_Translate;

class OSD_Main_Menu : public OSD, public Remote_Control_Handler
{
public:
    enum class Main_Menu_Options
    {
        MENU_HOME = 0,
        MENU_TV_CHANNEL = 1,
        MENU_RADIO_CHANNEL = 2,
        MENU_MIDIA = 3,
        MENU_AJUSTES = 4,
    };

private:
    std::unique_ptr<OSD_Menu_Home> m_osd_menu_home;
    std::unique_ptr<OSD_Menu_Multimidia> m_osd_menu_multimedia;
    std::unique_ptr<OSD_Box_Info> mb_osd_box_info;
    std::unique_ptr<OSD_Menu_Channel_List> mb_osd_menu_channel_list;
    std::unique_ptr<OSD_Menu_Ajustes> mb_osd_menu_ajustes;

    typedef std::function<void(void)> OSD_Main_Menu_CB_t;
    OSD_Main_Menu_CB_t m_callback;

    //Menu
    static constexpr uint16_t m_mn_top = 198;
    static constexpr uint16_t m_mn_left = 90;
    static constexpr uint16_t m_mn_icon_spacing = 47;
    static constexpr uint16_t m_mn_spacing = 73;
    static constexpr uint16_t m_mn_logo_width = 50;
    static constexpr uint16_t m_mn_logo_heigth = 50;
    uint16_t m_button_selected_item = 0;

    //Menu_Options
    uint16_t m_mnopt_top = 0;
    static constexpr uint16_t m_mnopt_spacing = 10;
    uint16_t m_mnopt_left = 0;
    Main_Menu_Options m_selected_item = Main_Menu_Options::MENU_HOME;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bgd_menu { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };
    lv_obj_t *m_logo_century_c { nullptr };
    lv_obj_t *m_logo_century { nullptr };

    lv_obj_t *m_rigth_line { nullptr };
    lv_obj_t *m_info_box = { nullptr };
    static constexpr uint16_t info_heigth = 350;
    static constexpr uint16_t info_width = 450;
    static constexpr uint16_t info_x = 735;
    static constexpr uint16_t info_y = 200;

    std::vector<Menu_Data> m_menu_data;
    std::vector<std::pair<lv_obj_t *, lv_obj_t * >> m_menu_labels;

    int add_menu(std::string_view _title, std::string_view _icon, std::string_view _icon_sel);
    void set_menu_selection(uint8_t _menu, bool _selected);
    void move_selection(uint8_t _from, uint8_t _to);
    void reset_menu_selection();
    void clear_btn_selection(uint8_t _item);
    void set_main_menu();
    void enter_menu_home();
    void enter_menu_channels_edit(OSD_Channels_List_Type _channel_list_type);
    void enter_menu_ajustes();
    void enter_menu_multimidia();

    void enter_menu_multimidia_callback();
    void enter_menu_home_callback(bool _result);
    void show_menu_channel_list();

#ifdef MBGUI_ANIMATION
    enum Main_Menu_Anim
    {
        MENU_EXPAND = 0,
        MENU_DECREASE = 1,
    };

    lv_anim_t m_menu_anim;
    void menu_anim(Main_Menu_Anim _menu_anim);
#endif

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

public:
    OSD_Main_Menu(OSD *_parent);
    virtual ~OSD_Main_Menu();

    virtual void show_main_menu(lv_obj_t *_bgd, OSD_Main_Menu_CB_t _callback);

    void move_menu_up();
    void move_menu_down();

    void process();
};

} // namespace mb
