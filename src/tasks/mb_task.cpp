#include "mb_task.h"

#include "common/mb_globals.h"
#include "common/mb_lineup.h"

#if defined(MBGUI_APP_GUI) or defined(MBGUI_APP_CAS)
#define MBGUI_HAS_IPC 1
#else
#define MBGUI_HAS_IPC 0
#endif

#if defined(MBGUI_APP_GUI)
#include "mb_task_demux.h"
#include "mb_task_player.h"
#include "mb_task_tuner.h"
#include "mb_task_osd.h"
#include "mb_task_application.h"
#include "mb_task_database.h"
#include "mb_task_remote_control.h"
#include "mb_zone_id.h"
#endif // MBGUI_APP_GUI

#if defined(MBGUI_APP_SAT_MONITOR)
#include "mb_task_demux.h"
#include "mb_task_tuner.h"
#include "mb_task_osd.h"
#include "mb_task_remote_control.h"
#include "mb_zone_id.h"
#endif // MBGUI_APP_SAT_MONITOR

#if defined(MBGUI_APP_GUI) or defined(MBGUI_APP_CAS)
#include "mb_task_cas.h"
#endif // MBGUI_APP_GUI

#ifdef MB_USE_MICROHTTPD
#include "tpm/mb_task_http_server.h"
#endif

#ifdef MBGUI_SAT_MONITOR
#include "sm_task_sat_monitor.h"
#endif

#ifndef NDEBUG
#undef basename
#include <string.h>
#include "hal/mb_remote_control_keys.h"
#endif

#include "private/mb_task_dump_params.h"
#include "private/mb_task_ipc_messages.h"
#include "private/mb_task_macros.h"
#include "private/mb_task_mqueue_helpers.h"

#include <sys/stat.h>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace mb {

// Private
mqd_t Task::s_queue_cas = {};
mqd_t Task::s_queue_gui = {};

std::atomic<int> Task::s_queue_use_count = {};

std::vector<Task::Event> Task::s_events;
std::vector<Task::Timed_Event> Task::s_timed_events;
std::vector<Task::Timed_Event> Task::s_timed_events_alt;
bool Task::s_events_in_progress { false };

std::mutex Task::s_events_async_mutex;
std::vector<Task::Event> Task::s_events_async;

bool thread_local t_is_main_thread = false;
std::mutex Task::s_ipc_mutex;
std::chrono::steady_clock::time_point Task::s_last_ipc_reopen{};

Task::Task()
{
    t_is_main_thread = true;

#if MBGUI_HAS_IPC
    {
        if(s_queue_use_count.fetch_add(1) == 0)
        {
            // Allowed Permissions rw-rw----
            std::lock_guard<std::mutex> lock(s_ipc_mutex);
            auto old_umask = umask(0117);
            auto _ = defer([old_umask]
            {
                umask(old_umask);
            });
            s_queue_local = open_local_queue(MBGUI_SOCKET_PATH_LOCAL);
            struct mq_attr attr;
            auto ret = mq_getattr(s_queue_local, &attr);
            mb_assert(ret != -1);

            if(ret != -1)
            {
                mb_assert(attr.mq_msgsize >= MBGUI_SOCKET_MSGSIZE);
            }

#if defined(MBGUI_APP_GUI)
            s_queue_cas = open_remote_queue(MBGUI_SOCKET_PATH_CAS);
#elif defined(MBGUI_APP_CAS)
            s_queue_gui = open_remote_queue(MBGUI_SOCKET_PATH_GUI);
#elif defined(MBGUI_APP_TPM)
            // Nothing - TPM does not use IPC
#else
#error Missing MBGUI_APP_* define to set correct app
#endif
        }
    }
#endif // MBGUI_HAS_IPC
}

Task::~Task()
{
    if(s_queue_use_count.fetch_sub(1) == 1)
    {
        std::lock_guard<std::mutex> lock(s_ipc_mutex);
        if(s_queue_cas > 0)
        {
            mq_close(s_queue_cas);
            s_queue_cas = -1;
        }

        if(s_queue_gui > 0)
        {
            mq_close(s_queue_gui);
            s_queue_gui = -1;
        }
    }
}

namespace {

    PROFILE_PREPARE_TASK(PROCESS_APPLICATION);
    PROFILE_PREPARE_TASK(PROCESS_CAS);
    PROFILE_PREPARE_TASK(PROCESS_DATABASE);
    PROFILE_PREPARE_TASK(PROCESS_DEMUX);
    PROFILE_PREPARE_TASK(PROCESS_EIT_EVENTS);
    PROFILE_PREPARE_TASK(PROCESS_HTTP);
    PROFILE_PREPARE_TASK(PROCESS_OSD);
    PROFILE_PREPARE_TASK(PROCESS_PLAYER);
    PROFILE_PREPARE_TASK(PROCESS_REMOTE_CONTROL);
    PROFILE_PREPARE_TASK(PROCESS_SAT_MONITOR);
    PROFILE_PREPARE_TASK(PROCESS_TUNER);

}

