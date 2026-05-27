#include "mb_osd_channel_list_update.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_osd_footer.h"
#include "mb_zone_id.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"
#include "mb_osd_keys.h"

#include "hal/mb_sound.h"

#include <lvgl.h>
#include <stdio.h>


namespace mb {

OSD_Channel_List_Update *OSD_Channel_List_Update::s_instance { nullptr };

OSD_Channel_List_Update::OSD_Channel_List_Update(OSD *_parent):
    OSD(_parent),
    keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_current_progress = 0;
    s_instance = this;
}

OSD_Channel_List_Update::~OSD_Channel_List_Update()
{
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_main);
    s_instance = nullptr;
    Osd_Breadcrumb::s_instance.remove_name();
}

bool OSD_Channel_List_Update::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Durante a atulização da lista de canais, não aceita eventos do controle remoto
    if (m_status == Status::Start)
    {
        return true;
    }

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
            if (m_status == Status::Start)
            {
                to_fail();
            }
            else if (m_status == Status::Fail)
            {
                Task::post_event(std::bind(m_callback, false));
            }
            else if (m_status == Status::Success)
            {
                lv_obj_remove_flag(m_main, LV_OBJ_FLAG_HIDDEN);
                Task::post_event(std::bind(m_callback, true));
            }

            return true;

        case Remote_Control_Key::KEY_VOLUP:
        {
            return true;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            return true;
        }

        case Remote_Control_Key::KEY_CHUP:
        case Remote_Control_Key::KEY_CHDOWN:
            return true;

        default:
            return true;
    }

    return true;
}

void OSD_Channel_List_Update::show_menu_channel_list_update(channel_list_update_callback_t _callback, bool restart)
{
    if (m_main)
    {
        DEBUG_MSG(OSD, WARN, "show_menu_channel_list_update already running, ignoring\n");
        return;
    }

    m_restart_after_update = restart;
    if (m_restart_after_update)
    {
        DEBUG_MSG(OSD, DEBUG, TERM_GREEN_BOLD "Restart after update\n" TERM_RESET);
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, TERM_YELLOW_BOLD "No restart after update\n" TERM_RESET);
    }

    set_focus();
    m_callback = _callback;
    auto network_id = Config::get_config()->selected_satellite_config().network_id;
    m_operator = network_id == Network_Id_Sky ? Satellite_Operator::Sky : 
                network_id == Network_Id_Claro ? Satellite_Operator::Claro : Satellite_Operator::Generic;
    m_local_zone_id = Zone_ID::get_zone_id(m_operator);
    //MainMenu
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    lv_obj_move_foreground(m_main);

    // Cria timer para 10 segundos
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create([](lv_timer_t *tm) {
            auto instance = static_cast<OSD_Channel_List_Update *>(lv_timer_get_user_data(tm));
            instance->refresh_progress();
        }, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
        lv_timer_ready(m_refresh_timer);
    }

#ifdef MBGUI_USE_RLOTTIE
    if (not m_list_logo)
    {
        m_list_logo = lv_rlottie_create_from_file(m_main, 200, 200, ANIM_CHANNEL_LIST);
        lv_obj_null_on_delete(&m_list_logo);
    }

    lv_rlottie_set_play_mode(m_list_logo, LV_RLOTTIE_CTRL_LOOP);
    lv_obj_align(m_list_logo, LV_ALIGN_TOP_MID, 0, 270);
#endif

    // Criar barra de progresso
    create_progress_bar();
    using namespace std::placeholders;
    m_channel_list_update_callback = std::make_shared<Event_List_Update>(Event_List_Update{ 
        .callback = std::bind(&OSD_Channel_List_Update::channel_list_update_callback, this, _1),
        .partial_callback = std::bind(&OSD_Channel_List_Update::channel_list_update_partial_callback, this, _1, _2)
    });
    Task::post_event_lineup_build(m_channel_list_update_callback, m_restart_after_update);
    // Popula tela
    to_start();
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Atualizacao_de_Lista_de_Canais));
}

void OSD_Channel_List_Update::channel_list_update_partial_callback(size_t _progress, const std::vector<Service>& /*_services*/)
{
    DEBUG_MSG(OSD, WARN, TERM_BLUE_BOLD "Progress: " << _progress << "\n" TERM_RESET);
    if ( m_status != Status::Start or 
        m_operator != Satellite_Operator::Sky or
        _progress == m_current_progress)
    {
        return;
    }

    m_current_progress = _progress;
    auto progress = std::min(m_current_progress, 100);
    lv_slider_set_value(m_slider, progress, LV_ANIM_OFF);
    lv_label_set_text_fmt(m_slider_label, "%d%%", progress);
}

