#pragma once

#include "common/mb_globals.h"
#include "mb_menu_data.h"
#include "mb_osd.h"
#include "tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_confirm_save.h"
#include "lvgl.h"
#include "mb_menu_resources.h"
#include "mb_osd_auto_search_channel_list.h"
#include "mb_osd_auto_search_choose_satellite.h"
#include "mb_osd_select_satellite.h"
#include "mb_osd_select_switch.h"
#include "mb_osd_password.h"
#include "mb_osd_confirm_save.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_instala_facil.h"
#include "mb_osd_keys.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Manual_Search;

class OSD_Menu_Satellite: public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> Menu_Satellite_t;

private:
    std::unique_ptr<OSD_Auto_Search_Choose_Satellite> mb_choose_satellite;
    std::unique_ptr<Select_Satellite> m_select_satellite;
    std::unique_ptr<OSD_Password> m_password;
    std::unique_ptr<Select_Switch> m_edit_diseq_switch;
    std::unique_ptr<OSD_Edit_Satellite> m_edit_satellite;
    std::unique_ptr<OSD_Instala_Facil> m_instala_facil;
    std::unique_ptr<OSD_Manual_Search> m_auto_search;
    std::unique_ptr<OSD_Confirm_Save> m_confirm_save;

    static constexpr auto offset_x = 400;
    static constexpr auto offset_y = 200;
    static constexpr auto width = 300;
    static constexpr auto heigth = 60 * 6;

    // Linha vertical
    lv_obj_t *m_line_left = nullptr;
    lv_obj_t *m_line_right = nullptr;

    //Menu_Options
    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_box { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };
    lv_obj_t *m_passwd { nullptr };
    lv_obj_t *m_footer { nullptr };
    lv_obj_t *m_save_bckg { nullptr };

    static constexpr auto btn_width = 260;
    static constexpr auto btn_heigth = 50;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_x = 429;
    static constexpr auto button_y = offset_y;
    static constexpr auto button_w = 260;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 271 - offset_y;

    enum class Func_Active
    {
        Auto_Search,
        Easy,
        Satellite,
        Manual_Search,
        Edit_Keys
    };

    Menu_Satellite_t m_callback;

    void auto_search();
    void auto_search_callback(bool _result);
    void auto_search_confirm_callback(bool _ok);
    void select_satellite_for_auto_search_callback();

    void easy_install();
    void easy_install_callback(bool _result);
    void show_menu_easy_install_callback();

    void edit_satellite();
    void edit_satellite_callback(bool _result);
    void select_satellite_for_edit_satellite_callback(Satellite _satellite, bool _result);
    void show_edit_satellite_callback(bool _result);
    void remove_all_channels_confirm_callback(bool _ok);

    void edit_diseq_switch();
    void edit_diseq_switch_callback(bool _result);
    void show_edit_diseq_switch_callback(bool _result);

    void manual_search();
    void manual_search_confirm_callback(bool _result);
    void manual_search_callback();

    lv_obj_t *create_password_box();
    void remove_password_box();

    void change_breadcrumb();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Menu_Satellite(OSD *_parent);
    virtual ~OSD_Menu_Satellite();
    void show_menu_satellite(Menu_Satellite_t _callback, lv_obj_t *_bgd);
};

} // namespace mb
