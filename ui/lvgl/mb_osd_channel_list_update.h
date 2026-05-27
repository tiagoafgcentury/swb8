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

#include <map>

namespace mb {

class OSD_Translate;
class OSD_Instala_Facil_Antena;

class OSD_Channel_List_Update : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> channel_list_update_callback_t;

private:
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main               { nullptr };
    lv_obj_t *m_title               { nullptr };
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
    static constexpr auto footer_y =      643 - offset_y;
    static constexpr auto footer_h =      info_h;

    // Barra de progresso
    lv_obj_t *m_slider_label { nullptr };
    static constexpr auto slider_label_x = 610;
    static constexpr auto slider_label_y = 440 - offset_y;
    lv_obj_t *m_slider;
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
    Satellite_Operator m_operator;
    bool m_restart_after_update = true;

    Zone_ID_t m_local_zone_id;

#ifdef MBGUI_USE_RLOTTIE
    lv_obj_t *m_list_logo { nullptr };
#endif

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
        lv_color_t bar_color;
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
                tr(__Atualizacao_de_Lista_de_Canais),
                tr(__Atualizando_lista_de_canais_via_satelite_Por_favor_aguarde),
                "",
                OSD_COLOR_BLUE,
                100,
            }
        },
        {
            Status::Fail,
            {
                tr(__Nao_foi_possivel_concluir_a_atualizacao_da_lista_de_canais),
                "",
                tr(__Pressione_ok_para_continuar),
                OSD_COLOR_RED,
                250,
            }
        },
        {
            Status::Success,
            {
                tr(__Lista_de_canais_atualizada_com_sucesso),
                "",
                tr(__Pressione_ok_para_continuar),
                OSD_COLOR_BLUE,
                250,
            }
        }
    };

    // Botão da tela de detecção de LNBF
    MB_OSD_Keys keys;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto button_x = 530;
    static constexpr auto button_y = 570;
    static constexpr auto spacing = 0;

    channel_list_update_callback_t m_callback;

    void create_progress_bar();
    void draw_button();
    void start();
    void to_start();
    void success();
    void to_success();
    void fail();
    void to_fail();
    void populate(Status _status);

    std::shared_ptr<Event_List_Update> m_channel_list_update_callback;
    void channel_list_update_callback(bool _is_done);
    void channel_list_update_partial_callback(size_t _progress, const std::vector<Service>& /*_services*/);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Channel_List_Update *s_instance;
    std::atomic<Player::State> m_state;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Channel_List_Update(OSD *_parent);
    virtual ~OSD_Channel_List_Update();

    virtual void show_menu_channel_list_update(channel_list_update_callback_t _callback, bool restart = true);
    virtual void hide_menu();
    void refresh_progress();

    void process();
};

} // namespace mb
