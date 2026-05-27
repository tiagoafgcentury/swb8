#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_breadcrumb.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task_database.h"
#include "mb_osd_footer.h"
#include "mb_osd_select_satellite.h"
#include "mb_osd_enhanced_keys.h"
#include "mb_osd_terms_of_use.h"
#include "mb_osd_channel_list_update.h"
#include "mb_osd_message_box.h"
#include "mb_osd_service_table_progress.h"

#include <memory>
#include <array>

namespace mb {

class Fade_Canvas;

class Select_Switch : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> select_switch_callback_t;
    select_switch_callback_t m_callback;

private:
    std::unique_ptr<OSD_Message_Box> m_message_box;
    std::unique_ptr<OSD_Terms_of_Use> m_osd_terms_of_use;
    std::unique_ptr<Select_Satellite> m_select_satellite;
    std::shared_ptr<Event_Blind_Scan_Progress> m_post_event_blind_scan;
    std::weak_ptr<Event_Blind_Scan_Progress> m_event_weak;
    std::shared_ptr<Event_List_Update> m_channel_list_update_callback;
    std::unique_ptr<MB_OSD_Service_Table_Progress<>> m_service_table;

    lv_obj_t *m_main { nullptr };
    lv_obj_t *m_blank_screen { nullptr };
    lv_obj_t *m_warning_content { nullptr };
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 100;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT - offset_y;

    // Teclado
    MB_OSD_Enhanced_Keys m_switch_keys;
    static constexpr auto m_switch_keys_num = 4;
    static constexpr auto m_switch_keys_y = 200;
    static constexpr auto m_switch_keys_w = 220;
    static constexpr auto m_switch_keys_h = 50;
    static constexpr auto m_switch_keys_spacing = 20;
    static constexpr auto m_switch_keys_x = (width - m_switch_keys_num*m_switch_keys_w - (m_switch_keys_num-1)*m_switch_keys_spacing) / 2;

    static constexpr auto m_command_keys_num = 2;
    static constexpr auto m_command_keys_y = 400;
    static constexpr auto m_command_keys_w = 220;
    static constexpr auto m_command_keys_h = 50;
    static constexpr auto m_command_keys_spacing = 20;
    static constexpr auto m_command_keys_x = (width - m_command_keys_num*m_command_keys_w - (m_command_keys_num-1)*m_command_keys_spacing) / 2;

    size_t m_main_group = 0;
    size_t m_command_group = 0;
    int m_progress_step_value = 0;
    int m_progress_current_value = 0;
    uint8_t m_progress_last_value = 0;

    bool m_exit_all = false;

    // Funções para quando estiver selecionado opção de chaves
    std::map<Remote_Control_Key, MB_OSD_Enhanced_Keys::KeyCallback> m_switch_callbacks = {
        { Remote_Control_Key::KEY_VOLUP, std::bind(&Select_Switch::next_callback, this) },
        { Remote_Control_Key::KEY_VOLDOWN, std::bind(&Select_Switch::previous_callback, this) },
        { Remote_Control_Key::KEY_CHUP, std::bind(&Select_Switch::next_group_callback, this) },
        { Remote_Control_Key::KEY_CHDOWN, std::bind(&Select_Switch::previous_group_callback, this) },
        { Remote_Control_Key::KEY_VOLTAR, std::bind(&Select_Switch::voltar_callback, this) },
        { Remote_Control_Key::KEY_OK, std::bind(&Select_Switch::pick_satellite, this) },
    };

    // Funções para quando estiver selecionado opções de comando
    std::map<Remote_Control_Key, MB_OSD_Enhanced_Keys::KeyCallback> m_voltar_callbacks = {
        { Remote_Control_Key::KEY_VOLUP, std::bind(&Select_Switch::next_callback, this) },
        { Remote_Control_Key::KEY_VOLDOWN, std::bind(&Select_Switch::previous_callback, this) } ,
        { Remote_Control_Key::KEY_CHUP, std::bind(&Select_Switch::next_group_callback, this) },
        { Remote_Control_Key::KEY_CHDOWN, std::bind(&Select_Switch::previous_group_callback, this) },
        { Remote_Control_Key::KEY_VOLTAR, std::bind(&Select_Switch::voltar_callback, this) },
        { Remote_Control_Key::KEY_OK, std::bind(&Select_Switch::voltar_callback, this) },
    };

