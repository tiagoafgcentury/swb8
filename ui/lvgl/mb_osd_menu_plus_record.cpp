#include "common/mb_globals.h"
#include "mb_osd_menu_plus_record.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_clock.h"
#include "mb_osd_message_box.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_application.h"
#include "common/mb_lineup.h"

#include <lvgl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace mb {

OSD_Menu_Plus_Record::OSD_Menu_Plus_Record(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y),
    m_confirm_keys(offset_x, offset_y, button_w / 2, button_h, 133, button_x + 160, button_y)
{
}

OSD_Menu_Plus_Record::~OSD_Menu_Plus_Record()
{
    remove_focus();
    DELETE_TIMER(m_hide_timer);
    DELETE_TIMER(m_record_info_timer);
    DELETE_TIMER(m_record_time_timer);
    DELETE_OBJ(m_box_record);
    DELETE_OBJ(m_box_confirm_stop);
}

bool OSD_Menu_Plus_Record::handle_event_remote_control(const Event_Remote_Control &_event)
{

    if(_event.key ==  Remote_Control_Key::KEY_POWER)
    {
        if (not m_rec_stop_recording)
        {
            m_rec_stop_recording = true;
            lv_obj_add_flag(m_box_record, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
            DELETE_TIMER(m_hide_timer);
        }
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
        if (m_rec_info_enable)
        {
            if (_event.key == Remote_Control_Key::KEY_OK || _event.key == Remote_Control_Key::KEY_VOLTAR)
            {
                m_rec_info_enable = false;
                del_record_info_timer();
                lv_obj_add_flag(m_info_box, LV_OBJ_FLAG_HIDDEN);
                start_hide_timer();
            }

            return true;
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
                    if (m_rec_stop_recording)
                    {
                        Task::post_event(std::bind(m_callback));
                    }

                    return true;

                case Remote_Control_Key::KEY_OK:
                    if (static_cast<FunctionOption>(m_keys.get_selected()) == FunctionOption::STOP)
                    {
                        if (m_rec_stop_recording)
                        {
                            if (static_cast<FunctionYesNoOption>(m_confirm_keys.get_selected()) == FunctionYesNoOption::YES)
                            {
                                Task::s_task_player->pvr_record_stop();
                                Task::post_event(std::bind(m_callback));
                            }
                            else
                            {
                                m_rec_stop_recording = false;
                                lv_obj_remove_flag(m_box_record, LV_OBJ_FLAG_HIDDEN);
                                lv_obj_add_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
                                start_hide_timer();
                            }
                        }
                        else
                        {
                            m_rec_stop_recording = true;
                            lv_obj_add_flag(m_box_record, LV_OBJ_FLAG_HIDDEN);
                            lv_obj_remove_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
                            DELETE_TIMER(m_hide_timer);
                        }
                    }
                    else
                    {
                        m_rec_info_enable = true;
                        start_record_info_timer();
                        lv_obj_remove_flag(m_info_box, LV_OBJ_FLAG_HIDDEN);
                        DELETE_TIMER(m_hide_timer);
                    }

                    return true;

                case Remote_Control_Key::KEY_VOLUP:
                    if (m_rec_stop_recording)
                    {
                        m_confirm_keys.next();
                    }
                    else
                    {
                        m_keys.next();
                    }

                    return true;

                case Remote_Control_Key::KEY_VOLDOWN:
                    if (m_rec_stop_recording)
                    {
                        m_confirm_keys.previous();
                    }
                    else
                    {
                        m_keys.previous();
                    }

                    return true;

                default:
                    //show_video_screen();
                    return true;
            }
        }
    }

    return false;
}

