#include "mb_task_osd.h"

#include "common/mb_state_file.h"
#include "hal/mb_display.h"
#include "mb_zone_id.h"
#include "mb_main.h"
#include "tasks/mb_task_eit_events.h"

#include <lvgl.h>

#include "lvgl/mb_menu_resources.h"
#include "lvgl/mb_osd.h"
#include "lvgl/mb_osd_audio_lr.h"
#include "lvgl/mb_osd_change_channel.h"
#include "lvgl/mb_osd_channel_detail.h"
#include "lvgl/mb_osd_channel_info.h"
#include "lvgl/mb_osd_channel_list.h"
#include "lvgl/mb_osd_fast_install.h"
#include "lvgl/mb_osd_lnbf_detection.h"
#include "lvgl/mb_osd_main_menu.h"
#include "lvgl/mb_osd_menu_plus.h"
#include "lvgl/mb_osd_menu_plus_record.h"
#include "lvgl/mb_osd_message_box.h"
#include "lvgl/mb_osd_popup_message.h"
#include "lvgl/mb_osd_production_info.h"
#include "lvgl/mb_osd_software_update_finish.h"
#include "lvgl/mb_osd_volume.h"
#include "lvgl/mb_osd_waiting_signal.h"
#include "lvgl/mb_osd_welcome_banner.h"
#include "lvgl/mb_osd_radio_icon.h"
#include "lvgl/mb_osd_parental_block_screen.h"
#include "lvgl/mb_osd_closed_caption.h"
#include "lvgl/mb_osd_draw_closed_caption.h"
#include "lvgl/mb_osd_draw_subtitle.h"
#include "lvgl/mb_osd_usb_disk.h"
#include "lvgl/mb_osd_program_reminder.h"
#include "lvgl/mb_osd_message_system_restart.h"
#include "lvgl/mb_osd_auto_search_channel_list.h"
#include "lvgl/mb_osd_software_update.h"
#include "mb_zone_id.h"
#include "fw_env.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdio>

#ifdef MBGUI_FORCED_UPDATE
#include "lvgl/mb_osd_software_update_forced.h"
#endif

namespace mb {

static std::chrono::steady_clock::time_point s_dbg_lvgl_next_run;

struct Task_OSD::Data
{
    lv_display_t *display { nullptr };

    Data()
    {
        /*LittlevGL init*/
        lv_init();
    }

