#include "mb_osd_menu_plus_timeshift.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_events.h"
#include "mb_menu_resources.h"
#include "mb_osd.h"
#include "mb_osd_clock.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_application.h"

#include <filesystem>
#include <fstream>
#include <lvgl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

namespace mb
{

static constexpr std::array<std::string_view, 6> s_logo_mp_functions =
{
    LOGO_MEDIA_PLAYER_PLAY_34X34,
    LOGO_MEDIA_PLAYER_PAUSE_34X34,
    LOGO_MEDIA_PLAYER_REWIND_34X34,
    LOGO_MEDIA_PLAYER_FORWARD_34X34,
};

static constexpr std::array<std::string_view, 6> s_logo_mp_functions_sel =
{
    LOGO_MEDIA_PLAYER_PLAY_SEL_34X34,
    LOGO_MEDIA_PLAYER_PAUSE_SEL_34X34,
    LOGO_MEDIA_PLAYER_REWIND_SEL_34X34,
    LOGO_MEDIA_PLAYER_FORWARD_SEL_34X34,
};

OSD_Menu_Plus_Timeshift::OSD_Menu_Plus_Timeshift(OSD *_parent)
    : OSD(_parent), m_confirm_keys(offset_x, offset_y, button_w, button_h,
                                   spacing, button_x, button_y)
{
}

OSD_Menu_Plus_Timeshift::~OSD_Menu_Plus_Timeshift()
{
    remove_focus();
    DELETE_TIMER(m_hide_timer);
    DELETE_TIMER(m_record_time_timer);
    DELETE_OBJ(m_box_timeshift);
    DELETE_OBJ(m_box_confirm_stop);
}

bool OSD_Menu_Plus_Timeshift::handle_event_remote_control(const Event_Remote_Control &_event)
{

    if(_event.key ==  Remote_Control_Key::KEY_POWER)
    {
        if (not m_rec_stop_recording)
        {
            lv_obj_add_flag(m_box_timeshift, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
            m_rec_stop_recording = true;
            DELETE_TIMER(m_hide_timer);
        }
    }

    if (m_timeshift_initializing)
    {
        return true; // ignora qualquer tecla
    }

    if (m_hide_screen_enabled)
    {
        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLUP:
            case Remote_Control_Key::KEY_VOLDOWN:
            case Remote_Control_Key::KEY_MUTE:
                Task::s_task_application->handle_event_remote_control(_event);
                return true;

            default:
                show_video_screen();
                return true;
        }
    }
    else
    {
        if (not m_rec_stop_recording)
        {
            lv_timer_reset(m_hide_timer);
        }

        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLTAR:
            case Remote_Control_Key::KEY_MENU:
                if (not m_rec_stop_recording)
                {
                    lv_obj_add_flag(m_box_timeshift, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_remove_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
                    m_rec_stop_recording = true;
                    DELETE_TIMER(m_hide_timer);
                }
                //else
                //{
                //    Task::post_event(std::bind(m_callback));
                //}
            return true;

            case Remote_Control_Key::KEY_OK:
                if (m_rec_stop_recording)
                {
                        if (static_cast<FunctionYesNoOption>(m_confirm_keys.get_selected()) == FunctionYesNoOption::YES)
                        {
                            Task::s_task_player->pvr_timeshift_stop();
                            Task::post_event(std::bind(m_callback));
                        }
                        else
                        {
                            m_rec_stop_recording = false;
                            lv_obj_remove_flag(m_box_timeshift, LV_OBJ_FLAG_HIDDEN);
                            lv_obj_add_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
                            start_hide_timer();
                        }
                }
                else
                {
                    switch (static_cast<Tms_Functions_t>(m_selected_item))
                    {
                        case Tms_Functions_t::TMS_PLAY:
                            lv_label_set_text(m_label_speed, "");
                            m_current_forward_speed = Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL;
                            m_current_rewind_speed = Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL;
                            Task::s_task_player->pvr_timeshift_resume();
                            break;

                        case Tms_Functions_t::TMS_PAUSED:
                            lv_label_set_text(m_label_speed, "");
                            m_current_forward_speed = Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL;
                            m_current_rewind_speed = Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL;
                            Task::s_task_player->pvr_timeshift_pause();
                            break;

                        case Tms_Functions_t::TMS_REWIND:
                            next_rewind_speed();
                            update_rewind_label_speed();
                            if (m_current_rewind_speed == Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL)
                            {
                                Task::s_task_player->pvr_timeshift_resume();
                            }
                            else
                            {
                                Task::post_event_cas_pvr_play_rewind(                                   static_cast<uint16_t>(m_current_rewind_speed));
                            }
                            break;

                        case Tms_Functions_t::TMS_FORWARD:
                            next_forward_speed();
                            update_forward_label_speed();
                            if (m_current_forward_speed == Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL)
                            {
                                Task::s_task_player->pvr_timeshift_resume();
                            }
                            else
                            {
                                Task::post_event_cas_pvr_play_forward(                                   static_cast<uint16_t>(m_current_forward_speed));
                            }
                            break;

                        case Tms_Functions_t::TMS_CANCEL:
                        default:
                            lv_obj_add_flag(m_box_timeshift, LV_OBJ_FLAG_HIDDEN);
                            lv_obj_remove_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
                            m_rec_stop_recording = true;
                            DELETE_TIMER(m_hide_timer);
                            break;
                    }
                }
                return true;

            case Remote_Control_Key::KEY_VOLUP:
                if (m_rec_stop_recording)
                {
                    m_confirm_keys.next();
                }
                else
                {
                    move_right();
                }
                return true;

            case Remote_Control_Key::KEY_VOLDOWN:
                if (m_rec_stop_recording)
                {
                    m_confirm_keys.previous();
                }
                else {
                    move_left();
                }
                return true;

            default:
                // show_video_screen();
                return true;
        }
    }
    return false;
}

void OSD_Menu_Plus_Timeshift::update_forward_label_speed()
{
    switch (m_current_forward_speed)
    {
        case Media_Player::MP_Speed_Forward::MP_SPEED_FASTFORWARD_2:
            lv_label_set_text(m_label_speed, "2X");
            break;

        case Media_Player::MP_Speed_Forward::MP_SPEED_FASTFORWARD_4:
            lv_label_set_text(m_label_speed, "4X");
            break;

        case Media_Player::MP_Speed_Forward::MP_SPEED_FASTFORWARD_8:
            lv_label_set_text(m_label_speed, "8X");
            break;
#if 0
        case Media_Player::MP_Speed_Forward::MP_SPEED_FASTFORWARD_16:
            lv_label_set_text(m_label_speed, "16X");
            break;

        case Media_Player::MP_Speed_Forward::MP_SPEED_FASTFORWARD_24:
            lv_label_set_text(m_label_speed, "24X");
            break;
#endif
        case Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL:
        default:
            lv_label_set_text(m_label_speed, "");
            break;
    }
}

void OSD_Menu_Plus_Timeshift::update_rewind_label_speed()
{
    switch (m_current_rewind_speed)
    {
        case Media_Player::MP_Speed_Rewind::MP_SPEED_FASTREWIND_2:
            lv_label_set_text(m_label_speed, "2X");
            break;

        case Media_Player::MP_Speed_Rewind::MP_SPEED_FASTREWIND_4:
            lv_label_set_text(m_label_speed, "4X");
            break;

        case Media_Player::MP_Speed_Rewind::MP_SPEED_FASTREWIND_8:
            lv_label_set_text(m_label_speed, "8X");
            break;
#if 0
        case Media_Player::MP_Speed_Rewind::MP_SPEED_FASTREWIND_16:
            lv_label_set_text(m_label_speed, "16X");
            break;

        case Media_Player::MP_Speed_Rewind::MP_SPEED_FASTREWIND_24:
            lv_label_set_text(m_label_speed, "24X");
            break;
#endif
        case Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL:
        default:
            lv_label_set_text(m_label_speed, "");
            break;
    }
}


void OSD_Menu_Plus_Timeshift::next_forward_speed()
{
    int speed = static_cast<int>(m_current_forward_speed);

    speed++;
    if (speed > static_cast<int>(Media_Player::MP_Speed_Forward::MP_SPEED_FASTFORWARD_8))
        speed = static_cast<int>(Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL);

    m_current_forward_speed =
        static_cast<Media_Player::MP_Speed_Forward>(speed);
}

void OSD_Menu_Plus_Timeshift::next_rewind_speed()
{
    int speed = static_cast<int>(m_current_rewind_speed);

    speed++;
    if (speed > static_cast<int>(Media_Player::MP_Speed_Rewind::MP_SPEED_FASTREWIND_8))
        speed = static_cast<int>(Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL);

    m_current_rewind_speed =
        static_cast<Media_Player::MP_Speed_Rewind>(speed);
}

void OSD_Menu_Plus_Timeshift::create_tms_frame()
{

    m_box_timeshift = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20,
                                    AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_timeshift);
    lv_obj_set_style_bg_opa(m_box_timeshift, 0, LV_PART_MAIN);
    m_play_time = set_label_text_static(m_box_timeshift, "00:00:00", 400, 90,
                                        font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_play_time);
    m_rec_time = set_label_text_static(m_box_timeshift, "00:00:00", 400, 130,
                                        font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_rec_time);
    static constexpr auto base_x_line = 600;
    static constexpr auto base_y_line = 110;
    static constexpr auto x_start = 44;

