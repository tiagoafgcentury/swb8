#pragma once

namespace mb {

#if defined(MBGUI_APP_GUI)
#define MBGUI_IPC_SOURCE IPC_Address::ipc_addr_mbgui
#define s_queue_local s_queue_gui
#define MBGUI_SOCKET_PATH_LOCAL MBGUI_SOCKET_PATH_GUI
#elif defined(MBGUI_APP_CAS)
#define MBGUI_IPC_SOURCE IPC_Address::ipc_addr_mbcas
#define s_queue_local s_queue_cas
#define MBGUI_SOCKET_PATH_LOCAL MBGUI_SOCKET_PATH_CAS
#elif defined(MBGUI_APP_TPM)
// Nothing - TPM does not use IPC
#elif defined(MBGUI_APP_SAT_MONITOR)
#else
#error Missing MBGUI_APP_* define to set correct app
#endif

// Only call a function if the class overrides the function,
// and thus do NOT call the function if it is only implemented in
// the base class.
#define POST_EVENT(CLASS, FUNCTION, OBJECT, PARAMS...) \
    static_assert(std::is_same<decltype(* OBJECT), CLASS &>::value, "CLASS does not match OBJECT" );                    \
    if constexpr (!std::is_same_v<decltype(& CLASS :: FUNCTION), decltype(&Task:: FUNCTION)>)                           \
    {                                                                                                                   \
        DEBUG_MSG(TASK, DEBUG, "Post: " << (# FUNCTION) + 13 << " 🠦 " << (# OBJECT) + 2 << "(" << dump_params(PARAMS) << ")\n");    \
        post_event([ = ] { OBJECT -> FUNCTION ( PARAMS ); });                                                           \
    }

#ifndef NDEBUG
// Macro that does nothing and has no side effects, except for allowing a breakpoint
// to be set before a POST_EVENT call, since POST_EVENT triggers on the POST and the HANDLE
#define NOOP do { __asm__("nop\n\t"); } while (false)
#else
#define NOOP do { } while (false)
#endif

#if defined(MBGUI_APP_GUI)
Task_Application *Task::s_task_application { nullptr };
Task_Database *Task::s_task_database { nullptr };
Task_Demux *Task::s_task_demux { nullptr };
Task_EIT_Events *Task::s_task_eit_events { nullptr };
Task_Easy_Install *Task::s_task_easy_install { nullptr };
Task_OSD *Task::s_task_osd { nullptr };
Task_Player *Task::s_task_player { nullptr };
Task_Remote_Control *Task::s_task_remote_control { nullptr };
Task_Tuner *Task::s_task_tuner { nullptr };

#define POST_EVENT_TO_APPLICATION(FUNCTION, PARAMS...) POST_EVENT(Task_Application, FUNCTION, s_task_application, PARAMS)
#define POST_EVENT_TO_DATABASE(FUNCTION, PARAMS...) POST_EVENT(Task_Database, FUNCTION, s_task_database, PARAMS)
#define POST_EVENT_TO_DEMUX(FUNCTION, PARAMS...) POST_EVENT(Task_Demux, FUNCTION, s_task_demux, PARAMS)
#define POST_EVENT_TO_EASY_INSTALL(FUNCTION, PARAMS...) POST_EVENT(Task_Easy_Install, FUNCTION, s_task_easy_install, PARAMS)
#define POST_EVENT_TO_EIT_EVENTS(FUNCTION, PARAMS...) POST_EVENT(Task_EIT_Events, FUNCTION, s_task_eit_events, PARAMS)
#define POST_EVENT_TO_OSD(FUNCTION, PARAMS...) POST_EVENT(Task_OSD, FUNCTION, s_task_osd, PARAMS)
#define POST_EVENT_TO_PLAYER(FUNCTION, PARAMS...) POST_EVENT(Task_Player, FUNCTION, s_task_player, PARAMS)
#define POST_EVENT_TO_REMOTE_CONTROL(FUNCTION, PARAMS...) POST_EVENT(Task_Remote_Control, FUNCTION, s_task_remote_control, PARAMS)
#define POST_EVENT_TO_TUNER(FUNCTION, PARAMS...) POST_EVENT(Task_Tuner, FUNCTION, s_task_tuner, PARAMS)

#define PROCESS_APPLICATION s_task_application->process()
#define PROCESS_DATABASE s_task_database->process()
#define PROCESS_DEMUX s_task_demux->process()
#define PROCESS_EASY_INSTALL s_task_easy_install->process()
#define PROCESS_EIT_EVENTS s_task_eit_events->process()
#define PROCESS_OSD s_task_osd->process()
#define PROCESS_PLAYER s_task_player->process()
#define PROCESS_REMOTE_CONTROL s_task_remote_control->process()
#define PROCESS_TUNER s_task_tuner->process()
#endif // defined(MBGUI_APP_GUI)

#if defined(MBGUI_APP_SAT_MONITOR)
Task_Demux *Task::s_task_demux { nullptr };
Task_OSD *Task::s_task_osd { nullptr };
Task_Remote_Control *Task::s_task_remote_control { nullptr };
Task_Tuner *Task::s_task_tuner { nullptr };

#define POST_EVENT_TO_DEMUX(FUNCTION, PARAMS...) POST_EVENT(Task_Demux, FUNCTION, s_task_demux, PARAMS)
#define POST_EVENT_TO_OSD(FUNCTION, PARAMS...) POST_EVENT(Task_OSD, FUNCTION, s_task_osd, PARAMS)
#define POST_EVENT_TO_REMOTE_CONTROL(FUNCTION, PARAMS...) POST_EVENT(Task_Remote_Control, FUNCTION, s_task_remote_control, PARAMS)
#define POST_EVENT_TO_TUNER(FUNCTION, PARAMS...) POST_EVENT(Task_Tuner, FUNCTION, s_task_tuner, PARAMS)

#define PROCESS_DEMUX s_task_demux->process()
#define PROCESS_OSD s_task_osd->process()
#define PROCESS_REMOTE_CONTROL s_task_remote_control->process()
#define PROCESS_TUNER s_task_tuner->process()
#endif // defined(MBGUI_APP_SAT_MONITOR)

#ifdef MB_USE_MICROHTTPD
Task_HTTP_Server *Task::s_task_http_server;

#define POST_EVENT_TO_HTTP(FUNCTION, PARAMS...) POST_EVENT(Task_HTTP_Server, FUNCTION, s_task_http_server, PARAMS)

#define PROCESS_HTTP s_task_http_server->process()
#endif // MB_USE_MICROHTTPD

#if defined(MBGUI_APP_GUI) or defined(MBGUI_APP_CAS)
Task_CAS *Task::s_task_cas { nullptr };

#define POST_EVENT_TO_CAS(FUNCTION, PARAMS...) POST_EVENT(Task_CAS, FUNCTION, s_task_cas, PARAMS)

#define PROCESS_CAS s_task_cas->process()
#endif // defined(MBGUI_APP_GUI) or defined(MBGUI_APP_CAS)

#ifdef MBGUI_APP_SAT_MONITOR
Task_Sat_Monitor *Task::s_task_sat_monitor { nullptr };

#define POST_EVENT_TO_SAT_MONITOR(FUNCTION, PARAMS...) POST_EVENT(Task_Sat_Monitor, FUNCTION, s_task_sat_monitor, PARAMS)

#define PROCESS_SAT_MONITOR s_task_sat_monitor->process()
#endif

// Fallbacks - when task is not present, declare a macro that just sets an assert/exit the code if called
// This should never happen - post an event to a class that this app does not know.

#ifndef POST_EVENT_TO_APPLICATION
#define POST_EVENT_TO_APPLICATION(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_APPLICATION do { } while (false)
#endif

#ifndef POST_EVENT_TO_DATABASE
#define POST_EVENT_TO_DATABASE(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_DATABASE do { } while (false)
#endif

#ifndef POST_EVENT_TO_DEMUX
#define POST_EVENT_TO_DEMUX(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_DEMUX do { } while (false)
#endif

#ifndef POST_EVENT_TO_EASY_INSTALL
#define POST_EVENT_TO_EASY_INSTALL(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_EASY_INSTALL do { } while (false)
#endif

#ifndef POST_EVENT_TO_HTTP
#define POST_EVENT_TO_HTTP(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_HTTP do { } while (false)
#endif

#ifndef POST_EVENT_TO_CAS
#define POST_EVENT_TO_CAS(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_CAS do { } while (false)
#endif

#ifndef POST_EVENT_TO_REMOTE_CONTROL
#define POST_EVENT_TO_REMOTE_CONTROL(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_REMOTE_CONTROL do { } while (false)
#endif

#ifndef POST_EVENT_TO_OSD
#define POST_EVENT_TO_OSD(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_OSD do { } while (false)
#endif

#ifndef POST_EVENT_TO_EIT_EVENTS
#define POST_EVENT_TO_EIT_EVENTS(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_EIT_EVENTS do { } while (false)
#endif

#ifndef POST_EVENT_TO_SAT_MONITOR
#define POST_EVENT_TO_SAT_MONITOR(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_SAT_MONITOR do { } while (false)
#endif

#ifndef POST_EVENT_TO_PLAYER
#define POST_EVENT_TO_PLAYER(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_PLAYER do { } while (false)
#endif

#ifndef POST_EVENT_TO_TUNER
#define POST_EVENT_TO_TUNER(FUNCTION, PARAMS...) do { } while (false)
#define PROCESS_TUNER do { } while (false)
#endif

// Macro to ensure that ALL tasks receive ALL events
#define POST_EVENT_TO_ALL(FUNCTION, PARAMS...) \
    POST_EVENT_TO_APPLICATION(FUNCTION, PARAMS); \
    POST_EVENT_TO_CAS(FUNCTION, PARAMS); \
    POST_EVENT_TO_DATABASE(FUNCTION, PARAMS); \
    POST_EVENT_TO_DEMUX(FUNCTION, PARAMS); \
    POST_EVENT_TO_EIT_EVENTS(FUNCTION, PARAMS); \
    POST_EVENT_TO_HTTP(FUNCTION, PARAMS); \
    POST_EVENT_TO_OSD(FUNCTION, PARAMS); \
    POST_EVENT_TO_PLAYER(FUNCTION, PARAMS); \
    POST_EVENT_TO_REMOTE_CONTROL(FUNCTION, PARAMS); \
    POST_EVENT_TO_SAT_MONITOR(FUNCTION, PARAMS); \
    POST_EVENT_TO_TUNER(FUNCTION, PARAMS);

#if not defined(MBGUI_APP_GUI)
#define POST_IPC_EVENT_TO_GUI(MSG, FUNCTION, PARAMS...) \
    do {                                                                                                \
        DEBUG_MSG(TASK, DEBUG, "Post: " << (# FUNCTION) + 13 << " 🠦 {IPC GUI}(" << dump_params(PARAMS) << ")\n");    \
        send_ipc_message(s_queue_gui, reinterpret_cast<uint8_t *>(&MSG), sizeof(MSG));                  \
    } while (false)
#else
#define POST_IPC_EVENT_TO_GUI(MSG, FUNCTION, PARAMS...) \
    (void) MSG; \
    POST_EVENT_TO_ALL(FUNCTION, PARAMS);
#endif

#if not defined(MBGUI_APP_CAS)
#define POST_IPC_EVENT_TO_CAS(MSG, FUNCTION, PARAMS...) \
    do {                                                                                                \
        DEBUG_MSG(TASK, DEBUG, "Post: " << (# FUNCTION) + 13 << " 🠦 {IPC CAS}(" << dump_params(PARAMS) << ")\n");    \
        send_ipc_message(s_queue_cas, reinterpret_cast<uint8_t *>(&MSG), sizeof(MSG));                  \
    } while (false)
#else
#define POST_IPC_EVENT_TO_CAS(MSG, FUNCTION, PARAMS...) \
    (void) MSG; \
    POST_EVENT_TO_ALL(FUNCTION, PARAMS);
#endif

#ifdef MBGUI_PERIODIC_DUMP
    #define PROFILE_PREPARE_TASK(TASK)  \
        int __elapsed ## TASK = 0;

    #define PROFILE_TASK(TASK) \
        do {                                                                                                                    \
            typedef std::chrono::high_resolution_clock clock;                                                                   \
            auto __start = clock::now();                                                                                        \
            TASK;                                                                                                               \
            __elapsed ## TASK += std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - __start ).count();        \
        } while (false)

    #define PROFILE_DUMP_TASK(TASK_NAME) \
        do {                                                                                                                \
            DEBUG_MSG(TASK, DEBUG, setw(25) << setfill(' ') << right << # TASK_NAME " " << setw(0) << left <<  __elapsed ## TASK_NAME << "\n");    \
            __elapsed ## TASK_NAME = 0;                                                                                          \
        } while (false)
#else
    #define PROFILE_PREPARE_TASK(TASK)

    #define PROFILE_TASK(TASK) TASK
#endif

} // namespace mb