bool Task::run_processes()
{
#ifdef MBGUI_PERIODIC_DUMP
    static std::chrono::steady_clock::time_point s_last_dump;
    auto now = decltype(s_last_dump)::clock::now();

    if(now - s_last_dump >= 5s)
    {
        POST_EVENT_TO_ALL(handle_event_debbug_dump_status);
        s_last_dump = now;

        PROFILE_DUMP_TASK(PROCESS_APPLICATION);
        PROFILE_DUMP_TASK(PROCESS_CAS);
        PROFILE_DUMP_TASK(PROCESS_DATABASE);
        PROFILE_DUMP_TASK(PROCESS_DEMUX);
        PROFILE_DUMP_TASK(PROCESS_EIT_EVENTS);
        PROFILE_DUMP_TASK(PROCESS_HTTP);
        PROFILE_DUMP_TASK(PROCESS_OSD);
        PROFILE_DUMP_TASK(PROCESS_PLAYER);
        PROFILE_DUMP_TASK(PROCESS_REMOTE_CONTROL);
        PROFILE_DUMP_TASK(PROCESS_SAT_MONITOR);
        PROFILE_DUMP_TASK(PROCESS_TUNER);
    }

#endif // MBGUI_PERIODIC_DUMP
    PROFILE_TASK(PROCESS_APPLICATION);
    PROFILE_TASK(PROCESS_CAS);
    PROFILE_TASK(PROCESS_DATABASE);
    PROFILE_TASK(PROCESS_DEMUX);
    PROFILE_TASK(PROCESS_EIT_EVENTS);
    PROFILE_TASK(PROCESS_HTTP);
    PROFILE_TASK(PROCESS_OSD);
    PROFILE_TASK(PROCESS_PLAYER);
    PROFILE_TASK(PROCESS_REMOTE_CONTROL);
    PROFILE_TASK(PROCESS_SAT_MONITOR);
    PROFILE_TASK(PROCESS_TUNER);
    process_events();
    process_ipc_messages();
    return s_events.empty();
}

void Task::send_ipc_message(mqd_t _mqdes, const uint8_t *_msg_ptr, size_t _msg_len)
{
#if MBGUI_HAS_IPC
    int retry = 0;
RETRY_SEND_MESSAGE:
    DEBUG_MSG(TASK, DEBUG, "Send IPC Message(" << retry << "): " << dump_params(_msg_ptr, std::min<size_t>(_msg_len, 30u)) << (_msg_len > 30u ? "..." : "") << "\n");
    auto ret = mq_send(_mqdes, (const char *)_msg_ptr, _msg_len, PRIORITY_NORMAL);

    if(ret != 0)
    {
        switch(errno)
        {
            case EAGAIN:
            {
                DEBUG_MSG(TASK, WARN, "The queue was full, and the O_NONBLOCK flag was set for the message queue description referred to by mqdes - ");

                if(retry++ < 200)
                {
                    DEBUG_MSG(TASK, WARN, "will retry\n");
                    std::this_thread::sleep_for(10ms);
                    goto RETRY_SEND_MESSAGE;
                }
                else
                {
                    DEBUG_MSG(TASK, ERROR, "Too many retries, dropping message.\n");
                    //exit(EXIT_FAILURE);
                }

                break;
            }

            case EBADF:
            {
                DEBUG_MSG(TASK, ERROR, "The descriptor specified in mqdes was invalid or not opened for writing - Attempting to re-open\n");
                std::lock_guard<std::mutex> lock(s_ipc_mutex);
                auto now = std::chrono::steady_clock::now();
                if (now - s_last_ipc_reopen > 1s)
                {
                    if (_mqdes == s_queue_cas)
                    {
                        if (s_queue_cas >= 0) mq_close(s_queue_cas);
                        s_queue_cas = open_remote_queue(MBGUI_SOCKET_PATH_CAS);
                        s_last_ipc_reopen = now;
                    }
                    else if (_mqdes == s_queue_gui)
                    {
                        if (s_queue_gui >= 0) mq_close(s_queue_gui);
                        s_queue_gui = open_remote_queue(MBGUI_SOCKET_PATH_GUI);
                        s_last_ipc_reopen = now;
                    }
                }
                break;
            }
            case EINTR:
                DEBUG_MSG(TASK, ERROR, "The call was interrupted by a signal handler; see signal(7).\n");
                break;

            case EINVAL:
                DEBUG_MSG(TASK, ERROR, "The call would have blocked, and abs_timeout was invalid, either because tv_sec was less than zero, or because tv_nsec was less than zero or greater than 1000 million.\n");
                break;

            case EMSGSIZE:
                DEBUG_MSG(TASK, ERROR, "msg_len was greater than the mq_msgsize attribute of the message queue.\n");
                break;

            case ETIMEDOUT:
                DEBUG_MSG(TASK, ERROR, "The call timed out before a message could be transferred.\n");
                break;

            default:
                DEBUG_MSG(TASK, ERROR, "mq_send error: " << strerror(errno) << "\n");
                break;
        }
    }

    mb_assert(ret == 0);
#else
    (void)_mqdes;
    (void)_msg_ptr;
    (void)_msg_len;
#endif // MBGUI_HAS_IPC
}

