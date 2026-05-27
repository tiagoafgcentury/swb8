#include "mb_task_mqueue_helpers.h"

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <filesystem>
#include <thread>
#include <chrono>

#include "common/mb_globals.h"

using namespace std::chrono_literals;

namespace mb {

#if defined(MBGUI_APP_GUI)
mqd_t create_queue(const char *_path, int _mode)
{
    struct mq_attr attr
    {
        .mq_flags = 0,                          /* Flags (ignored for mq_open()) */
        .mq_maxmsg = MBGUI_SOCKET_MAX_MESSAGES, /* Max. # of messages on queue */
        .mq_msgsize = MBGUI_SOCKET_MSGSIZE,     /* Max. message size (bytes) */
        .mq_curmsgs = {},                       /* # of messages currently in queue
        (ignored for mq_open()) */
                      .__pad = {},
    };
    return mq_open(_path, _mode | O_CREAT | O_NONBLOCK, 0660, &attr);
}

#else
mqd_t open_queue(const char *_path, int _mode)
{
    auto full_path = std::string("/dev/mqueue") + _path;

    while(true)
    {
        if(std::filesystem::exists(full_path))
        {
            break;
        }

        std::this_thread::sleep_for(50ns);
    }

    return mq_open(_path, _mode | O_NONBLOCK);
}

#endif

mqd_t open_remote_queue(const char *_path)
{
#if defined(MBGUI_APP_GUI)
    auto result = create_queue(_path, O_WRONLY);
#else
    auto result = open_queue(_path, O_WRONLY);
#endif

    if(result == -1)
    {
        DEBUG_MSG(TASK, ERROR, "Open Remote Message Queue " << _path << " failed(" << errno << "): " << strerror(errno) << endl);
        mb_assert(result == -1);
    }
    else
    {
        DEBUG_MSG(TASK, DEBUG, "Opened Remote Message Queue " << _path << endl);
    }

    return result;
}

mqd_t open_local_queue(const char *_path)
{
#if defined(MBGUI_APP_GUI)
    auto result = create_queue(_path, O_RDONLY);
#else
    auto result = open_queue(_path, O_RDONLY);
#endif

    if(result == -1)
    {
        DEBUG_MSG(TASK, ERROR, "Open Local Message Queue " << _path << " failed(" << errno << "): " << strerror(errno) << endl);
        mb_assert(result == -1);
    }
    else
    {
        DEBUG_MSG(TASK, DEBUG, "Opened Local Message Queue " << _path << endl);
    }

    return result;
}

} // namespace mb