void OSD_Channel_List_Update::channel_list_update_callback(bool /*_is_done*/)
{
    DEBUG_MSG(OSD, DEBUG, "Callback done\n");
    lv_slider_set_value(m_slider, 100, LV_ANIM_OFF);
    lv_label_set_text(m_slider_label, "100%");
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto number_of_services_found = current_lineup->services.size();
    DEBUG_MSG(OSD, DEBUG, "NUMBER OF SERVICES FOUND: " << number_of_services_found << "\n");

#ifdef MBGUI_USE_RLOTTIE
    if (m_list_logo)
    {
        lv_rlottie_set_play_mode(m_list_logo, LV_RLOTTIE_CTRL_PAUSE);
        lv_obj_add_flag(m_list_logo, LV_OBJ_FLAG_HIDDEN);

        if (number_of_services_found > 0)
        {
            DELETE_OBJ(m_list_logo);
            m_list_logo = lv_rlottie_create_from_file(m_main, 200, 200, ANIM_OK);
            lv_obj_null_on_delete(&m_list_logo);
            lv_rlottie_set_play_mode(m_list_logo, LV_RLOTTIE_CTRL_LOOP);
            lv_obj_align(m_list_logo, LV_ALIGN_TOP_MID, 0, 180);
        }
    }
#endif

    if (number_of_services_found > 0)
    {
        Sound::get_instance()->mute_state();
        m_status = Status::Success;
    }
    else
    {
        m_status = Status::Fail;
    }
}

void OSD_Channel_List_Update::create_progress_bar()
{
    /*Create a slider in the center of the display*/
    m_slider = lv_slider_create(m_main);
    lv_obj_null_on_delete(&m_slider);
    lv_obj_set_pos(m_slider, slider_x, slider_y);
    lv_obj_set_size(m_slider, slider_w, slider_h);
    lv_obj_set_style_anim_duration(m_slider, 2000, 0);
    /*Create a label below the slider*/
    m_slider_label = set_label_text(m_main, "", slider_label_x, slider_label_y, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_slider_label);
    lv_label_set_text(m_slider_label, "");
    // Set the indicator color to green
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_BLUE, LV_PART_INDICATOR);
    // Set the background color to light gray
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREY, LV_PART_MAIN);
    // Make the knob completely transparent
    lv_obj_set_style_bg_opa(m_slider, LV_OPA_TRANSP, LV_PART_KNOB);
}

void OSD_Channel_List_Update::refresh_progress()
{

    if (!m_slider && m_status == Status::Start)
    {
        return;
    }

    switch (m_status)
    {
        case Status::Start:
            start();
            break;

        case Status::Fail:
            fail();
            break;

        case Status::Success:
            success();
            break;

        case Status::COUNT:
            DEBUG_MSG(OSD, ERROR, "Invalid status for progress refresh\n");
            break;
    }
}

void OSD_Channel_List_Update::success()
{
    to_success();
}

void OSD_Channel_List_Update::start()
{
    if ( m_current_progress <= 100 and
        m_operator != Satellite_Operator::Sky)
    {
        m_current_progress += 2;
        auto progress = std::min(m_current_progress, 100);
        lv_slider_set_value(m_slider, progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(m_slider_label, "%d%%", progress);
    }
}

void OSD_Channel_List_Update::fail()
{
    to_fail();
}

void OSD_Channel_List_Update::populate(Status st)
{
    m_status = st;
    const auto &sc = m_screen_content[st];
    // Título
    DELETE_OBJ(m_title);
    m_title = set_label_text(m_main, sc.title, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    // Subtítulo
    DELETE_OBJ(m_subtitle);
    m_subtitle = set_label_text(m_main, sc.subtitle, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, subtitle_y);
    // Rodapé
    DELETE_OBJ(m_footer);
    m_footer = MB_OSD_Footer::draw(m_main, sc.footer, -40);
    lv_obj_null_on_delete(&m_footer);

    if (m_status == Status::Start)
    {
        // Seta cor do slider
        lv_obj_set_style_bg_color(m_slider, sc.bar_color, LV_PART_INDICATOR);
    }

    // Altera status e redesenha teclas
    lv_timer_set_period(m_refresh_timer, sc.period);
}

void OSD_Channel_List_Update::to_start()
{
    populate(Status::Start);
}

void OSD_Channel_List_Update::to_success()
{
    populate(Status::Success);
    m_current_progress = 10;
    DELETE_OBJ(m_slider_label);
    DELETE_OBJ(m_slider);
    draw_button();
}

void OSD_Channel_List_Update::to_fail()
{
    populate(Status::Fail);
    m_current_progress = 10;
    DELETE_OBJ(m_slider_label);
    DELETE_OBJ(m_slider);
    draw_button();
}

void OSD_Channel_List_Update::draw_button()
{
    auto lbl_text = tr(__Proximo);

    if ((m_status == Status::Success && m_local_zone_id > 0) or
            (m_status == Status::Fail))
    {
        lbl_text = tr(__Finalizar);
    }

    keys.clear();
    keys.set_background(m_main);
    keys.add_label(lbl_text);
    keys.draw();
    keys.select();
}

void OSD_Channel_List_Update::hide_menu()
{
    remove_focus();
    Task::post_event(std::bind(m_callback, false));
}

void OSD_Channel_List_Update::got_focus()
{
}

} // namespace mb