void Task::process_ipc_messages()
{
#if MBGUI_HAS_IPC
    uint8_t buffer[MBGUI_SOCKET_MSGSIZE];
    MB_ZERO(buffer);
    unsigned int msg_prio = {};
    auto ssize = mq_receive(s_queue_local, (char *)buffer, sizeof(buffer), &msg_prio);

    if(ssize == -1)
    {
        auto err = errno;

        switch(err)
        {
            case EAGAIN:
                return;

            case EINTR:
                DEBUG_MSG(TASK, WARN, "The call was interrupted by a signal handler\n");
                return;
#ifdef NDEBUG

            case EBADF:
                DEBUG_MSG(TASK, ERROR, "The descriptor specified in mqdes was invalid or not opened for reading.\n");
                return;

            case EINVAL:
                DEBUG_MSG(TASK, ERROR, "The call would have blocked, and abs_timeout was invalid\n");
                return;

            case EMSGSIZE:
                DEBUG_MSG(TASK, ERROR, "The descriptor specified in mqdes was invalid or not opened for reading - Attempting to re-open\n");
                s_queue_local = open_local_queue(MBGUI_SOCKET_PATH_LOCAL);
                return;

            case ETIMEDOUT:
                DEBUG_MSG(TASK, ERROR, "The call timed out before a message could be transferred.\n");
                return;
#endif

            case EBADF:
            {
                DEBUG_MSG(TASK, ERROR, "The descriptor specified in mqdes was invalid or not opened for reading - Attempting to re-open\n");
                std::lock_guard<std::mutex> lock(s_ipc_mutex);
                auto now = std::chrono::steady_clock::now();
                if (now - s_last_ipc_reopen > 1s)
                {
                    if (s_queue_local >= 0) mq_close(s_queue_local);
                    s_queue_local = open_local_queue(MBGUI_SOCKET_PATH_LOCAL);
                    s_last_ipc_reopen = now;
                }
                return;
            }

            default:
                DEBUG_MSG(TASK, ERROR, "Unknown mq_receive error: " << err << "\n");
                return;
        }
    }

    DEBUG_MSG(TASK, DEBUG, "Recieve IPC Message: " << dump_params(buffer, std::min<int>(ssize, 30)) << (ssize > 30 ? "..." : "") << "\n");
    auto msg = reinterpret_cast<const IPC_Message *>(buffer);

    switch(msg->cmd)
    {
        case ipc_cmd_cas_fingerprint_get:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_fingerprint_get);
#else
            mb_assert(false);
#endif // MBGUI_APP_CAS
            break;
        }

        case ipc_cmd_cas_popup_display_message:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            auto evt = reinterpret_cast<const MSG_CAS_Popup_Message *>(buffer);
            Event_Display_Message event;
            event.message = std::string_view(evt->payload.message, evt->payload.size);
            event.category = evt->payload.category;
            POST_EVENT_TO_ALL(handle_event_osd_display_message, event);
#endif // MBGUI_APP_CAS
            break;
        }

        case ipc_cmd_cas_fingerprint_ready:
        {
#if defined(MBGUI_APP_GUI)
            auto evt = reinterpret_cast<const MSG_CAS_Fingerprint *>(buffer);
            Event_CAS_Fingerprint event;
            event.nuid = evt->payload.nuid;
            event.caid = evt->payload.caid;
            event.scua = evt->payload.scua;
            event.cak_version = evt->payload.cak_version;
            event.project_info = evt->payload.project_info;
            event.chipset_type = evt->payload.chipset_type;
            event.chipset_revision = evt->payload.chipset_revision;
            POST_EVENT_TO_ALL(handle_event_cas_fingerprint_ready, event);
#else
            mb_assert(false);
#endif // MBGUI_APP_GUI
            break;
        }

        case ipc_cmd_cas_request_descramble:
        {
#if defined(MBGUI_APP_CAS)
            Event_CAS_Request_Descramble event;
            auto evt = reinterpret_cast<const MSG_CAS_Request_Descramble *>(buffer);
            evt->payload.copy_to_event(event);
            POST_EVENT_TO_ALL(handle_event_cas_request_descramble, event);
#else
            mb_assert(false);
#endif // MBGUI_APP_GUI
            break;
        }

        case ipc_cmd_cas_request_descramble_done:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            auto evt = reinterpret_cast<const MSG_CAS_Request_Descramble_Done *>(buffer);
            POST_EVENT_TO_ALL(handle_event_cas_request_descramble_done, evt->payload);
#endif // MBGUI_APP_GUI
            break;
        }

        case ipc_cmd_cas_request_descramble_pmt_update:
        {
#if defined(MBGUI_APP_CAS)
            Event_CAS_Request_Descramble event;
            auto evt = reinterpret_cast<const MSG_CAS_Request_Descramble *>(buffer);
            evt->payload.copy_to_event(event);
            POST_EVENT_TO_ALL(handle_event_cas_request_descramble_pmt_update, event);
#else
            mb_assert(false);
#endif // MBGUI_APP_GUI
            break;
        }

        case ipc_cmd_cas_request_descramble_stop:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_request_descramble_stop);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_descramble_stop_done:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            POST_EVENT_TO_ALL(handle_event_cas_request_descramble_stop_done);
#endif
            break;
        }

        case ipc_cmd_cas_system_need_exit:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_exit);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_switch_folder:
        {
#if defined(MBGUI_APP_CAS)
            auto evt = reinterpret_cast<const MSG_CAS_Switch_Folder *>(buffer);
            POST_EVENT_TO_ALL(handle_event_cas_switch_folder, evt->is_sky);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_system_factory_reset:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_system_factory_reset);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_system_factory_reset_done:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            POST_EVENT_TO_ALL(handle_event_system_factory_reset_done);
#endif
            break;
        }

        case ipc_cmd_cas_system_need_reset:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            POST_EVENT_TO_ALL(handle_event_system_need_reset);
