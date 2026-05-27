#pragma once

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_edit_channel.h"
#include "mb_osd_guide_channel.h"
#include "mb_osd_scheduled_list.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"

#include "common/mb_lineup.h"
#include <memory>
#include <chrono>
#include <vector>

namespace mb {

class OSD_Translate;
class OSD_Edit_Channel;
class OSD_Guide_Channel;
class OSD_Scheduled_List;

class OSD_Menu_Channel_List : public OSD, public Remote_Control_Handler
{
private:
    std::unique_ptr<OSD_Edit_Channel> m_osd_edit_ch;
    std::unique_ptr<OSD_Guide_Channel> m_osd_guide_channel;
    std::unique_ptr<OSD_Scheduled_List> m_osd_scheduled_list;

    typedef std::function<void(bool)> OSD_Menu_channel_list_CB_t;

    lv_obj_t *m_bgd { nullptr };
    lv_obj_t *m_main_menu { nullptr };
    lv_obj_t *m_bgd_top { nullptr };
    static constexpr auto main_x = 150;
    static constexpr auto main_y = 0;
    static constexpr auto main_w = DISPLAY_WIDTH / 2 + 20;
    static constexpr auto main_h = DISPLAY_HEIGHT;

    lv_obj_t *m_info_box = { nullptr };
    static constexpr uint16_t info_heigth = 350;
    static constexpr uint16_t info_width = 260 + 2 * 23;
    static constexpr uint16_t info_x = 430 - 23;
    static constexpr uint16_t info_y = 205;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto offset_x = main_x;
    static constexpr auto offset_y = 0;
    static constexpr auto button_x = 163;
    static constexpr auto button_y = 198;
    static constexpr auto button_w = 260;
    static constexpr auto button_h = 50;
    static constexpr auto spacing = 271 - 198;

    // Função ativada
    enum Func_Active
    {
        Edit_Channels,
        Guide,
        Schedules,
        COUNT
    };

    Func_Active m_function_active = Edit_Channels;

    Service *m_current_srv = nullptr;

    lv_obj_t *m_bgd_fade { nullptr };
    OSD_Menu_channel_list_CB_t m_callback;
    OSD_Channels_List_Type m_channel_list_type;
    Channel_List_Type m_current_channel_list;

    void show_menu_channel_callback();
    void show_menu_guide_channel_callback();
    void show_menu_schedule_list_callback();
    void update_breadcrumb();

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

public:
    OSD_Menu_Channel_List(OSD *_parent);
    virtual ~OSD_Menu_Channel_List();

    virtual void show_menu_channel_list(OSD_Menu_channel_list_CB_t _callback, OSD_Channels_List_Type _channel_list_type);

    void process();
};

} // namespace mb