void OSD_Menu_Plus_Record::create_pvr_midia_informations()
{

    if (m_info_box == nullptr)
    {
        m_info_box = create_rect(m_main_screen, 0, 0, info_w, info_h, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_info_box);
        lv_obj_set_style_radius(m_info_box, 25, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_opa(m_info_box, LV_OPA_80, LV_PART_MAIN);
        lv_obj_align(m_info_box, LV_ALIGN_CENTER, 0, 0);
        auto usb_logo = load_image(m_info_box, LOGO_FILE_USB_68X80, 0, 0, 68, 80);
        lv_obj_align(usb_logo, LV_ALIGN_TOP_LEFT, 280, 40);
        auto file_box = create_rect(m_info_box, 0, 0, file_w, file_h, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(file_box, 25, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_opa(file_box, LV_OPA_100, LV_PART_MAIN);
        lv_obj_align(file_box, LV_ALIGN_RIGHT_MID, -20, 0);
        auto file_ts_logo = load_image(file_box, LOGO_FILE_VIDEO_SELECTED_68X80, 0, 0, 68, 80);
        lv_obj_align(file_ts_logo, LV_ALIGN_TOP_MID, 0, 20);
        auto pvr_title_box = create_rect(m_info_box, 0, 0, 280, 240, OSD_COLOR_BLACK);
        lv_obj_align(pvr_title_box, LV_ALIGN_LEFT_MID, 20, 50);
        lv_obj_set_style_bg_opa(pvr_title_box, LV_OPA_0, LV_PART_MAIN);
        auto pvr_info_box = create_rect(m_info_box, 0, 0, 130, 240, OSD_COLOR_BLACK);
        lv_obj_align(pvr_info_box, LV_ALIGN_LEFT_MID, 320, 50);
        lv_obj_set_style_bg_opa(pvr_info_box, LV_OPA_0, LV_PART_MAIN);
        auto lbl_mode = set_label_text(pvr_title_box, tr(__Modo_PVR), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_mode, LV_ALIGN_TOP_RIGHT, 0, 0);
        auto lbl_fs = set_label_text(pvr_title_box, tr(__Sistema_de_Arquivos), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_fs, LV_ALIGN_TOP_RIGHT, 0, 50);
        auto lbl_total = set_label_text(pvr_title_box, tr(__Espaco_Total), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_total, LV_ALIGN_TOP_RIGHT, 0, 100);
        auto lbl_available = set_label_text(pvr_title_box, tr(__Espaco_Disponivel), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_available, LV_ALIGN_TOP_RIGHT, 0, 150);
        auto lbl_max_rec = set_label_text(pvr_title_box, tr(__Tempo_max_de_gravacao), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_max_rec, LV_ALIGN_TOP_RIGHT, 0, 200);
        m_lbl_mode = set_label_text(pvr_info_box, tr(__Gravacao), 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_mode);
        lv_obj_align(m_lbl_mode, LV_ALIGN_TOP_LEFT, 0, 0);
        m_lbl_fs = set_label_text(pvr_info_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_fs);
        lv_obj_align(m_lbl_fs, LV_ALIGN_TOP_LEFT, 0, 50);
        m_lbl_total = set_label_text(pvr_info_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_align(m_lbl_total, LV_ALIGN_TOP_LEFT, 0, 100);
        lv_obj_null_on_delete(&m_lbl_total);
        m_lbl_available = set_label_text(pvr_info_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_available);
        lv_obj_align(m_lbl_available, LV_ALIGN_TOP_LEFT, 0, 150);
        m_lbl_max_rec = set_label_text(pvr_info_box, "00:00:00", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_max_rec);
        lv_obj_align(m_lbl_max_rec, LV_ALIGN_TOP_LEFT, 0, 200);
        auto lbl_filename = set_label_text(file_box, tr(__Nome_do_arquivo), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_filename, LV_ALIGN_TOP_MID, 0, 130);
        m_lbl_prog_name = set_label_text(file_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_prog_name);
        lv_label_set_long_mode(m_lbl_prog_name, LV_LABEL_LONG_DOT);
        lv_obj_set_width(m_lbl_prog_name, 450);
        lv_obj_set_height(m_lbl_prog_name, 40);
        lv_obj_set_style_text_align(m_lbl_prog_name, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_y(m_lbl_prog_name, 170);
        auto lbl_atual_file_size = set_label_text(file_box, tr(__Tamanho_atual_do_arquivo), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_atual_file_size, LV_ALIGN_TOP_MID, 0, 220);
        m_lbl_file_size = set_label_text(file_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_file_size);
        lv_obj_align(m_lbl_file_size, LV_ALIGN_TOP_MID, 0, 260);
        auto lbl_record_rate = set_label_text(file_box, tr(__Taxa_de_gravacao), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl_record_rate, LV_ALIGN_TOP_MID, 0, 310);
        m_lbl_record_rate = set_label_text(file_box, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_record_rate);
        lv_obj_align(m_lbl_record_rate, LV_ALIGN_TOP_MID, 0, 350);
        lv_obj_add_flag(m_info_box, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Menu_Plus_Record::show_menu_plus_record(lv_obj_t *_bgd, const Service *, std::chrono::time_point<std::chrono::system_clock> _time_to_end, Plus_Record_CB_t _callback)
{
    // Direciona recepção de tecla e função de retorno
    set_focus();
    m_time_to_end = _time_to_end;
    m_callback = _callback;
    m_main_screen = _bgd;
    m_box_record = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_record);
    lv_obj_set_style_bg_opa(m_box_record, 0, LV_PART_MAIN);
    m_rec_time = set_label_text_static(m_box_record, "00:00:00", 320, 110, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_rec_time);
    // Desenha botões
    m_keys.clear();
    m_keys.set_background(m_box_record);

    for (const auto &option : m_options)
    {
        m_keys.add_label(option.text);
    }

    m_keys.set_horizontal();
    m_keys.draw();
    m_keys.select();
    m_box_confirm_stop = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_confirm_stop);
    lv_obj_set_style_bg_opa(m_box_confirm_stop, 0, LV_PART_MAIN);
    auto box_message = create_rect(m_box_confirm_stop, 300, 110, 400, 50, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(box_message, 0, LV_PART_MAIN);
    auto message = set_label_text_static(box_message, tr(__Interromper_o_modo_de_gravacao), 300, 110, font_25, OSD_COLOR_WHITE);
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
    create_pvr_midia_informations();
    auto _pvr_state = Task::s_task_player->get_pvr_player_state();

    if (_pvr_state == Task_Player::PVR_State::Idle or _pvr_state == Task_Player::PVR_State::Stopped)
    {
        Task::post_event_pvr_start();
    }
    start_record_time_timer();
    start_hide_timer();
}

void OSD_Menu_Plus_Record::hide_video_screen_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Plus_Record *thiz = static_cast<OSD_Menu_Plus_Record *>(lv_timer_get_user_data(_tm));
    lv_obj_add_flag(thiz->m_main_screen, LV_OBJ_FLAG_HIDDEN);
    thiz->m_hide_screen_enabled = true;
    thiz->m_hide_timer = nullptr;
}

void OSD_Menu_Plus_Record::show_video_screen()
{
    lv_obj_remove_flag(m_main_screen, LV_OBJ_FLAG_HIDDEN);
    m_hide_screen_enabled = false;
    start_hide_timer();
}

void OSD_Menu_Plus_Record::start_hide_timer()
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

void OSD_Menu_Plus_Record::pvr_stop_record_callback()
{
    m_rec_stop_recording = true;

    if (m_box_record)
    {
        lv_obj_add_flag(m_box_record, LV_OBJ_FLAG_HIDDEN);
    }

    if (m_box_confirm_stop)
    {
        lv_obj_remove_flag(m_box_confirm_stop, LV_OBJ_FLAG_HIDDEN);
    }

    DELETE_TIMER(m_hide_timer);

    if (m_callback)
    {
        Task::post_event(std::bind(m_callback));
    }
}

void OSD_Menu_Plus_Record::show_usb_plug_event(Event_USB_Plug _event)
{
    auto pvr_state = Task::s_task_player->get_pvr_player_state();

    if ((pvr_state == Task_Player::PVR_State::Started or pvr_state == Task_Player::PVR_State::Starting) and
            _event.type == Event_USB_Plug::Removed)
    {
        Task::s_task_player->pvr_record_stop();

        if (not m_message_box)
        {
            m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
        }

        m_message_box->show_message_box_ok(std::bind(&OSD_Menu_Plus_Record::pvr_stop_record_callback, this), tr(__Pendrive_removido_Gravacao_interrompida));
    }
}

void OSD_Menu_Plus_Record::update_record_time()
{
    m_pvr_current_time = Task::s_task_player->get_pvr_record_curr_time();
    m_mount_point      = Task::s_task_player->get_pvr_mount_point();
    m_filesystem_type  = Task::s_task_player->get_pvr_filesystem_type();
    m_pvr_filename     = Task::s_task_player->get_pvr_record_filename();

    m_pvr_file_path = m_mount_point + "/PVR/" + m_pvr_filename + "/000.ts";

    /* Atualiza tempo na tela */
    {
        std::time_t current_time = m_pvr_current_time;
        char buf[80] {};
        std::strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&current_time));
        lv_label_set_text(m_rec_time, buf);
    }

    /* Tempo inválido ou muito curto → evita divisões erradas */
    if (m_pvr_current_time < 5)
        return;

    /* Verifica filesystem */
    if (statvfs(m_mount_point.c_str(), &m_pvr_statfs) != 0)
    {
        DEBUG_MSG(OSD, ERROR,
                  "Erro statvfs em " << m_mount_point
                  << " (" << strerror(errno) << ")");
        return;
    }

    /* Verifica arquivo sendo gravado */
    if (stat(m_pvr_file_path.c_str(), &m_pvr_info) != 0)
    {
        printf("\nFile_NAME: %s \n", m_pvr_filename.c_str());
        printf("\nFile_PATH: %s \n", m_pvr_file_path.c_str());
        DEBUG_MSG(OSD, ERROR,
                  "Arquivo PVR ainda indisponível: "
                  << m_pvr_file_path
                  << " (" << strerror(errno) << ")");
        return;
    }

    m_pvr_file_size      = static_cast<uint64_t>(m_pvr_info.st_size);
    m_pvr_total_size     = static_cast<uint64_t>(m_pvr_statfs.f_blocks) *
                           static_cast<uint64_t>(m_pvr_statfs.f_frsize);
    m_pvr_available_size = static_cast<uint64_t>(m_pvr_statfs.f_bavail) *
                           static_cast<uint64_t>(m_pvr_statfs.f_frsize);

    /* Cálculo da taxa com suavização */
    static uint64_t last_size = 0;
    static uint64_t last_time = 0;

    if (last_time > 0 && m_pvr_current_time > last_time)
    {
        uint64_t delta_size = m_pvr_file_size - last_size;
        uint64_t delta_time = m_pvr_current_time - last_time;

        if (delta_time > 0)
            m_pvr_record_rate = delta_size / delta_time;
    }

    last_size = m_pvr_file_size;
    last_time = m_pvr_current_time;

    /* Proteção contra taxa inválida */
    if (m_pvr_record_rate == 0)
        return;

    m_pvr_max_rec_time = m_pvr_available_size / m_pvr_record_rate;

    /* Espaço em disco crítico */
    if (m_pvr_available_size <= ONE_MEGA_BYTE)
    {
        Task::s_task_player->pvr_record_stop();

        if (not m_message_box)
            m_message_box = std::make_unique<OSD_Message_Box>(nullptr);

        m_message_box->show_message_box_ok(
            std::bind(&OSD_Menu_Plus_Record::pvr_stop_record_callback, this),
            tr(__Sem_espaco_disponivel_no_disco));
    }
}


void OSD_Menu_Plus_Record::update_record_info()
{
    lv_label_set_text(m_lbl_prog_name, m_pvr_filename.data());
    std::transform(m_filesystem_type.begin(), m_filesystem_type.end(), m_filesystem_type.begin(), ::toupper);
    lv_label_set_text(m_lbl_fs, m_filesystem_type.data());
    char buf[80];
    sprintf(buf, "%2.2f MB", (double)m_pvr_file_size / (1024.0 * 1024.0));
    lv_label_set_text(m_lbl_file_size, buf);
    sprintf(buf, "%2.2f GB", (double)m_pvr_total_size / (1024.0 * 1024.0 * 1024.0));
    lv_label_set_text(m_lbl_total, buf);
    sprintf(buf, "%2.2f GB", (double)m_pvr_available_size / (1024.0 * 1024.0 * 1024.0));
    lv_label_set_text(m_lbl_available, buf);
    sprintf(buf, "%2.2f MB/s", m_pvr_record_rate / (1024.0 * 1024.0));
    lv_label_set_text(m_lbl_record_rate, buf);
    std::time_t max_rec_time = m_pvr_max_rec_time;
    std::strftime(buf, 80, "%H:%M:%S", localtime(&max_rec_time));
    lv_label_set_text(m_lbl_max_rec, buf);
}

void OSD_Menu_Plus_Record::record_time_cb(lv_timer_t *_tm)
{
    using std::chrono::system_clock;

    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Plus_Record *thiz = static_cast<OSD_Menu_Plus_Record *>(lv_timer_get_user_data(_tm));

    auto now_tmp = System::get_system_time().to_unix_epoch();
    auto now = system_clock::from_time_t(now_tmp);

    if((system_clock::to_time_t(thiz->m_time_to_end) > 0) and (thiz->m_time_to_end < now))
    {
        Task::s_task_player->pvr_record_stop();
        Task::post_event(std::bind(thiz->m_callback));
    }
    else
    {
        thiz->update_record_time();
    }
}

void OSD_Menu_Plus_Record::start_record_time_timer()
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

void OSD_Menu_Plus_Record::del_record_time_timer()
{
    DELETE_TIMER(m_record_time_timer);
}

void OSD_Menu_Plus_Record::record_info_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Plus_Record *thiz = static_cast<OSD_Menu_Plus_Record *>(lv_timer_get_user_data(_tm));
    thiz->update_record_info();
}

void OSD_Menu_Plus_Record::start_record_info_timer()
{
    // Inicia timer para ler informaçõesdo do video
    if (m_record_info_timer == nullptr)
    {
        m_record_info_timer = lv_timer_create(record_info_cb, 5000, this);
        lv_timer_set_repeat_count(m_record_info_timer, -1);
        update_record_info();
    }
    else
    {
        lv_timer_reset(m_record_info_timer);
    }
}

void OSD_Menu_Plus_Record::del_record_info_timer()
{
    DELETE_TIMER(m_record_info_timer);
}

}