#endif
            break;
        }

        case ipc_cmd_cas_send_cat_table_section:
        {
#if defined(MBGUI_APP_CAS)
            Event_CAS_CAT_Table_Section event;
            auto evt = reinterpret_cast<const MSG_CAS_CAT_Table_Section *>(buffer);
            evt->payload.copy_to_event(event);
            POST_EVENT_TO_ALL(handle_event_cas_send_cat_table_section, event);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_save_zone_id:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            auto evt = reinterpret_cast<const MSG_CAS_Zone_Id_Save *>(buffer);
            POST_EVENT_TO_ALL(handle_event_lineup_save_zone_id, evt->payload.zone_id, evt->payload.segment_id);
#endif
            break;
        }

        case ipc_cmd_player_change_audio:
        {
#if defined(MBGUI_APP_CAS)
            auto evt = reinterpret_cast<const MSG_CAS_Audio_Changed *>(buffer);
            POST_EVENT_TO_ALL(handle_event_player_change_audio, evt->payload);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_send_pvr_status:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            Event_PVR_Status event = {};
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Send_Status *>(buffer);
            if(evt->payload.copy_to_event(event))
            {
                POST_EVENT_TO_ALL(handle_event_cas_pvr_get_status, event);
            }
            else
            {

                POST_EVENT_TO_ALL(handle_event_cas_pvr_get_status, event);
            }
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_timeshift_start:
        {
#if defined(MBGUI_APP_CAS)
            Event_PVR_Record_Param event;
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Record_Param *>(buffer);

            evt->payload.copy_to_event(event);

            POST_EVENT_TO_ALL(handle_event_cas_pvr_timeshift_start, event);
#else
            mb_assert(false);
#endif // MBGUI_APP_GUI
            break;

        }

        case ipc_cmd_cas_request_pvr_timeshift_play:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_timeshift_play);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_timeshift_stop:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_timeshift_stop);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_record_start:
        {
#if defined(MBGUI_APP_CAS)
            Event_PVR_Record_Param event;
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Record_Param *>(buffer);

            evt->payload.copy_to_event(event);

            POST_EVENT_TO_ALL(handle_event_cas_pvr_record_start, event);
#else
            mb_assert(false);
#endif // MBGUI_APP_GUI
            break;

        }

        case ipc_cmd_cas_request_pvr_record_stop:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_record_stop);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_record_pause:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_pause);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_record_resume:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_record_resume);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_start:
        {
#if defined(MBGUI_APP_CAS)
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Url *>(buffer);
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_start, evt->payload);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_stop:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_stop);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_pause:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_pause);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_resume:
        {
#if defined(MBGUI_APP_CAS)
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_resume);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_forward:
        {
#if defined(MBGUI_APP_CAS)
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Speed *>(buffer);
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_forward, evt->payload);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_rewind:
        {
#if defined(MBGUI_APP_CAS)
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Speed *>(buffer);
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_rewind, evt->payload);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_cas_request_pvr_play_next:
        {
#if defined(MBGUI_APP_CAS)
            auto evt = reinterpret_cast<const MSG_CAS_PVR_Url *>(buffer);
            POST_EVENT_TO_ALL(handle_event_cas_pvr_play_next, evt->payload);
#else
            mb_assert(false);
#endif
            break;
        }

        case ipc_cmd_reset_pin_code:
        {
#if defined(MBGUI_APP_CAS)
            mb_assert(false);
#else
            POST_EVENT_TO_ALL(handle_event_pin_reset);
#endif
            break;
        }

        default:
            DEBUG_MSG(TASK, ERROR, "IPC message not handled: " << msg->cmd << endl);
            mb_assert(false);
            break;
    };

#endif // MBGUI_HAS_IPC
}

void Task::process_events()
{
    s_events_in_progress = true;
    deferred_call _([] { s_events_in_progress = false; });

    // Normal Events
    if(not s_events.empty())
    {
        auto ev = std::move(s_events);

        for(const auto &e : ev)
        {
            e.callback();
        }
    }

    // Events from other threads
    if (s_events_async_mutex.try_lock())
    {
        if (not s_events_async.empty())
        {
            auto ev = std::move(s_events_async);
            s_events_async_mutex.unlock();

            for(const auto &e : ev)
            {
                e.callback();
            }
        }
        else
        {
            s_events_async_mutex.unlock();
        }
    }

    // Timed Events
    if(!s_timed_events.empty())
    {
        std::vector<decltype(s_timed_events)::iterator> erase_items;

        for(auto it = s_timed_events.begin(); it != s_timed_events.end(); it++)
        {
            if(it->time_point <= now())
            {
                it->callback();

                if(it->interval.count() == 0)
                {
                    erase_items.push_back(it);
                }
                else
                {
                    it->time_point = now() + it->interval;
                }
            }
        }

        if(erase_items.size() == s_timed_events.size())
        {
            s_timed_events.clear();
        }
        else
        {
            for(auto it = erase_items.rbegin(); it != erase_items.rend(); it++)
            {
                s_timed_events.erase(*it);
            }
        }

        if(!s_timed_events_alt.empty())
        {
            s_timed_events.insert(s_timed_events.end(),
                                  std::make_move_iterator(s_timed_events_alt.begin()),
                                  std::make_move_iterator(s_timed_events_alt.end())
                                 );
            s_timed_events_alt.clear();
        }
    }
}

void Task::post_event(Event_Callback _callback)
{
    if (t_is_main_thread)
    {
        s_events.emplace_back(std::move(_callback));
    }
    else
    {
        std::lock_guard<std::mutex> guard(s_events_async_mutex);
        s_events_async.emplace_back(std::move(_callback));
    }
}

void Task::post_event(std::chrono::time_point<std::chrono::steady_clock> _tp, Event_Callback _callback)
{
    if(!s_events_in_progress)
    {
        s_timed_events.emplace_back(std::move(_tp), std::chrono::seconds(0), std::move(_callback));
    }
    else
    {
        s_timed_events_alt.emplace_back(std::move(_tp), std::chrono::seconds(0), std::move(_callback));
    }
}

void Task::post_timer(std::chrono::milliseconds _tp, Event_Callback _callback)
{
    if(!s_events_in_progress)
    {
        s_timed_events.emplace_back(std::chrono::steady_clock::now() + _tp, std::move(_tp), std::move(_callback));
    }
    else
    {
        s_timed_events_alt.emplace_back(std::chrono::steady_clock::now() + _tp, std::move(_tp), std::move(_callback));
    }
}

void Task::post_event_application_ready()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_application_ready);
}

