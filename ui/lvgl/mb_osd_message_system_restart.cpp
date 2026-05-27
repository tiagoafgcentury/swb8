#include <iostream>
#include <map>
#include <sstream>

#include "mb_osd_message_system_restart.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_message_box.h"

#include "mb_main.h"

#include "tasks/mb_task.h"
#include "hal/mb_system.h"
#include "common/mb_globals.h"

namespace mb {

OSD_Message_System_Restart::OSD_Message_System_Restart(OSD *_parent):
    OSD(_parent)
{
}

OSD_Message_System_Restart::~OSD_Message_System_Restart()
{
    DEBUG_MSG(OSD, DEBUG, "OSD_Message_System_Restart::~OSD_Message_System_Restart\n");
    DELETE_TIMER(m_exit_timer);
}

void OSD_Message_System_Restart::show_message_system_restart(osd_message_system_restart_cb _callback)
{
    DEBUG_MSG(OSD, DEBUG, "show_message_system_restart\n");
    m_callback = _callback;

    if(not m_message_box)
    {
        m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
    }

    char buffer[128];
    snprintf(buffer, sizeof(buffer), tr(__O_sistema_precisa_ser_reiniciado_em_x_segundos).data(), 10);
    std::string_view str = buffer;
    m_message_box->show_message_box(str);
    m_exit_timer = lv_timer_create(process_exit_cb, 1000, this);
    lv_timer_set_repeat_count(m_exit_timer, max_time_exit+1);


}

void OSD_Message_System_Restart::process_exit_cb(lv_timer_t *tm)
{
    OSD_Message_System_Restart *thiz = static_cast<OSD_Message_System_Restart *>(lv_timer_get_user_data(tm));
    thiz->m_count -= 1;
    if(thiz->m_count == 255)
    {
        thiz->system_restart_callback();
    }
    else
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), tr(__O_sistema_precisa_ser_reiniciado_em_x_segundos).data(), thiz->m_count);
        thiz->m_message_box->update_message(std::string(buffer));
    }
}

void OSD_Message_System_Restart::system_restart_callback()
{

    DELETE_TIMER(m_exit_timer);
    m_message_box.reset();

    //O ideal é que estas linhas abaixo funcionem.
    g_mbgui_reboot_after_exit.store(true, std::memory_order_release);
    g_mbgui_keep_running.store(false, std::memory_order_release);
    g_mbgui_restart_on_exit.store(false, std::memory_order_release);

    Task::post_event(m_callback);
}

} // namespace mb