    for (uint8_t idx = 0; idx < m_mp_functions.size(); idx++)
    {
        switch (static_cast<Tms_Functions_t>(idx))
        {
            case Tms_Functions_t::TMS_CANCEL:
            {
                m_button = create_rect(m_box_timeshift, 850, 100, 220, 50,
                                        OSD_COLOR_GREY_MEDIUM);
                lv_obj_null_on_delete(&m_button);
                lv_obj_set_style_radius(m_button, 25, DEFAULT_SELECTOR);
                auto label = set_label_text(m_button, tr(__Cancelar), 0, 0, font_semi_25,
                                            OSD_COLOR_WHITE);
                lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
            }
            break;

            case Tms_Functions_t::TMS_PLAY:
            case Tms_Functions_t::TMS_PAUSED:
            case Tms_Functions_t::TMS_REWIND:
            case Tms_Functions_t::TMS_FORWARD:
            default:
                m_mp_functions[idx] = load_image(m_box_timeshift, s_logo_mp_functions[idx].data(),
                                base_x_line + (x_start * idx), base_y_line, 34, 34);
                lv_obj_null_on_delete(&m_mp_functions[idx]);
                m_mp_functions_sel[idx] = load_image(m_box_timeshift, s_logo_mp_functions_sel[idx].data(),
                                base_x_line + (x_start * idx), base_y_line, 34, 34);
                lv_obj_add_flag(m_mp_functions_sel[idx], LV_OBJ_FLAG_HIDDEN);
                break;
        }
    }