void Task::post_event_easy_install_save(bool _easy_install_finish)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_easy_install_save, _easy_install_finish);
}

void Task::post_event_application_state_save(const Event_Save_Application_State &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_application_state_save, _event);
}

void Task::post_event_application_state_load()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_application_state_load);
}

void Task::post_event_application_state_loaded(const Event_Save_Application_State &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_application_state_loaded, _event);
}

void Task::post_event_blind_scan_progress(std::weak_ptr<Event_Blind_Scan_Progress> _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_blind_scan_progress, _event);
}

void Task::post_event_autodetect_lnbf_start(std::weak_ptr<Event_Autodetect_LNBf> _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_autodetect_lnbf_start, _event);
}

void Task::post_event_set_lnbf_type(LNBF_Type _lnbf_type)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_set_lnbf_type, _lnbf_type)
}

void Task::post_event_set_default_lnbf()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_set_default_lnbf);
}

void Task::post_event_cas_start_emm_filtering(const Transponder *_tp)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_cas_start_emm_filtering, _tp);
}

void Task::post_event_cc(Event_CC _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_cc, _event);
}

void Task::post_event_subtitle(Event_Subtitle_Image _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_subtitle, _event);
}

void Task::post_event_cc_enable(CC_Type _type)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_cc_enable, _type);
}

void Task::post_event_autodetect_lnbf_finished(bool _success)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_autodetect_lnbf_finished, _success);
}

void Task::post_event_system_memory_is_low()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_system_memory_is_low);
}

#if MBGUI_HAS_IPC
void Task::post_event_system_need_reset()
{
    NOOP;
#if defined(MBGUI_APP_GUI)
    POST_EVENT_TO_ALL(handle_event_system_need_reset);
#else
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_system_need_reset,
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_system_need_reset);
#endif
}

void Task::post_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event)
{
    NOOP;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // msg.payload.char[] cannot be initialized below, has to be set with memset
    auto msg = MSG_CAS_Fingerprint
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_fingerprint_ready,
        },
    };
#pragma GCC diagnostic pop
    MSGCPY(nuid);
    MSGCPY(caid);
    MSGCPY(scua);
    MSGCPY(cak_version);
    MSGCPY(project_info);
    MSGCPY(chipset_type);
    MSGCPY(chipset_revision);
    POST_IPC_EVENT_TO_GUI(msg, handle_event_cas_fingerprint_ready, _event);
}

void Task::post_event_cas_popup_message(Event_Display_Message _event)
{
    NOOP;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // msg.payload.message cannot be initialized below, has to be set with memset
    auto msg = MSG_CAS_Popup_Message
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_popup_display_message,
        },
        .payload = {
            .category = _event.category,
            .size = std::min<uint16_t>(_event.message.size(), MSG_CAS_POPUP_MESSAGE_PAYLOAD_SIZE),
        },
    };
#pragma GCC diagnostic pop
    MSGCPY(message);
    POST_IPC_EVENT_TO_GUI(msg, handle_event_osd_display_message, _event);
}

void Task::post_event_cas_fingerprint_get()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_fingerprint_get,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_fingerprint_get);
}

void Task::post_event_cas_request_descramble(Event_CAS_Request_Descramble _event)
{
    NOOP;
    auto msg = MSG_CAS_Request_Descramble
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_descramble,
        },
        .payload = _event,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_request_descramble, _event);
}

void Task::post_event_cas_request_descramble_done(bool _result)
{
    NOOP;
    auto msg = MSG_CAS_Request_Descramble_Done
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_descramble_done,
        },
        .payload = _result,
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_cas_request_descramble_done, _result);
}

void Task::post_event_cas_request_descramble_pmt_update(Event_CAS_Request_Descramble _event)
{
    NOOP;
    auto msg = MSG_CAS_Request_Descramble
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_descramble_pmt_update,
        },
        .payload = _event,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_request_descramble_pmt_update, _event);
}

void Task::post_event_cas_request_descramble_stop()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_descramble_stop,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_request_descramble_stop);
}

void Task::post_event_cas_request_descramble_stop_done()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_descramble_stop_done,
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_cas_request_descramble_stop_done);
}

void Task::post_event_cas_send_cat_table_section(Event_CAS_CAT_Table_Section _event)
{
    NOOP;
    auto msg = MSG_CAS_CAT_Table_Section
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_send_cat_table_section,
        },
        .payload = _event,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_send_cat_table_section, _event);
}

void Task::post_event_cas_pvr_timeshift_start(Event_PVR_Record_Param _event)
{
    NOOP;
    auto msg = MSG_CAS_PVR_Record_Param
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_pvr_timeshift_start,
        },
        .payload = _event,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_timeshift_start, _event);
}

void Task::post_event_cas_pvr_timeshift_play()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_timeshift_play,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_timeshift_play);
}

void Task::post_event_cas_pvr_timeshift_stop()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_timeshift_stop,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_timeshift_stop);
}

void Task::post_event_cas_pvr_record_start(Event_PVR_Record_Param _event)
{
    NOOP;
    auto msg = MSG_CAS_PVR_Record_Param
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_pvr_record_start,
        },
        .payload = _event,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_record_start, _event);
}

void Task::post_event_cas_pvr_record_stop()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_record_stop,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_record_stop);
}

void Task::post_event_cas_pvr_record_pause()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_record_pause,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_record_pause);
}

void Task::post_event_cas_pvr_record_resume()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_record_resume,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_record_resume);
}

void Task::post_event_cas_pvr_play_start(std::string _url)
{
    NOOP;
    MSG_CAS_PVR_Url msg{};
    msg.header.source = MBGUI_IPC_SOURCE;
    msg.header.cmd    = ipc_cmd_cas_request_pvr_play_start;
    std::snprintf(msg.payload, sizeof(msg.payload), "%s", _url.c_str());
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_start, _url);
}

