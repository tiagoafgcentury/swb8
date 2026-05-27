#include <iostream>
#include <map>
#include <sstream>

#include "mb_osd_program_reminder.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_message_box.h"
#include "mb_osd_menu_plus.h"

#include "tasks/mb_task.h"
#include "tasks/mb_task_player.h"

#include "common/mb_globals.h"
#include <hal/mb_system.h>

namespace mb {

OSD_Program_Reminder::OSD_Program_Reminder(OSD *_parent):
    OSD(_parent)
{
}

OSD_Program_Reminder::~OSD_Program_Reminder()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Program_Reminder::~OSD_Program_Reminder\n");
    DELETE_TIMER(m_exit_timer);
    DELETE_TIMER(m_change_channel_exit_timer);
}

void OSD_Program_Reminder::change_channel()
{
    Service_ID_t service_id = m_s_entry.service_id;
    for(auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
    {
        if(srv.service_id() == service_id)
        {
            Task::post_event_channel_change(POST_CALLER & srv);
            break;
        }
    }
}

void OSD_Program_Reminder::show_program_reminder(osd_program_reminder_cb _callback, ScheduleEntry _s_entry)
{
    DEBUG_MSG(OSD, DEBUG, "show_program_reminder\n");
    m_callback = _callback;
    m_s_entry = _s_entry;

    if(not m_message_box)
    {
        m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
    }

    if(_s_entry.operation == Schedule_Operation::RECORD)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), tr(__A_gravacao_do_programa_agendado_comecara_em_x_seg_deseja_continuar).data(), 10);
        m_message_box->show_message_box_yes_no(std::bind(&OSD_Program_Reminder::record_schedule_start_callback, this, std::placeholders::_1), buffer, true);
        m_exit_timer = lv_timer_create(process_exit_cb, 1000, this);
        lv_timer_set_repeat_count(m_exit_timer, max_time_exit+1);
    }
    else
    {
        m_message_box->show_message_box_ok(std::bind(&OSD_Program_Reminder::remember_schedule_start_callback, this), tr(__O_programa_agendado_comecara_em_10_segundos));
        m_change_channel_exit_timer = lv_timer_create(process_change_channel_cb, 1000, this);
        lv_timer_set_repeat_count(m_change_channel_exit_timer, max_time_exit+1);
    }
}

void OSD_Program_Reminder::process_change_channel_cb(lv_timer_t *tm)
{
    OSD_Program_Reminder *thiz = static_cast<OSD_Program_Reminder *>(lv_timer_get_user_data(tm));
    thiz->m_count -= 1;
    if(thiz->m_count == 255)
    {
        // Indica mudança de canal
        thiz->remember_schedule_start_callback();
    }
    else
    {
        // Procurar pelo texto '10' e substituir pelo valor de thiz->m_count
        std::string text = tr(__O_programa_agendado_comecara_em_10_segundos).data();
        if ( std::find(text.begin(), text.end(), '1') != text.end() &&
             std::find(text.begin(), text.end(), '0') != text.end() )
        {
            size_t pos = text.find("10");
            if (pos != std::string::npos)
            {
                text.replace(pos, 2, std::to_string(thiz->m_count));
            }
        }
        thiz->m_message_box->update_message(text);
    }
}

void OSD_Program_Reminder::process_exit_cb(lv_timer_t *tm)
{
    OSD_Program_Reminder *thiz = static_cast<OSD_Program_Reminder *>(lv_timer_get_user_data(tm));
    thiz->m_count -= 1;
    if(thiz->m_count == 255)
    {
        // Indica início de gravação
        thiz->record_schedule_start_callback(true);
    }
    else
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), tr(__A_gravacao_do_programa_agendado_comecara_em_x_seg_deseja_continuar).data(), thiz->m_count);
        thiz->m_message_box->update_message(std::string(buffer));
    }
}

void  OSD_Program_Reminder::remember_schedule_start_callback()
{
    DEBUG("Changing channel as per schedule reminder\n");
    DELETE_TIMER(m_change_channel_exit_timer);
    change_channel();
    m_message_box.reset();
    Task::post_event(m_callback);
}

void  OSD_Program_Reminder::record_schedule_start_callback(bool _result)
{
    if(_result)
    {
        DELETE_TIMER(m_exit_timer);
        Task::post_event_start_recording(m_s_entry.service_id, m_s_entry.time_to_end);
    }

    m_message_box.reset();
    Task::post_event(m_callback);
}

} // namespace mb
