#pragma once

#include <mqueue.h>

namespace mb {

#if defined(MBGUI_APP_GUI)
mqd_t create_queue(const char *_path, int _mode);
#else
mqd_t open_queue(const char *_path, int _mode);
#endif

mqd_t open_remote_queue(const char *_path);
mqd_t open_local_queue(const char *_path);

} // namespace mb
