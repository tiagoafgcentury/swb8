#pragma once

#include "mb_osd.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"
#include "mb_events.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "tasks/mb_task_cas.h"
#include <map>

namespace mb {

class OSD_Translate;
class OSD_Instala_Facil_Antena;

class OSD_Activate : public OSD, public Remote_Control_Handler
{

public:
    typedef std::function<void(bool)> Activate_CB_t;

private:
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main               { nullptr };
    lv_obj_t *m_title               { nullptr };
    static constexpr auto title_y = 180 - offset_y;
    static constexpr auto title_w = 744;
    static constexpr auto title_h = 53;

    lv_obj_t *m_logo_plus               { nullptr };
    lv_obj_t *m_logo_b8               { nullptr };

    lv_obj_t *m_subtitle                 { nullptr };
    static constexpr auto subtitle_x = 214;
    static constexpr auto subtitle_y = 247 - offset_y;
    static constexpr auto subtitle_h = 66;
    static constexpr auto subtitle_w = 862;

    lv_obj_t *m_info                     { nullptr };
    lv_obj_t *m_info_box                 { nullptr };
    static constexpr auto info_y =      380 - offset_y;
    static constexpr auto info_h =      27;

    lv_obj_t *m_instructions { nullptr };

    static constexpr auto instructions_x = 350;
    static constexpr auto instructions_y = 353 - offset_y;
    static constexpr auto instructions_w = 198;
    static constexpr auto instructions_h = 120;
    static constexpr auto caid_y = 417 - offset_y;
    static constexpr auto scua_y = 447 - offset_y;

    lv_obj_t *m_caid { nullptr };
    lv_obj_t *m_scua { nullptr };
    lv_obj_t *m_point { nullptr };

    static constexpr auto point_x = 840;
    static constexpr auto point_y = 376 - offset_y;
    static constexpr auto point_w = 176;
    static constexpr auto point_h = 81;

    lv_obj_t *m_line { nullptr };
    static constexpr auto line_x = 640;
    static constexpr auto line_y = 353 - offset_y;
    static constexpr auto line_w = 3;
    static constexpr auto line_h = DISPLAY_HEIGHT - 353 - 240;

    lv_obj_t *m_or { nullptr };
    static constexpr auto or_x = 629;
    static constexpr auto or_y = 400 - offset_y;
    static constexpr auto or_w = 27;
    static constexpr auto or_h = 27;

    lv_obj_t *m_qrcode { nullptr };
    static constexpr auto box_x = 676;
    static constexpr auto box_y = 353 - offset_y;
    static constexpr auto box_w = 126;
    static constexpr auto box_h = 126;

    lv_obj_t *m_footer                     { nullptr };
    static constexpr auto footer_y =      643 - offset_y;
    static constexpr auto footer_h =      info_h;

    enum class Status
    {
        Start,
        Fail,
        Success,
        COUNT
    };
    Status m_status = Status::Start;

    struct Screen_Content
    {
        std::string_view title;
        std::string_view subtitle;
        std::string_view footer;
        int period;
    };

    // cria screen_content_map
    typedef std::map<Status, Screen_Content> Screen_Content_Map;
    // popula screen_content_map
    Screen_Content_Map m_screen_content =
    {
        {
            Status::Start,
            {
                tr(__Ative_o_seu_Midiabox),
                tr(__Para_ativar_seu_midiabox_siga_as_instrucoes_abaixo_ou_use_o_qrcode),
                tr(__Ative_o_seu_Midiabox),
                1000,
            }
        },
        {
            Status::Fail,
            {
                tr(__Nao_foi_possivel_concluir_a_ativacao),
                "",
                tr(__Pressione_ok_para_continuar),
                1000,
            }
        },
        {
            Status::Success,
            {
                "",
                "",
                tr(__Pressione_ok_para_continuar),
                1000,
            }
        }
    };

    // Botão da tela de detecção de LNBF
    MB_OSD_Keys m_keys;
    lv_obj_t *m_button_next { nullptr };
    lv_obj_t *m_button_label { nullptr };
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto button_x = 530;
    static constexpr auto button_y = 570;
    static constexpr auto spacing = 0;

    Activate_CB_t m_callback;

    void draw_button();
    void start();
    void to_start();
    void success();
    void to_success();
    void fail();
    void to_fail();
    void to_detect();
    void populate(Status);

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_scua_cb;
    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Activate *s_instance;
    std::atomic<Player::State> m_state;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Activate(OSD *_parent);
    virtual ~OSD_Activate();

    virtual void show_menu_activate(Activate_CB_t _callback, bool _stb_activated);
    virtual void hide_menu();
    void refresh_progress();

    void process();
};

} // namespace mb