void Task::post_event_cas_pvr_play_stop()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_play_stop,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_stop);
}

void Task::post_event_cas_pvr_play_pause()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_play_pause,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_pause);
}

void Task::post_event_cas_pvr_play_resume()
{
    NOOP;
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_request_pvr_play_resume,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_resume);
}

void Task::post_event_cas_pvr_play_forward(uint16_t _mp_speed)
{
    NOOP;
    auto msg = MSG_CAS_PVR_Speed
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_pvr_play_forward,
        },
        .payload = _mp_speed,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_forward, _mp_speed);
}

void Task::post_event_cas_pvr_play_rewind(uint16_t _mp_speed)
{
    NOOP;
    auto msg = MSG_CAS_PVR_Speed
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_request_pvr_play_rewind,
        },
        .payload = _mp_speed,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_rewind, _mp_speed);
}

void Task::post_event_cas_pvr_play_next(std::string _url)
{
    NOOP;
    MSG_CAS_PVR_Url msg{};
    msg.header.source = MBGUI_IPC_SOURCE;
    msg.header.cmd    = ipc_cmd_cas_request_pvr_play_next;
    std::snprintf(msg.payload, sizeof(msg.payload), "%s", _url.c_str());
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_pvr_play_next, _url);
}

#endif // MBGUI_HAS_IPC

void Task::post_event_add_satellite(Satellite satellite)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_add_satellite, satellite);
}

void Task::post_event_autodetect_progress(Event_Transponder_data _progress)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_autodetect_progress, _progress);
}

void Task::post_event_update_satellite(Satellite satellite)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_update_satellite, satellite);
}

void Task::post_event_delete_satellite(unsigned int id)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_delete_satellite, id);
}

void Task::post_event_starting_application()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_starting_application);
}

void Task::post_event_service_pmt_section(PID_t _pid, Service_ID_t _service_id, DVB_Table_Section _pmt)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_service_pmt_section, _pid, _service_id, _pmt);
}

void Task::post_event_service_pmt_get_next_section(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_service_pmt_get_next_section, _pid, _service_id, _current_version_number);
}

void Task::post_event_process_automatic_search()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_process_automatic_search);
}

void Task::post_event_lineup_satellite_found()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_satellite_found);
}

void Task::post_event_lineup_build(std::weak_ptr<Event_List_Update> _event, bool restart)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_build, _event, restart);
}

void Task::post_event_lineup_update(std::weak_ptr<Event_List_Update_Channel_List> _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_update, _event);
}

void Task::post_event_lineup_ready(const Event_Lineup_Ready &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_ready, _event);
}

void Task::post_event_lineup_save()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_save);
}

void Task::post_event_lineup_save_zone_id(Zone_ID_t _zone_id, Segment_ID_t _segment_id)
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = MSG_CAS_Zone_Id_Save
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_save_zone_id,
        },
        .payload = {_zone_id, _segment_id},
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_lineup_save_zone_id, _zone_id, _segment_id);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_lineup_save_zone_id, _zone_id, _segment_id);
}

void Task::post_event_cas_pvr_get_status(Event_PVR_Status _event)
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = MSG_CAS_PVR_Send_Status
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_send_pvr_status,
        },
        .payload = _event,
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_cas_pvr_get_status, _event);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_cas_pvr_get_status, _event);
}

void Task::post_event_lineup_start()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_start);
}

void Task::post_event_lineup_changed()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_changed);
}

void Task::post_event_lineup_load()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lineup_load);
}

void Task::post_event_osd_display_message(Event_Display_Message _message)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_display_message, _message);
}

void Task::post_event_osd_menu_plus(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_menu_plus, _call_pvr, _call_sleep_timer, _time_to_end);
}

void Task::post_event_ota_found()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_ota_found);
}

void Task::post_event_osd_closed_caption()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_closed_caption);
}

void Task::post_event_osd_audio_lr()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_audio_lr);
}

void Task::post_event_osd_production_info()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_production_info);
}

void Task::post_event_osd_factory_reset()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_factory_reset);
}

void Task::post_event_osd_mainmenu_show()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_mainmenu_show);
}

void Task::post_event_osd_channel_list_show()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_osd_channel_list_show);
}

void Task::post_event_player_change_audio(PID_t _new_pid)
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = MSG_CAS_Audio_Changed
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_player_change_audio,
        },
        .payload = _new_pid,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_player_change_audio, _new_pid);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_player_change_audio, _new_pid);
}

void Task::post_event_player_restart()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_player_restart);
}

void Task::post_event_player_started()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_player_started);
}

void Task::post_event_player_stop()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_player_stop);
}

void Task::post_event_player_stopped()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_player_stopped);
}

void Task::post_event_media_player_start(std::string url, uint8_t _mode)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_start, url, _mode);
}

void Task::post_event_media_player_stop()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_stop);
}

void Task::post_event_media_player_pause()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_pause);
}

void Task::post_event_media_player_resume()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_resume);
}

void Task::post_event_media_player_next(std::string url, uint8_t _mode)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_next, url, _mode);
}

void Task::post_event_media_player_previous(std::string url, uint8_t _mode)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_previous, url, _mode);
}

void Task::post_event_media_player_forward(uint16_t _mp_speed, bool _is_video)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_forward, _mp_speed, _is_video);
}

void Task::post_event_media_player_rewind(uint16_t _mp_speed, bool _is_video)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_media_player_rewind, _mp_speed, _is_video);
}

void Task::post_event_pvr_start()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_pvr_start);
}

void Task::post_event_pvr_playback_start(std::string url)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_pvr_playback_start, url);
}

