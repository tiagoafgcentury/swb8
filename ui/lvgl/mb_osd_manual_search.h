#pragma once

#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "mb_osd_translate.h"
#include "mb_osd_keys.h"

#include <functional>
#include <vector>

namespace mb {

class OSD_Manual_Search : public OSD, public Remote_Control_Handler
{
public:
    using manual_search_callback_t = std::function<void(bool)>;

private:
    lv_obj_t *m_bgd { nullptr };
    manual_search_callback_t m_callback;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 140;    
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT - offset_y;
	
    void close_screen(bool _result);
    void lock_transponder();

    void draw_keys();
    MB_OSD_Keys m_keys;
    static constexpr auto keys_w = 220;
    static constexpr auto keys_h = 50;
    static constexpr auto keys_y = 80;
    static constexpr auto x_spacing = 40;
    static constexpr auto keys_x_spacing = keys_w + x_spacing;
    static constexpr auto keys_y_spacing = 80;
    static constexpr auto keys_x = width/2 - keys_w - x_spacing/2;
    static constexpr auto buscar_keys_x = width/2 - keys_w/2;
    static constexpr auto buscar_keys_y = keys_y + keys_y_spacing + 2*keys_h;

    std::vector<Satellite> m_satellites;
    size_t m_selected_satellite = 0;
    Polarity m_polarity = Polarity::Horizontal;
    uint32_t m_frequency = 0;
    uint32_t m_symbol_rate = 0;
    bool m_refresh_signal_info = false;
    Remote_Control_Key m_pressed_key = Remote_Control_Key::KEY_UNDEFINED;
    Transponder m_tp;

    lv_timer_t* m_beep_timer { nullptr };
    double m_last_snr { 0.0 };
    int m_last_range { -1 };
    bool m_beep_on { false };

    void start_refresh_timer();
    lv_timer_t  *m_refresh_timer { nullptr };

    void create_quality_slider();
    lv_obj_t *m_quality_slider { nullptr };
    lv_obj_t *m_quality_line { nullptr };
    lv_style_t m_quality_main;
    lv_style_t m_quality_indicator;

    void create_strength_slider();
    lv_obj_t *m_strength_slider { nullptr };
    lv_obj_t *m_strength_line { nullptr };
    lv_style_t m_strength_main;
    lv_style_t m_strength_indicator;

    static constexpr auto slider_width = 530;
    static constexpr auto slider_height = 20;

    void create_snr_box();
    lv_obj_t *m_snr_box { nullptr };
    lv_obj_t *m_snr_title { nullptr };
    lv_obj_t *m_snr_value { nullptr };
    static constexpr auto snr_box_width = 180;
    static constexpr auto snr_box_height = 80;
    static constexpr auto snr_box_x = slider_width/2 + snr_box_width - 20;
    static constexpr auto snr_box_y = 140;

    lv_timer_t* m_lock_timer { nullptr };
    void start_lock_timer();

    void refresh_signal_info();

    void process_beep();
    static void beep_cb(lv_timer_t* tm);


    enum class Key_Index
    {
        Satellite_Name = 0,
        Polarity = 1,
        Frequency = 2,
        Symbol_Rate = 3,
        Buscar = 4
    };
    void on_satellite_callback();
    void on_polarity_callback();
    void on_frequency_callback();
    void on_symbol_rate_callback();
    void on_buscar_callback();
    void not_implemented_response(Remote_Control_Key _key);

    std::shared_ptr<Event_List_Update> m_channel_list_update_callback;
    void channel_list_update_callback(bool);
    void channel_list_update_partial_callback(size_t, std::vector<Service>);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Manual_Search(OSD *_parent);
    virtual ~OSD_Manual_Search();

    void show_manual_search_list(manual_search_callback_t _callback);
};

} // namespace mb
