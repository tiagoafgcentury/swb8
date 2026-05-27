#pragma once

#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_activate.h"
#include "mb_osd_channel_list_update.h"
#include "mb_osd_activate.h"

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "common/mb_satellites.h"

#include "mb_remote_control_handler.h"
#include "mb_events.h"
#include "mb_osd_keys.h"

#include "tasks/mb_task.h"

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace mb {

class OSD_Translate;

class OSD_Software_Update : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Software_Update_CB_t;

private:
    std::unique_ptr<OSD_Activate> mb_osd_activate;
    std::unique_ptr<OSD_Channel_List_Update> m_osd_channel_list_update;
    OTA_TS_PID_List m_tp_params;

    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main               { nullptr };
    lv_obj_t *m_bgd_main           { nullptr };
    lv_obj_t *m_title              { nullptr };
    static constexpr auto title_x = 0;
    static constexpr auto title_y = 280 - offset_y;
    static constexpr auto title_h = 53;

    lv_obj_t *m_logo_plus               { nullptr };
    lv_obj_t *m_subtitle                 { nullptr };
    static constexpr auto subtitle_x = 214;
    static constexpr auto subtitle_y = 347 - offset_y;
    static constexpr auto subtitle_h = 66;
    static constexpr auto subtitle_w = 862;

    lv_obj_t *m_info                     { nullptr };
    lv_obj_t *m_info_box                 { nullptr };
    static constexpr auto info_y =      380 - offset_y;
    static constexpr auto info_h =      27;

    lv_obj_t *m_footer                     { nullptr };
    static constexpr auto footer_y =      -40;
    static constexpr auto footer_h =      info_h;

    // Barra de progresso
    lv_obj_t *m_slider_label { nullptr };
    static constexpr auto slider_label_x = 610;
    static constexpr auto slider_label_y = 440 - offset_y;
    lv_obj_t *m_slider { nullptr };
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
    lv_timer_t  *m_sw_update_timer { nullptr };
    std::string m_strFile;
    int m_current_progress = 0;
    int m_detection_time = 0;
    bool m_updt_sattelite = false;
    static void refresh_cb(lv_timer_t *_tm);
    static void force_sw_update_cb(lv_timer_t *_tm);

    static constexpr auto s_tp_lock_timeout = 5;
    static constexpr auto s_ota_timeout = 22;
    uint32_t m_tp_lock_current_time = 0;
    uint32_t m_ota_current_time = 0;
    uint32_t m_sky_timer = 0;

    size_t m_tp_pointer = 0;

    enum class Status
    {
        TP_Init,          // carrega lista de transponder
        TP_Lock,          // Tenta locar um tp
        TP_Locked,        // Transponder atual locado
        TP_Lock_Fail,     // Falha ao locar tp atual
        OTA_Detect,       // Inicia detecção do ota
        OTA_Detected,     // ota detectado
        OTA_Detect_Fail,  // falha na detecção do ota
        Fail,             // falha na detecção do ota
        Success,          // Sucesso na instalação do ota
        USB,              // Atualização via USB
        Updated,          // Software atualizado
        Sky_Init,         // Atualização via satélite Sky
    };
    Status m_status = Status::TP_Init;

    struct Screen_Content
    {
        std::string_view title;
        std::string_view subtitle;
        std::string_view footer;
    };

    // cria screen_content_map
    typedef std::map<Status, Screen_Content> Screen_Content_Map;
    Screen_Content_Map m_screen_content =
    {
        {
            Status::TP_Lock,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Procurando_transponder),
                "",
            }
        },
        {
            Status::TP_Init,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Sintonizando_novo_transponder),
                "",
            }
        },
        {
            Status::TP_Locked,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Transponder_encontrado),
                "",
            }
        },
        {
            Status::OTA_Detect,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Procurando_atualizacao),
                "",
            }
        },
        {
            Status::OTA_Detected,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Encontrada_atualizacao_de_programa),
                "",
            }
        },
        {
            Status::OTA_Detect_Fail,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Nao_foi_encontrada_atualizacao_no_transponder_atual),
                "",
            }
        },
        {
            Status::TP_Lock_Fail,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Transponder_nao_encontrado),
                "",
            }
        },
        {
            Status::Fail,
            {
                tr(__Nao_foi_possivel_concluir_a_atualizacao_do_software),
                "",
                tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar),
            }
        },
        {
            Status::Success,
            {
                tr(__Software_atualizado_com_sucesso),
                "",
                tr(__Pressione_ok_para_continuar),
            }
        },
        {
            Status::USB,
            {
                tr(__Atualizacao_de_software_via_usb),
                tr(__Procurando_o_arquivo_para_atualizacao_por_favor_aguarde),
                tr(__Pressione_voltar_para_voltar),
            }
        },
        {
            Status::Updated,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Seu_receptor_esta_com_software_atualizado),
                tr(__Pressione_ok_para_continuar),
            }
        },
        {
            Status::Sky_Init,
            {
                tr(__Atualizacao_de_software_via_satelite),
                tr(__Procurando_o_arquivo_para_atualizacao_por_favor_aguarde),
                "",
            },
        }
    };
    std::string m_sc_fail;

    // Botão da tela de detecção de LNBF
    lv_obj_t *m_button_next { nullptr };
    lv_obj_t *m_label_next { nullptr };
    static constexpr auto next_w = 220;
    static constexpr auto next_h = 50;
    static constexpr auto next_x = 530;
    static constexpr auto next_y = 570 - offset_y;

    MB_OSD_Keys m_options;
    static constexpr auto option_x = 369;
    static constexpr auto option_y = 570;;
    static constexpr auto option_w = 260;
    static constexpr auto option_h = 50;
    static constexpr auto option_s = 283;

    bool m_is_more_informations = false;
    bool m_is_easy_install = false;

    Software_Update_CB_t m_callback;
    std::shared_ptr<Event_OTA_DSI> m_ota_callback;

    void create_progress_bar();
    void draw_button();
    void draw_buttons();
    void populate(Status);
    void tp_init();
    void tp_lock();
    void tp_locked();
    void tp_lock_fail();
    void ota_detect();
    void ota_detect_fail();
    void to_success();
    void to_fail();
    void to_usb();
    void looking_for_channel_list_update();
    void create_more_informations_screen();
    bool is_pendrive_mounted();
    void stop_timer(bool _result);
    void process_ota_sky();

    std::vector<std::string> list_usb_files(std::string_view _path);
    std::vector<std::string> m_files;

    void show_menu_channel_list_update_callback();
    void show_menu_activate_callback(bool _result);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    static OSD_Software_Update *s_instance;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Software_Update(OSD *_parent);
    virtual ~OSD_Software_Update();

    virtual void show_menu_software_update(Software_Update_CB_t _callback, bool _satellite, bool _is_easy_install);
    virtual void hide_menu();
    void refresh_progress();

    void process_ota(uint32_t _product, uint16_t _sw, uint16_t _sw_min);
    static void process_ota_cb(uint32_t _product, uint16_t _sw, uint16_t _sw_min);

};

} // namespace mb
