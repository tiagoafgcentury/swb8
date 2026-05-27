#pragma once

#include "mb_osd.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task.h"
#include "mb_events.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "mb_osd_keys.h"
#include "hal/mb_sound.h"
#include "mb_lnbf_detection.h"

#include <map>

namespace mb {

class OSD_Translate;

class OSD_Lnbf_Detection : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool, Transponder_Id)> osd_lnbf_detection_callback_t;
    osd_lnbf_detection_callback_t m_callback;

private:
    std::unique_ptr<MB_Detect_Lnbf> m_detect_lnbf;

    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main                { nullptr };
    lv_obj_t *m_title               { nullptr };
    static constexpr auto title_y = 0;
    static constexpr auto title_h = 53;

    lv_obj_t *m_logo_plus           { nullptr };

    lv_obj_t *m_bgd_subtitle        { nullptr };
    lv_obj_t *m_subtitle            { nullptr };
    static constexpr auto subtitle_x = 214;
    static constexpr auto subtitle_y = 260 - offset_y;
    static constexpr auto subtitle_h = 66;
    static constexpr auto subtitle_w = 852;

    lv_obj_t *m_snr                 { nullptr };
    static constexpr auto snr_x = 929;
    static constexpr auto snr_y = 440 - offset_y;
    static constexpr auto snr_h = 33;
    static constexpr auto snr_w = 180;

    lv_obj_t *m_info                { nullptr };
    lv_obj_t *m_info_box            { nullptr };
    static constexpr auto info_y =  380 - offset_y;
    static constexpr auto info_h =  27;

    lv_obj_t *m_footer              { nullptr };
    static constexpr auto footer_y =      643 - offset_y;
    static constexpr auto footer_h =      info_h;

    // Barra de progresso
    lv_obj_t *m_slider_label        { nullptr };
    static constexpr auto slider_label_x = 507;
    static constexpr auto slider_label_y = 440 - offset_y;

    lv_obj_t *m_slider              { nullptr };
    static constexpr auto slider_y =      418 - offset_y;
    static constexpr auto slider_w =      910;
    static constexpr auto slider_x = (width - slider_w) / 2;
    static constexpr auto slider_h =      20;
    lv_obj_t *m_progress_line { nullptr };
    lv_obj_t *m_progress_slider { nullptr };
    lv_style_t m_progress_main;
    lv_style_t m_progress_indicator;
    lv_style_t m_progress_knob;
    static constexpr auto width_event = 728;
    lv_style_transition_dsc_t m_transition_dsc;
    lv_timer_t  *m_refresh_timer { nullptr };
    int m_current_progress = 0;
    static void refresh_cb(lv_timer_t *_tm);

    static void beep_cb(lv_timer_t* t);
    void process_beep();

    lv_timer_t* m_beep_timer {nullptr};

    double   m_last_snr {0};
    bool     m_beep_on {false};
    uint32_t m_beep_elapsed {0};

    enum class Status
    {
        Detect,
        Fail,
        To_Fail,
        Success,
        To_Success,
        Point,
    };
    Status m_status = Status::Detect;

    struct Screen_Content
    {
        std::string_view title;
        std::string_view subtitle;
        std::string_view footer;
        lv_color_t bar_color;
        int period;
    };

    // cria screen_content_map
    typedef std::map<Status, Screen_Content> Screen_Content_Map;
    // popula screen_content_map
    Screen_Content_Map m_screen_content =
    {
        {
            Status::Detect,
            {
                tr(__Deteccao_do_LNBF),
                tr(__Detectando_tipo_de_LNBF_da_sua_antena),
                "",
                OSD_COLOR_BLUE,
                300,
            }
        },
        {
            Status::Fail,
            {
                tr(__Nao_foi_possivel_detectar_o_lnbf),
                tr(__Nova_tentativa_em_xxx_segundos),
                tr(__Pressione_ok_para_continuar),
                OSD_COLOR_RED,
                1000,
            }
        },
        {
            Status::Success,
            {
                tr(__LNBF_detectado_com_sucesso),
                "",
                tr(__Pressione_ok_para_continuar),
                OSD_COLOR_BLUE,
                1000,
            }
        },
        {
            Status::Point,
            {
                tr(__Aponte_sua_antena),
                tr(__Ajuste_antena_utilizando_medidor),
                tr(__Pressione_a_tecla_mais_para_alterar_o_tipo_de_lnbf),
                OSD_COLOR_GREEN,
                1000,
            }
        }
    };

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto btn_x = 530;
    static constexpr auto btn_y = 530;
    static constexpr auto btn_w = 220;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_s = 0;

    void draw_buttons();

    Satellite m_satellite;
    void create_progress_bar();
    void detect();
    void to_success();
    void to_fail();
    void to_point();
    void to_detect();
    void populate(Status _status);
    void success();
    void fail();
    void point();
    void update_subtitle();
    void save_config_params();
    void lnbf_detection();
    void lnbf_detection_callback(bool _result, Transponder_Id _tp);
    void set_lnbf_type();

    Player::State m_state;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Lnbf_Detection *s_instance;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Lnbf_Detection(OSD *_parent);
    virtual ~OSD_Lnbf_Detection();

    virtual void show_menu_lnbf_detection(osd_lnbf_detection_callback_t _callback, Satellite _sat);
    virtual void hide_menu();
    void refresh_progress();
    void update_info(Event_Transponder_data _progress);
};

} // namespace mb