void Task::post_event_pvr_timeshift_start(std::string url)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_pvr_timeshift_start, url);
}

void Task::post_event_channel_change(POST_CALLER_P Service *_srv)
{
    NOOP;
    POST_DUMP;
    POST_EVENT_TO_ALL(handle_event_channel_change, _srv);
}

void Task::post_event_transponder_lock(POST_CALLER_P const Transponder *_tp, bool _force)
{
    NOOP;
    POST_DUMP;
    POST_EVENT_TO_ALL(handle_event_transponder_lock, _tp, _force);
}

void Task::post_event_transponder_locked(const Event_Tuner_Lock &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_transponder_locked, _event);
}

void Task::post_event_channel_changed(POST_CALLER_P Service *_srv)
{
    NOOP;
    POST_DUMP;
    POST_EVENT_TO_ALL(handle_event_channel_changed, _srv);
}

void Task::post_event_channel_preview(POST_CALLER_P Service *_srv)
{
    NOOP;
    POST_DUMP;
    POST_EVENT_TO_ALL(handle_event_channel_preview, _srv);
}

void Task::post_event_display_clear()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_display_clear);
}

void Task::post_event_satellite_list_load(std::function<void(std::vector<Satellite>)> _cb)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_satellite_list_load, _cb);
}

void Task::post_event_change_lnbf_type(std::function<void(std::string)> _callback)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_change_lnbf_type, _callback)
}

void Task::post_event_schedule_load(std::function<void(std::vector<ScheduleEntry>)> _schedule)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_schedule_load, _schedule);
}

void Task::post_event_insert_schedule(ScheduleEntry _entry)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_insert_schedule, _entry);
}

void Task::post_event_delete_schedule(int _agenda_id)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_delete_schedule, _agenda_id);
}

void Task::post_event_update_schedule(ScheduleEntry _entry)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_update_schedule, _entry);
}

void Task::post_event_update_channel_list(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_data)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_update_channel_list, _channel_list_type, _channel_data);
}

void Task::post_event_ota_update_get(PID_t _pid, std::weak_ptr<Event_OTA_DSI> _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_ota_update_get, _pid, _event);
}

void Task::post_event_dvb_subtitle_get(PID_t _pid)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_dvb_subtitle_get, _pid);
}

void Task::post_event_dvb_subtitle_del(PID_t _pid)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_dvb_subtitle_del, _pid);
}

void Task::post_event_clock_need_update()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_clock_need_update);
}

void Task::post_event_clock_set_time(const Event_Time &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_clock_set_time, _event);
}

void Task::post_event_clock_time_set(bool _success)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_clock_time_set, _success);
}

void Task::post_event_save_terms_of_use()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_save_terms_of_use);
}

void Task::post_event_eit_update()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_eit_update);
}

void Task::post_event_services_update()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_services_update);
}

void Task::post_event_sound_changed(const Event_Sound &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_sound_changed, _event);
}

void Task::post_event_change_osd_home_state()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_change_osd_home_state);
}

void Task::post_event_change_osd_media_player_state()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_change_osd_media_player_state);
}

void Task::post_event_system_display_settings_save(const Event_System_Settings &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_system_display_settings_save, _event);
}

void Task::post_event_system_display_settings_saved()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_system_display_settings_saved);
}

void Task::post_event_system_display_settings_load()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_system_display_settings_load);
}

void Task::post_event_system_display_settings_loaded(const Event_System_Settings &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_system_display_settings_loaded, _event);
}

void Task::post_event_system_factory_reset()
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_system_factory_reset,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_system_factory_reset);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_system_factory_reset);
}

void Task::post_event_system_factory_reset_done()
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_system_factory_reset_done,
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_system_factory_reset_done);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_system_factory_reset_done);
}

void Task::post_event_cas_switch_folder(bool _is_sky)
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = MSG_CAS_Switch_Folder
    {
        .header = {
            .source = MBGUI_IPC_SOURCE,
            .cmd = ipc_cmd_cas_switch_folder,
        },
        .is_sky = _is_sky,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_switch_folder, _is_sky);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_cas_switch_folder, _is_sky);
}

void Task::post_event_cas_exit()
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_cas_system_need_exit,
    };
    POST_IPC_EVENT_TO_CAS(msg, handle_event_cas_exit);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_cas_exit);

}

void Task::post_event_service_favorite_changed(Service *_srv)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_service_favorite_changed, _srv);
}

void Task::post_event_usb_plug_event(Event_USB_Plug _event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_usb_plug_event, _event);
}

void Task::post_event_toggle_power()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_toggle_power);
}

void Task::post_event_pin_reset()
{
    NOOP;
#if MBGUI_HAS_IPC
    auto msg = IPC_Message
    {
        .source = MBGUI_IPC_SOURCE,
        .cmd = ipc_cmd_reset_pin_code,
    };
    POST_IPC_EVENT_TO_GUI(msg, handle_event_pin_reset);
#endif //MBGUI_HAS_IPC
    POST_EVENT_TO_ALL(handle_event_pin_reset);
}

void Task::post_event_delete_all_services()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_delete_all_services);
}

void Task::post_event_start_recording(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_start_recording, _service_id, _time_to_end);
}

void Task::post_event_send_message_to_start_record(ScheduleEntry _sc_entry)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_send_message_to_start_record, _sc_entry);
}

void Task::post_event_zone_id_changed(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_zone_id_changed, _from_zone_id, _to_zone_id);
}

void Task::post_event_lnbf_config_save(const Event_LNBF_Params &_event)
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_lnbf_config_save, _event);
}

void Task::post_event_fast_install()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_fast_install);
}