    m_label_speed = set_label_text_static(m_box_timeshift, "", base_x_line + (x_start * 4), base_y_line + 5, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_label_speed);

    m_box_confirm_stop = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20,
                                    AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_confirm_stop);
    lv_obj_set_style_bg_opa(m_box_confirm_stop, 0, LV_PART_MAIN);
    auto box_message =
        create_rect(m_box_confirm_stop, 300, 110, 400, 50, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(box_message, 0, LV_PART_MAIN);
    auto message =
        set_label_text_static(box_message, tr(__Interromper_o_modo_timeshift),
                                300, 110, font_25, OSD_COLOR_WHITE);
    lv_obj_align(message, LV_ALIGN_TOP_RIGHT, 0, 0);
    // Desenha botões
    m_confirm_keys.clear();
    m_confirm_keys.set_background(m_box_confirm_stop);

    for (const auto &option : m_yesnooptions)
    {
        m_confirm_keys.add_label(option.text);
    }

    m_confirm_keys.set_horizontal();
    m_confirm_keys.draw();
    m_confirm_keys.next();
    m_confirm_keys.select();
    lv_obj_add_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Menu_Plus_Timeshift::show_menu_plus_timeshift(lv_obj_t *_bgd, const Service *, Plus_Timeshift_CB_t _callback)
{
    // Direciona recepção de tecla e função de retorno
    set_focus();
    m_callback = _callback;
    m_main_screen = _bgd;
    create_tms_frame();
    move_selection(0, m_selected_item);
    start_hide_timer();
    get_usb_mount_point();
    show_message_timeshift_initializing();
    Task::post_event_pvr_timeshift_start(m_mount_point);
    start_record_time_timer();
}

void OSD_Menu_Plus_Timeshift::get_usb_mount_point()
{
    std::ifstream mounts("/proc/mounts");
    std::string line;

    while (std::getline(mounts, line))
    {
        std::istringstream iss(line);
        std::string device, mountpoint, fstype;

        if (iss >> device >> mountpoint >> fstype)
        {
            if (mountpoint.find(USB_PATH) == 0)
            {
                m_mount_point = mountpoint;
            }
        }
    }
}

void OSD_Menu_Plus_Timeshift::hide_video_screen_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Plus_Timeshift *thiz = static_cast<OSD_Menu_Plus_Timeshift *>(lv_timer_get_user_data(_tm));
    lv_obj_add_flag(thiz->m_main_screen, LV_OBJ_FLAG_HIDDEN);
    thiz->m_hide_screen_enabled = true;
    thiz->m_hide_timer = nullptr;
}

void OSD_Menu_Plus_Timeshift::show_video_screen()
{
  lv_obj_remove_flag(m_main_screen, LV_OBJ_FLAG_HIDDEN);
  m_hide_screen_enabled = false;
  start_hide_timer();
}

void OSD_Menu_Plus_Timeshift::start_hide_timer()
{
    // Inicia timer para apagar informações do canal
    if (m_hide_timer == nullptr)
    {
        m_hide_timer = lv_timer_create(hide_video_screen_cb, 5000, this);
        lv_timer_set_repeat_count(m_hide_timer, 1);
    }
    else
    {
        lv_timer_reset(m_hide_timer);
    }
}

