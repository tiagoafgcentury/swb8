#pragma once

#include "mb_osd.h"
#include "mb_menu_resources.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_osd_translate.h"
#include "mb_events.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_tuner.h"
#include "tasks/mb_task_player.h"
#include "mb_osd_keys.h"
#include "mb_remote_control_handler.h"
#include "mb_lnbf_detection.h"

#include <map>

namespace mb {

class OSD_Sat_Detect_Lnbf : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(void)> Sat_Detect_Lnbf_CB_t;

private:
    std::unique_ptr<MB_Detect_Lnbf> m_detect_lnbf;

    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT;
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;;

    static constexpr auto cover_w = 600;
    static constexpr auto cover_h = 426;

    lv_obj_t *m_main_screen { nullptr };
    lv_obj_t *m_line { nullptr };
    lv_obj_t *m_prgbar { nullptr };
    lv_obj_t *m_pgr_label { nullptr };

    lv_obj_t *m_info_box { nullptr };
    lv_obj_t *m_info1 { nullptr };
    lv_obj_t *m_info2 { nullptr };

    lv_obj_t *m_title { nullptr };
    lv_obj_t *m_subtitle { nullptr };
    lv_obj_t *m_footer { nullptr };

    static constexpr auto prgbar_w = 530;
    static constexpr auto prgbar_h = 20;

    static constexpr auto info_w = 530;
    static constexpr auto info_h = 50;

    lv_style_t  m_style;
    lv_style_t  m_style_progbar;

    Satellite m_satellite;

    lv_timer_t  *m_refresh_timer { nullptr };

    enum class Status
    {
        Detect,
        Fail,
        Success,
    };
    Status m_status = Status::Detect;

    struct Screen_Content
    {
        std::string_view title;
        std::string_view subtitle;
        std::string_view footer;
        lv_color_t  bar_color;
        int period = 0;
    };

    // cria screen_content_map
    typedef std::map<Status, Screen_Content> Screen_Content_Map;
    // popula screen_content_map
    Screen_Content_Map m_screen_content =
    {
        {
            Status::Detect,
            {
                tr(__Detectando_tipo_de_LNBF_da_sua_antena),
                tr(__Por_favor_aguarde),
                "",
                OSD_COLOR_BLUE,
                1000,
            }
        },
        {
            Status::Fail,
            {
                tr(__Nao_foi_possivel_detectar_o_lnbf),
                ""/*tr(__Nova_tentativa_em_xxx_segundos_pressione_ok_para_retornar_ao_menu_anterior)*/,
                tr(__Pressione_ok_para_continuar),
                OSD_COLOR_RED,
                1000,
            }
        },
        {
            Status::Success,
            {
                tr(__LNBF_detectado_com_sucesso),
                ""/*tr(__Pressione_ok_para_continuar)*/,
                tr(__Pressione_ok_para_continuar),
                OSD_COLOR_BLUE,
                1000,
            }
        }
    };

    MB_OSD_Keys m_keys;
    static constexpr auto btn_x = 150;
    static constexpr auto btn_y = 350;//570;
    static constexpr auto btn_w = 260;
    static constexpr auto btn_h = 50;
    static constexpr auto btn_s = 0;

    Sat_Detect_Lnbf_CB_t m_callback;
    int m_current_progress = 0;
    static void refresh_cb(lv_timer_t *_tm);
    void create_progress_bar();
    void print_satellite_config();
    void detect();
    void success();
    void fail();
    void looking_for_sat_config();
    void to_detect();
    void to_success();
    void to_fail();
    void populate(Status _status);
    void draw_buttons();

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
    OSD_Sat_Detect_Lnbf(OSD *_parent);
    virtual ~OSD_Sat_Detect_Lnbf();

    void osd_sat_detect_lnbf(Sat_Detect_Lnbf_CB_t _callback, lv_area_t _area, Satellite _sat);
    void refresh_progress();
};

} // namespace mb