    ~Data()
    {
#if LV_ENABLE_GC || !LV_MEM_CUSTOM
        lv_deinit();
#endif
    }
};

Task_OSD::Task_OSD():
    m_p(std::make_unique<Data>())
{
    s_dbg_lvgl_next_run = decltype(s_dbg_lvgl_next_run)::clock::now() + std::chrono::milliseconds(1000);
    mb_assert(s_task_osd == nullptr);
    s_task_osd = this;
    m_p->display = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(m_p->display, "/dev/fb0");
    lv_display_trigger_activity(NULL);
    lv_display_set_color_format(m_p->display, LV_COLOR_FORMAT_ARGB8888);
    // Força a atualização do display para a resolução atual
    set_force_refresh(true);
    lv_timer_handler();
    set_force_refresh(false);
    OSD::init_layers();
}

Task_OSD::~Task_OSD()
{
    mb_assert(s_task_osd == this);
    s_task_osd = nullptr;
}

void Task_OSD::process()
{
    auto now = decltype(s_dbg_lvgl_next_run)::clock::now();

    if(now > s_dbg_lvgl_next_run)
    {
#ifndef NDEBUG
        if(not g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed))
        {
            //DEBUG_MSG(OSD, DEBUG, TERM_RED_BOLD ">>> >>> >>> ALERT - SYSTEM BUSY <<< <<< <<< " TERM_YELLOW_BOLD "Pause low priority tasks." TERM_RESET "\n");
        }
#endif

        g_mbgui_pause_low_priority_tasks.store(true, std::memory_order_release);
        m_busy_until = now + 2s;
#ifdef MBGUI_USE_EXTRA_DEBUGGING
        auto late_by = std::chrono::duration_cast<std::chrono::microseconds>(now - s_dbg_lvgl_next_run).count();

        if(late_by > 500)
        {
            const char *colour = "";
            if(late_by > 100'000)
            {
                colour = TERM_RED_BOLD;
            }
            else if(late_by > 10'000)
            {
                colour = TERM_YELLOW_BOLD;
            }
            else if(late_by > 5'000)
            {
                colour = TERM_BLUE_BOLD;
            }

            DEBUG_MSG(OSD, DEBUG, "LVGL Late call by: " << colour << ((double)late_by)  / 1000 << "\n"TERM_RESET);
        }

#endif
    }
    else
    {
        if(g_mbgui_pause_low_priority_tasks.load(std::memory_order_acquire) and now > m_busy_until)
        {
            //DEBUG_MSG(OSD, DEBUG, ">>> >>> >>> ALERT - SYSTEM NOT BUSY <<< <<< <<< " TERM_YELLOW_BOLD "Resume low priority tasks." TERM_RESET "\n");
            g_mbgui_pause_low_priority_tasks.store(false, std::memory_order_relaxed);
        }
    }

    auto time = lv_timer_handler();
    s_dbg_lvgl_next_run = now + std::chrono::milliseconds(time);
}

void Task_OSD::set_force_refresh(bool _value)
{
    lv_linux_fbdev_set_force_refresh(m_p->display, _value);
}

void Task_OSD::show_menu()
{
    if(m_state == State::IDLE)
    {
        set_focus();

        if(not m_osd_main_menu)
        {
            m_osd_main_menu = std::make_unique<OSD_Main_Menu>(nullptr);
        }

        m_main_screen = OSD::create_rect(OSD::get_main_screen(OSD::OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(m_main_screen, LV_OPA_TRANSP, 0);
        m_osd_main_menu->show_main_menu(m_main_screen, ([this]
        {
            DELETE_OBJ(m_main_screen);
            m_osd_main_menu.reset();
        }));

        if(not Lineup_Mutex_Ref::is_empty())
        {
            Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Q2);
        }

        m_state = State::HOME;
    }
}

void Task_OSD::show_tv_channel_list()
{
    if(m_state == State::IDLE)
    {
        set_focus();

        if(not m_osd_ch_list)
        {
            m_osd_ch_list = std::make_unique<OSD_Channel_List>(nullptr);
        }

        m_osd_ch_list->show_menu(OSD::get_main_screen(OSD::OSD_Layer::MAIN_MENU), ([this]
        {
            m_state = State::IDLE;
            m_osd_ch_list.reset();
        }));
        m_state = State::INFO;
    }
}

void Task_OSD::show_waiting_signal()
{
#ifndef MBGUI_SAT_MONITOR

    if(m_state == State::IDLE or m_state == State::INFO or m_state == State::HOME)
    {
        if(not m_osd_waiting_signal)
        {
            m_osd_waiting_signal = std::make_unique<OSD_Waiting_Signal>(nullptr);
        }

        m_osd_waiting_signal->waiting_signal(OSD::get_main_screen(OSD::OSD_Layer::NO_SIGNAL));
    }

#endif
}

void Task_OSD::hide_waiting_signal()
{
#ifndef MBGUI_SAT_MONITOR

    if(m_osd_waiting_signal)
    {
        m_osd_waiting_signal->cleanup_screen_waiting_signal();
    }

#endif
}

void Task_OSD::show_menu_detail()
{
    if(not m_osd_ch_detail)
    {
        m_osd_ch_detail = std::make_unique<Channel_Detail>(nullptr);
    }
    m_osd_ch_detail->show_channel_detail([this]
    {
        DEBUG_MSG(OSD, DEBUG, "Apagando tela de informações detalhadas do canal\n");
        m_osd_ch_detail.reset();
    });
    Channel_Info::hide_channel_info();
    m_osd_ch_detail->start_refresh_timer();
}

void Task_OSD::close_menu_info()
{
    Channel_Info::hide_channel_info();
}

bool Task_OSD::has_menu_info()
{
    return Channel_Info::has_menuInfo();
}

void Task_OSD::show_menu_info(Service *_service)
{
    // Busca informações do canal atual
    if(not _service)
    {
        _service = s_task_player->current_srv();
    }
    auto osd_ch_info = Channel_Info::get_instance(nullptr);
    osd_ch_info->show_channel_info(_service);
}

void Task_OSD::message_box_callback(bool _ok)
{
    if(_ok)
    {
        DEBUG_MSG(OSD, DEBUG, "User accepted software update, showing software update screen\n");
        OSD::create_rect(OSD::get_main_screen(OSD::OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        if (!m_osd_software_update)
        {
            m_osd_software_update = std::make_unique<OSD_Software_Update>(nullptr);
        }
        m_osd_software_update->show_menu_software_update(std::bind(&Task_OSD::software_update_callback, this, std::placeholders::_1), true, true);
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "User declined software update, hiding message box\n");
        m_message_box.reset();
    }
}

void Task_OSD::software_update_callback(bool)
{
    if(m_message_box)
    {
        m_message_box.reset();
    }
    m_osd_software_update.reset();
}

void Task_OSD::fast_install_callback()
{
    DEBUG_MSG(OSD, DEBUG, "Apagando tela de instala fácil \n");
    m_osd_fast_install.reset();
}

void Task_OSD::handle_event_fast_install()
{
    if(m_state == State::IDLE)
    {
        if(not m_osd_fast_install)
        {
            m_osd_fast_install = std::make_unique<OSD_Fast_Install>(nullptr);
        }

        m_osd_fast_install->show_menu_fast_install(OSD::get_main_screen(OSD::OSD_Layer::MAIN_MENU), std::bind(&Task_OSD::fast_install_callback, this));
    }
}

bool Task_OSD::handle_event_remote_control(const Event_Remote_Control &)
{
    return false;
}

void Task_OSD::sound_changed_callback(bool _is_muted)
{
    DEBUG_MSG(OSD, DEBUG, "Apagando tela de volume \n");

    if(not _is_muted)
    {
        m_osd_volume.reset();
    }
}

void Task_OSD::handle_event_change_osd_media_player_state()
{
    m_state = State::MEDIA_PLAYER;
}

void Task_OSD::handle_event_change_osd_home_state()
{
    m_state = State::HOME;
}

void Task_OSD::handle_event_sound_changed(const Event_Sound &_event)
{
#if 0
    auto p_state = Task::s_task_player->get_player_state();
    auto mp_state = Task::s_task_player->get_media_player_state();
    auto pvr_state = Task::s_task_player->get_pvr_player_state();

    if((p_state == Player::State::Starting or p_state == Player::State::Started) or
            (mp_state == Media_Player::State::Starting or mp_state == Media_Player::State::Started) or
            (pvr_state == Task_Player::PVR_State::Starting or pvr_state == Task_Player::PVR_State::Started))
    {
        if(m_state == State::IDLE or
                m_state == State::MEDIA_PLAYER)
        {
            if(not m_osd_volume)
            {
                m_osd_volume = std::make_unique<OSD_Volume>();
            }

            m_osd_volume->set_volume(_event.vol, _event.muted, std::bind(&Task_OSD::sound_changed_callback, this, std::placeholders::_1));
        }
    }
#endif
    if(m_state == State::IDLE or m_state == State::MEDIA_PLAYER)
    {
        if(not m_osd_volume)
        {
            m_osd_volume = std::make_unique<OSD_Volume>();
        }

        m_osd_volume->set_volume(_event.vol, _event.muted, std::bind(&Task_OSD::sound_changed_callback, this, std::placeholders::_1));
    }
}

void Task_OSD::set_event_lineup_ready(Lineup_Ready_Event_CB _callback)
{
    m_lineup_ready_callback = _callback;
}

void Task_OSD::handle_event_lineup_ready(const Event_Lineup_Ready &_event)
{
    if(_event.origin == Lineup_Origin::LO_SATELLITE)
    {
        if(m_lineup_ready_callback)
        {
            m_lineup_ready_callback();
        }
    }
}

void Task_OSD::handle_event_osd_display_message(const Event_Display_Message &_message)
{
    auto ptr = m_osd_popup_message.lock();

    if(not ptr)
    {
        ptr = OSD_Popup_Message::get_instance();
        m_osd_popup_message = ptr;
    }
    ptr->show_popup_message(_message);
}

void Task_OSD::tvro_banner_hide()
{
    if(m_banner)
    {
        m_banner->hide_welcome_banner();
        m_banner.reset();
    }
}

void Task_OSD::tvro_banner_show()
{
    if(not m_banner)
    {
        m_banner = std::make_unique<OSD_Welcome_Banner>(nullptr);
    }

    m_banner->show_welcome_banner();
}

void Task_OSD::handle_event_osd_mainmenu_show()
{
    if(m_state == State::IDLE)
    {
        show_menu();
    }
}

void Task_OSD::handle_event_autodetect_progress(Event_Transponder_data _progress)
{
    auto ptr = m_osd_lnbf_detection.lock();

    if(ptr)
    {
        ptr->update_info(_progress);
    }
}

void Task_OSD::handle_event_osd_menu_plus(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    if(m_state == State::IDLE)
    {
        auto srv = Task::s_task_player->current_srv();
        m_plus = std::make_unique<OSD_Menu_Plus>(nullptr);
        m_plus->show_menu_plus(srv, ([this]
        {
            m_plus.reset();

        }), _call_pvr, _call_sleep_timer, _time_to_end);
    }
}

void Task_OSD::handle_event_osd_audio_lr()
{
    if(m_state == State::IDLE)
    {
        m_audio_lr = std::make_unique<OSD_Audio_LR>(nullptr);
        m_audio_lr->show_audio_lr(([this]
        {
            m_audio_lr.reset();
        }));
    }
}

void Task_OSD::handle_event_osd_production_info()
{
    if(not m_osd_prod_info)
    {
        m_osd_prod_info = std::make_unique<OSD_Production_Info>(nullptr);
    }

    m_osd_prod_info->show_production_info(([this]()
    {
        m_osd_prod_info.reset();
    }));
}

void Task_OSD::factory_reset_callback(bool _result)
{
    if(_result)
    {
        //Apaga a tela de informações
        if(m_osd_prod_info)
        {
            m_osd_prod_info.reset();
        }

        Task::post_event_system_factory_reset_done();
    }

    m_message_box.reset();
}

void Task_OSD::handle_event_osd_factory_reset()
{
    if(not m_message_box)
    {
        m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
    }
    m_message_box->show_message_box_yes_no(std::bind(&Task_OSD::factory_reset_callback, this, std::placeholders::_1), tr(__Padrao_de_fabrica_confirm), false);
}

void Task_OSD::handle_event_osd_channel_list_show()
{
    if(m_state == State::IDLE)
    {
        set_focus();

        if(m_osd_ch_list == nullptr)
        {
            //m_osdChList = std::make_unique<OSD_CH_List>();
        }

        show_tv_channel_list();
        m_state = State::INFO;
    }
}

static bool channel_exists(const Viewer_Channel_t _viewer_channel)
{
    auto srv = Task::s_task_player->current_srv();

    if((not srv) or (srv->viewer_channel() != _viewer_channel))
    {
        for(auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); const auto &srv : current_lineup->services)
        {
            if(srv.viewer_channel() == _viewer_channel)
            {
                return true;
            }
        }
    }
    else
    {
        return true;
    }

    return false;
}

void Task_OSD::show_menu_digit(char _digit, Viewer_Channel_CB _callback)
{
    if(not m_osd_change_channel)
    {
        m_osd_change_channel = std::make_unique<OSD_Change_Channel>(nullptr);
    }

    m_osd_change_channel->show_menu_digit(OSD::get_main_screen(OSD::OSD_Layer::MAIN_MENU), _digit, [this, _callback](Viewer_Channel_t _viewer_channel)
    {
        if (_viewer_channel == 0)
        {
            m_osd_change_channel.reset();
        }
        else
        {
            if(channel_exists(_viewer_channel))
            {
                _callback(_viewer_channel);
                m_osd_change_channel.reset();
            }
            else
            {
                m_osd_change_channel->show_message_box(([this]
                {
                    m_osd_change_channel.reset();
                }));
            }
        }
    });
}

void Task_OSD::show_instala_facil()
{
    if(not m_osd_instala_facil)
    {
        m_osd_instala_facil = std::make_unique<OSD_Instala_Facil>(nullptr);
    }

    m_osd_instala_facil->show_menu_easy_install(([this](bool _result)
    {
        m_osd_instala_facil.reset();

        if(_result == false)
        {
            show_menu();
        }
    }));
}

#ifdef MBGUI_FORCED_UPDATE
void Task_OSD::software_updated_forced()
{
    if(not m_sw_updt_forced)
    {
        m_sw_updt_forced = std::make_unique<OSD_Software_Update_Forced>(nullptr);
    }

    m_sw_updt_forced->show_menu_updt_forced(([this](bool /*_result*/)
    {
        m_sw_updt_forced.reset();
    }));
}
#endif

void Task_OSD::software_updated_finish_callback(bool )
{
    Osd_Breadcrumb::s_instance.remove_name();
    Osd_Breadcrumb::s_instance.clear();
    m_osd_sw_updt_finish.reset();
    post_event_starting_application();
}

void Task_OSD::show_menu_channel_list_update_callback()
{
    if(not mb_osd_activate)
    {
        mb_osd_activate = std::make_unique<OSD_Activate>(nullptr);
    }

    auto stb_activated = Zone_ID::get_zone_id() > 0;
    mb_osd_activate->show_menu_activate(std::bind(&Task_OSD::show_menu_activate_callback, this), stb_activated);
}

void Task_OSD::show_menu_activate_callback()
{
    mb_osd_activate.reset();
    m_osd_channel_list_update.reset();
    Osd_Breadcrumb::s_instance.remove_name(false);
    Osd_Breadcrumb::s_instance.remove_name(true);
    Osd_Breadcrumb::s_instance.clear();
    m_osd_sw_updt_finish.reset();
}

void Task_OSD::software_updated_finish()
{
    if(not m_osd_sw_updt_finish)
    {
        m_osd_sw_updt_finish = std::make_unique<OSD_Software_Update_finish>(nullptr);
    }

    m_osd_sw_updt_finish->show_menu_software_update_finish(std::bind(&Task_OSD::software_updated_finish_callback, this, std::placeholders::_1));
}

void Task_OSD::handle_event_player_started()
{
    if(m_state == State::IDLE)
    {
        Fade_Canvas::prepare_masks();
    }
}

void Task_OSD::handle_event_channel_change(Service *_service)
{
    m_draw_closed_caption.reset();
    m_draw_subtitle.reset();

    if(m_state == State::IDLE or m_state == State::INFO)
    {
        show_menu_info(_service);
    }

    auto ptr = m_osd_popup_message.lock();

    if(ptr)
    {
        ptr->hide_popup_message(Message_Categories::Program_Access);
    }
}

void Task_OSD::handle_event_channel_preview(Service *_srv)
{
    if (!_srv)
    {
        return;
    }

    show_menu_info(_srv);
}

void Task_OSD::handle_event_channel_changed(Service *_service)
{
    auto network_id = Config::get_config()->selected_satellite_config().network_id;
    Satellite_Operator _oper = network_id == Network_Id_Sky ? Satellite_Operator::Sky :
                               network_id == Network_Id_Claro ? Satellite_Operator::Claro : Satellite_Operator::Generic;

    auto zone_id = Zone_ID::get_zone_id(_oper);
    if (zone_id == 0 and network_id != Network_Id_Generic and _service->regionalizacao() != Regionalizacao::NaoRegionalizado)
    {
        tvro_banner_show();
    }
    else
    {
        tvro_banner_hide();
    }

    show_hide_radio_screen(_service);
    check_parental_control();
}

// Exibe tela de bloqueio no caso de programas inadequados
void Task_OSD::check_parental_control()
{

    DEBUG_MSG(OSD, DEBUG, "Checking parental control\n");
    if ( not m_osd_parental_block_screen )
    {
        DEBUG_MSG(OSD, DEBUG, "Creating OSD parental block screen\n");
        m_osd_parental_block_screen = std::make_unique<OSD_Parental_Block_Screen>(nullptr);
    }
    m_osd_parental_block_screen->init_parental_control();
}

void Task_OSD::update_parental_control()
{
    DEBUG_MSG(OSD, DEBUG, "Updating OSD parental control configuration\n");
    if (s_task_osd->m_osd_parental_block_screen)
    {
        DEBUG_MSG(OSD, DEBUG, "Updating OSD parental block screen\n");
        s_task_osd->m_osd_parental_block_screen->update_parental_control();
    }
}

// Verifica se o canal sintonizado é rádio ou tv e exibe tela quando for rádio
void Task_OSD::show_hide_radio_screen(const Service *_service)
{
    // Check if current service is tv
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

    if((_service == nullptr) or (current_lineup->is_tv_service(_service->service_type())))
    {
        hide_radio_screen();
    }
    else
    {
        if(not m_radio_icon)
        {
            m_radio_icon = std::make_unique<OSD_Radio_Icon>(nullptr);
        }

        // Get current service name
        auto current_service = current_lineup->get_current_service();
        std::stringstream current_service_name;
        current_service_name << current_service->viewer_channel() << " - " << current_service->name();
        // Get next service name
        auto next_service = current_lineup->get_next_service();
        std::stringstream next_service_name;
        if (next_service)
        {
            next_service_name << next_service->viewer_channel() << " - " << next_service->name();
        }
        // Get previous service name
        auto previous_service = current_lineup->get_previous_service();
        std::stringstream previous_service_name;
        if (previous_service)
        {
            previous_service_name << previous_service->viewer_channel() << " - " << previous_service->name();
        }
        // We no longer need to hold the reference
        current_lineup.reset();
        // Show the radio screen
        m_radio_icon->show_radio_icon(current_service_name.str(), next_service_name.str(), previous_service_name.str());
    }
}

void Task_OSD::hide_radio_screen()
{
    if(m_radio_icon)
    {
        m_radio_icon->hide_radio_icon();
        m_radio_icon.reset();
    }
}

void Task_OSD::handle_event_player_stop()
{
    tvro_banner_hide();
    hide_radio_screen();
}

void Task_OSD::handle_event_usb_plug_event(Event_USB_Plug _event)
{
    if(not m_usb_disk)
    {
        m_usb_disk = std::make_unique<OSD_USB_Disk>(nullptr);
    }
    m_usb_disk->show_usb_disk(([this]
    {
        m_usb_disk.reset();
    }),_event);
}

void Task_OSD::handle_event_zone_id_changed(Zone_ID_t, Zone_ID_t _to_zone_id)
{
    if(_to_zone_id > 0)
    {
        tvro_banner_hide();
    }

    // Não executa a atualização da lista de canais caso não seja no satélite da Sky
    auto config = Config::get_config();
    if(config->selected_satellite_config().network_id != Network_Id_Sky)
    {
        DEBUG_MSG(OSD, DEBUG, "Zone ID changed but not on Sky network, skipping channel list update\n");
        return;
    }

    // Não executa a atualização da lista de canais caso a lista de canais esteja vazia
    auto lineup = Lineup_Mutex_Ref::get_current_lineup();
    if(lineup->services.empty())
    {
        DEBUG_MSG(OSD, DEBUG, "Zone ID changed but channel list is empty, skipping channel list update\n");
        return;
    }

    // Atualização em andamento, ignorar nova solicitação
    if ( m_bkg_for_ch_update != nullptr )
    {
        DEBUG_MSG(OSD, DEBUG, "Channel list update already in progress, ignoring new request\n");
        return;
    }
    m_bkg_for_ch_update = OSD::create_rect(lv_screen_active(), 0, 0, m_ch_update_width, m_ch_update_height, OSD_COLOR_BLACK);
    lv_obj_move_background(m_bkg_for_ch_update);
    if (!m_osd_channel_list_update)
    {
        m_osd_channel_list_update = std::make_unique<OSD_Channel_List_Update>(nullptr);
    }
    m_osd_channel_list_update->show_menu_channel_list_update(std::bind(&Task_OSD::channel_list_update_callback, this, std::placeholders::_1));
} 

void Task_OSD::channel_list_update_callback(bool /*_result*/)
{
    DEBUG_MSG(OSD, WARN, "Channel list update finished callback\n");
    m_osd_channel_list_update.reset();
    auto ptr = m_osd_popup_message.lock();
    if(ptr)
    {
        ptr->hide_popup_message(Message_Categories::Event_Popup);
    }
    DELETE_OBJ(m_bkg_for_ch_update);
}

void Task_OSD::auto_search_channel_list_callback()
{
    m_osd_as_channel_list.reset();
}

void Task_OSD::handle_event_service_favorite_changed(Service *_service)
{
    if(m_state == State::IDLE)
    {
        show_menu_info(_service);
        Channel_Info::reset_hide_timer();
    }
}

void Task_OSD::got_focus()
{
    if(m_state == State::HOME)
    {
        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Fullscreen);
        remove_focus();
        m_state = State::IDLE;
    }
}

void Task_OSD::set_lnbf_detection(std::shared_ptr<OSD_Lnbf_Detection> _osd_lnbf_detection)
{
    Task::s_task_osd->m_osd_lnbf_detection = _osd_lnbf_detection;
}

void Task_OSD::handle_event_cc_enable(CC_Type _type)
{
    if (_type == CC_Type::Disabled)
    {
        m_draw_closed_caption.reset();
        m_draw_subtitle.reset();
    }
}

void Task_OSD::handle_event_osd_closed_caption()
{
    if(m_state == State::IDLE)
    {
        if(not m_plus_record)
        {
            m_draw_closed_caption = std::make_unique<OSD_Draw_Closed_Caption>(nullptr);
            m_closed_caption = std::make_unique<OSD_Closed_Caption>(nullptr);
            m_draw_subtitle = std::make_unique<OSD_Draw_Subtitle>(nullptr);
        }
        m_draw_subtitle->create_subtitle();
        m_draw_closed_caption->create_closed_caption();
        m_closed_caption->show_closed_caption(([this]
        {
            m_closed_caption.reset();
        }));
    }
}

void Task_OSD::handle_event_cc(const Event_CC &_event)
{
    if (m_draw_closed_caption)
    {
        m_draw_closed_caption->print_closed_caption(_event);
    }
}

void Task_OSD::handle_event_subtitle(const Event_Subtitle_Image &_event)
{
    if (m_draw_subtitle)
    {
        m_draw_subtitle->show_subtitle(_event);
    }
}

void Task_OSD::handle_event_send_message_to_start_record(ScheduleEntry _sc_entry)
{

    if(m_state == State::HOME)
    {
        DELETE_OBJ(m_main_screen);
        Display::get_instance()->resize_video_to_quadrant(Display::Quadrant::Fullscreen);
    }
    m_plus.reset();
    m_osd_main_menu.reset();
    m_osd_ch_list.reset();
    m_audio_lr.reset();
    m_osd_ch_detail.reset();
    m_state = State::IDLE;


    if (not m_program_reminder)
    {
        m_program_reminder = std::make_unique<OSD_Program_Reminder>(nullptr);
    }
    m_program_reminder->show_program_reminder(([this]
    {
        m_program_reminder.reset();
    }), _sc_entry);
}

void Task_OSD::handle_event_system_need_reset()
{
    if (not m_message_system_restart)
    {
        m_message_system_restart = std::make_unique<OSD_Message_System_Restart>(nullptr);
        m_message_system_restart->show_message_system_restart(([this]
        {
            m_message_system_restart.reset();
        }));
    }
}

void Task_OSD::handle_event_ota_found()
{
    DEBUG_MSG(OSD, DEBUG, "OTA found, showing software update screen *********************\n");
    if(not m_message_box)
    {
        m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
    }
    std::string update_msg = std::string(tr(__Encontrada_atualizacao_de_programa)) + "\n" + std::string(tr(__Deseja_atualizar_o_receptor));
    m_message_box->show_message_box_yes_no(std::bind(&Task_OSD::message_box_callback, this, std::placeholders::_1), update_msg, false);
}

} // namespace mb