void OSD_Menu_Plus_Timeshift::update_record_time()
{

    std::time_t current_rec_time = Task::s_task_player->get_pvr_timeshift_rec_curr_time();
    std::time_t current_play_time = Task::s_task_player->get_pvr_timeshift_play_curr_time();

    if (m_timeshift_initializing && current_rec_time > 3)
    {
        hide_message_timeshift_initializing();
        Task::s_task_player->pvr_timeshift_pause();
    }

    if (m_current_forward_speed != Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL ||
        m_current_rewind_speed  != Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL)
    {
        // Detecta se o player está pausado
        if (Task::s_task_player->get_pvr_player_state() == Task_Player::PVR_State::Paused)
        {
            lv_label_set_text(m_label_speed, "");
            current_rec_time = 0;

            m_current_forward_speed = Media_Player::MP_Speed_Forward::MP_SPEED_NORMAL;
            m_current_rewind_speed  = Media_Player::MP_Speed_Rewind::MP_SPEED_NORMAL;

            // Atualiza seleção visual para PLAY/PAUSE
            auto previous = m_selected_item;
            m_selected_item = static_cast<uint8_t>(Tms_Functions_t::TMS_PAUSED);
            move_selection(previous, m_selected_item);
        }
    }

    char buf1[80];
    std::strftime(buf1, 80, "%H:%M:%S", localtime(&current_play_time));
    lv_label_set_text(m_play_time, buf1);
    std::strftime(buf1, 80, "%H:%M:%S", localtime(&current_rec_time));
    lv_label_set_text(m_rec_time, buf1);
}

void OSD_Menu_Plus_Timeshift::record_time_cb(lv_timer_t *_tm)
{
  if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
  {
    return;
  }

  OSD_Menu_Plus_Timeshift *thiz = static_cast<OSD_Menu_Plus_Timeshift *>(lv_timer_get_user_data(_tm));
  thiz->update_record_time();
}

void OSD_Menu_Plus_Timeshift::start_record_time_timer()
{
    // Inicia timer para ler informaçõesdo do video
    if (m_record_time_timer == nullptr)
    {
        m_record_time_timer = lv_timer_create(record_time_cb, 1000, this);
        lv_timer_set_repeat_count(m_record_time_timer, -1);
    }
    else
    {
        lv_timer_reset(m_record_time_timer);
    }
}

void OSD_Menu_Plus_Timeshift::del_record_time_timer()
{
    DELETE_TIMER(m_record_time_timer);
}

void OSD_Menu_Plus_Timeshift::set_menu_selection(uint8_t _menu, bool _selected)
{
  if (_menu < static_cast<uint8_t>(Tms_Functions_t::MAX_TMS_FUNCTIONS))
  {
    if (_menu == static_cast<uint8_t>(Tms_Functions_t::TMS_CANCEL))
    {
      lv_obj_set_style_bg_color(m_button, _selected ? OSD_COLOR_ORANGE : OSD_COLOR_GREY_MEDIUM, mb::DEFAULT_SELECTOR);
    }
    else
    {
      if (_selected)
      {
        lv_obj_add_flag(m_mp_functions[_menu], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(m_mp_functions_sel[_menu], LV_OBJ_FLAG_HIDDEN);
      }
      else
      {
        lv_obj_remove_flag(m_mp_functions[_menu], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(m_mp_functions_sel[_menu], LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
}

void OSD_Menu_Plus_Timeshift::move_selection(uint8_t _from, uint8_t _to)
{
  set_menu_selection(_from, false);
  set_menu_selection(_to, true);
}

void OSD_Menu_Plus_Timeshift::move_right()
{
  auto currentPos = m_selected_item;
  auto count = static_cast<uint16_t>(Tms_Functions_t::MAX_TMS_FUNCTIONS);
  m_selected_item = (m_selected_item + count + 1) % count;
  move_selection(currentPos, m_selected_item);
}

void OSD_Menu_Plus_Timeshift::move_left()
{
  auto currentPos = m_selected_item;
  auto count = static_cast<uint16_t>(Tms_Functions_t::MAX_TMS_FUNCTIONS);
  m_selected_item = (m_selected_item + count - 1) % count;
  move_selection(currentPos, m_selected_item);
}

void OSD_Menu_Plus_Timeshift::show_message_timeshift_initializing()
{
    DEBUG_MSG(OSD, DEBUG, "show_message_timeshift_initializing\n");

    m_timeshift_initializing = true;

    if (not m_message_box)
    {
        m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
    }

    m_message_box->show_message_box(tr(__Iniciando_o_timeshift));
}

void OSD_Menu_Plus_Timeshift::hide_message_timeshift_initializing()
{
    if (m_message_box)
    {
        m_message_box.reset();
    }

    m_timeshift_initializing = false;
}

} // namespace mb
