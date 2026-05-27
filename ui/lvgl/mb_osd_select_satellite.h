#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_osd_edit_delete_satellite.h"
#include "mb_osd_confirm_delete_satellite.h"
#include "mb_osd_breadcrumb.h"

#include "tasks/mb_task_database.h"

#include <memory>
#include <array>

namespace mb {

class Select_Satellite :  public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(Satellite, bool)> select_satellite_callback_t;

private:
    std::unique_ptr<Confirm_Delete_Satellite> m_confirm_delete;

    static constexpr auto start_y = 308 - 147;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - 147;
    static constexpr auto MAX_SATELLITE = 12;

    lv_obj_t *m_mainscreen { nullptr };
    lv_obj_t *m_bgd_fade { nullptr };

    MB_OSD_Keys m_keys;
    static constexpr auto keys_w = 220;
    static constexpr auto keys_h = 50;
    static constexpr auto keys_y = 166;
    static constexpr auto keys_x = 147;
    static constexpr auto x_spacing = 40;
    static constexpr auto keys_x_spacing = keys_w + x_spacing;
    static constexpr auto keys_y_spacing = 80;
    static constexpr auto voltar_keys_x = width/2 - keys_w/2;
    static constexpr auto voltar_keys_y = 570 - start_y;
    Remote_Control_Key m_pressed_key = Remote_Control_Key::KEY_UNDEFINED;

    unsigned int m_selected_satellite { 0 };
    std::vector<Satellite> m_satellites;
    Satellite m_dummy_satellite = {0, "Novo satélite", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 0, false};

    lv_obj_t *m_footer_plus { nullptr };
    lv_obj_t *m_footer { nullptr };
    static constexpr auto instruction_w = 1020;
    static constexpr auto instruction_h = 27;
    static constexpr auto instruction_x = 140;
    static constexpr auto instruction_y = 641 - start_y;

    void on_voltar_callback();
    void on_satellite_callback();
    void confirm_delete_satellite_callback(bool _confirm_ok);

    select_satellite_callback_t m_callback;

protected:
    static Select_Satellite *s_instance;

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
public:
    Select_Satellite(OSD *_parent);
    virtual ~Select_Satellite();

    void show_select_satellite(select_satellite_callback_t _callback);
    void process_satellite_list();
    void load_satellite_list();
};

} // namespace mb