    std::map<Remote_Control_Key, MB_OSD_Enhanced_Keys::KeyCallback> m_buscar_callbacks = {
        { Remote_Control_Key::KEY_VOLUP, std::bind(&Select_Switch::next_callback, this) },
        { Remote_Control_Key::KEY_VOLDOWN, std::bind(&Select_Switch::previous_callback, this) } ,
        { Remote_Control_Key::KEY_CHUP, std::bind(&Select_Switch::next_group_callback, this) },
        { Remote_Control_Key::KEY_CHDOWN, std::bind(&Select_Switch::previous_group_callback, this) },
        { Remote_Control_Key::KEY_VOLTAR, std::bind(&Select_Switch::voltar_callback, this) },
        { Remote_Control_Key::KEY_OK, std::bind(&Select_Switch::buscar_callback, this) },
    };

    void next_callback();
    void previous_callback();
    void next_group_callback();
    void previous_group_callback();
    void voltar_callback() {
        //Task::post_event(m_callback);
        Task::post_event(std::bind(m_callback, m_exit_all));
    }
    void draw_switch_panel();
    void load_satellite_list_callback(const std::vector<Satellite> &sat);
    void pick_satellite();
    void buscar_callback();
    void buscar_confirm_callback(bool _result);
    void start_search();
    void terms_of_use_callback(bool _result);
    void blind_scan();
    void blind_scan_generic();
    uint32_t calculate_frequency(uint32_t _frequency, aui_nim_freq_band _band, aui_nim_polar _polarity);
    void finish_blind_scan(std::vector<Transponder> tp_list);
    void channel_list_update_callback(bool /*_is_done*/);
    void channel_list_partial_callback(size_t _transponder_seq, std::vector<Service> _services);
    void refresh_progress();
    void save_config_params();
    void update_satellite_list();
    void create_services_table();
    void start_switch_detection();
    bool start_sky_detection();
    bool start_claro_detection();
    bool signal_detected();
    void detection_process();
    void finish_detection();
    void satellite_error_callback();

    static constexpr auto diseq_1_0_switches_size = 4;
    std::vector<Satellite> m_switch_list;
    std::vector<Satellite> m_satellite_list;
    std::vector<Satellite> m_satellite_scan_list;
    Satellite m_dummy_satellite = {(uint16_t)-1, "", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 0, false};
    size_t m_satellite_scan_index = 0 ;
    std::vector<Transponder> m_detected_transponders;
    uint32_t m_total_transponders_found = 0;
    int32_t m_current_progress = 0;
    bool m_clear_table = true;
    bool m_emit_lineup_ready = true;
    Satellite m_satellite;
    lv_timer_t* m_refresh_timer{nullptr};
    Transponder m_transponder;

    lv_timer_t* m_detection_timer{nullptr};
    bool m_sky_detected = false;
    bool m_claro_detected = false;
    bool m_sky_detecting = false;
    bool m_claro_detecting = false;
    bool m_is_searching = false;
    bool m_detection_error = true;

    typedef enum {
        DETECTION_IDLE,
        DETECTING_SKY,
        DETECTING_CLARO,
        DETECTION_ERROR,
        SKY_DETECTED,
        CLARO_DETECTED,
        DETECTION_FINISHED
    } m_detection_status_t;
    m_detection_status_t m_detection_status = DETECTION_IDLE;
    lv_obj_t *m_footer { nullptr };

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    static Select_Switch* s_instance;

public:
    Select_Switch(OSD *_parent);
    virtual ~Select_Switch();
    void show_switch_list(select_switch_callback_t _callback);
};

}