void Task::post_event_program_access_denied()
{
    NOOP;
    POST_EVENT_TO_ALL(handle_event_program_access_denied);
}


#ifdef MBGUI_SAT_MONITOR
void Task::post_event_terminal_text(const StaticString &_text)
{
    NOOP;
    POST_EVENT_TO_ALL(post_event_terminal_text, _text);
}
#endif // MBGUI_SAT_MONITOR

void Task::handle_event_application_ready()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_easy_install_save(bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_application_state_save(const Event_Save_Application_State &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_blind_scan_progress(std::weak_ptr<Event_Blind_Scan_Progress>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_autodetect_lnbf_start(std::weak_ptr<Event_Autodetect_LNBf>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_set_default_lnbf()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_set_lnbf_type(LNBF_Type)
{
    mb_assert(false);
    __builtin_unreachable();
}


void Task::handle_event_change_lnbf_type(std::function<void(std::string &result)>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_autodetect_lnbf_finished(bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_application_state_load()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_start_emm_filtering(const Transponder *)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cc(const Event_CC &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_subtitle(const Event_Subtitle_Image &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cc_enable(CC_Type)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_application_state_loaded(const Event_Save_Application_State &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_autodetect_progress(const Event_Transponder_data)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_add_satellite(Satellite)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_update_satellite(Satellite)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_delete_satellite(unsigned int)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_satellite_list_load(std::function<void(std::vector<Satellite> &satellites)>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_update_channel_list(Channel_List_Type, std::vector<Lineup::Channel_Info>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_schedule_load(std::function<void(std::vector<ScheduleEntry> schedule)>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_insert_schedule(ScheduleEntry)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_delete_schedule(int)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_update_schedule(ScheduleEntry)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_ota_update_get(PID_t, std::weak_ptr<Event_OTA_DSI>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_dvb_subtitle_get(PID_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_dvb_subtitle_del(PID_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_clock_need_update()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_clock_set_time(const Event_Time &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_clock_time_set(bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_save_terms_of_use()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_eit_update()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_player_change_audio(PID_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_start(std::string, uint8_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_pause()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_resume()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_next(std::string, uint8_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_previous(std::string, uint8_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_forward(uint16_t, bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_media_player_rewind(uint16_t, bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_pvr_start()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_pvr_playback_start(std::string)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_pvr_timeshift_start(std::string)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_pvr_timeshift_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_pvr_timeshift_play()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_player_restart()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_player_started()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_player_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_player_stopped()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_starting_application()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_process_automatic_search()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_satellite_found()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_build(std::weak_ptr<Event_List_Update>, bool restart)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_update(std::weak_ptr<Event_List_Update_Channel_List>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_ready(const Event_Lineup_Ready &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_save()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_save_zone_id(Zone_ID_t, Segment_ID_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_start()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_changed()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lineup_load()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_display_message(const Event_Display_Message &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_mainmenu_show()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_menu_plus(bool, bool, std::chrono::time_point<std::chrono::system_clock>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_closed_caption()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_audio_lr()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_production_info()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_factory_reset()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_osd_channel_list_show()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_channel_change(Service *)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_channel_changed(Service *)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_channel_preview(Service *)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_display_clear()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_services_update()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_sound_changed(const Event_Sound &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_change_osd_home_state()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_change_osd_media_player_state()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_display_settings_save(const Event_System_Settings &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_display_settings_saved()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_display_settings_load()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_display_settings_loaded(const Event_System_Settings &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_factory_reset()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_factory_reset_done()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_memory_is_low()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_system_need_reset()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_ota_found()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_service_favorite_changed(Service *)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_exit()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_switch_folder(bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_service_pmt_section(PID_t, Service_ID_t, DVB_Table_Section)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_service_pmt_get_next_section(PID_t, Service_ID_t, uint8_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_transponder_lock(const Transponder *, bool /*_force*/)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_transponder_locked(const Event_Tuner_Lock &)
{
    mb_assert(false);
    __builtin_unreachable();
}


void Task::handle_event_usb_plug_event(Event_USB_Plug)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_zone_id_changed(Zone_ID_t, Zone_ID_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_toggle_power()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_pin_reset()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_delete_all_services()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_start_recording(Service_ID_t, std::chrono::time_point<std::chrono::system_clock>)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_send_message_to_start_record(ScheduleEntry)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_fast_install()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_lnbf_config_save(const Event_LNBF_Params &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_fingerprint_get()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_request_descramble(const Event_CAS_Request_Descramble &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_request_descramble_done(bool)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_request_descramble_pmt_update(const Event_CAS_Request_Descramble &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_request_descramble_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_request_descramble_stop_done()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_send_cat_table_section(const Event_CAS_CAT_Table_Section &)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_program_access_denied()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_timeshift_start(Event_PVR_Record_Param)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_timeshift_play()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_timeshift_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_record_start(Event_PVR_Record_Param)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_record_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_record_pause()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_record_resume()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_start(std::string)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_stop()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_pause()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_resume()
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_forward(uint16_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_rewind(uint16_t)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_play_next(std::string)
{
    mb_assert(false);
    __builtin_unreachable();
}

void Task::handle_event_cas_pvr_get_status(const Event_PVR_Status &)
{
    mb_assert(false);
    __builtin_unreachable();
}

#ifdef MBGUI_SAT_MONITOR
void Task::handle_event_terminal_text(const StaticString &)
{
    mb_assert(false);
    __builtin_unreachable();
}
#endif // MBGUI_SAT_MONITOR

#ifdef MBGUI_PERIODIC_DUMP
void Task::handle_event_debbug_dump_status()
{
    mb_assert(false);
    __builtin_unreachable();
}

#endif // MBGUI_PERIODIC_DUMP

}
