#pragma once

#include "aui_mp.h"

#define ALI_EXEC(fn)                                                           \
  do {                                                                         \
    int ret = fn;                                                              \
    if (0 != ret) {                                                            \
      DEBUG_MSG(                                                               \
          HAL, ERROR,                                                          \
          #fn " failed(" << ret << "): " << ::mb::ali_error_msg(ret) << endl); \
    }                                                                          \
  } while (false)

namespace mb {

const char* ali_error_msg(int error_code);

extern aui_hdl g_ali_demux_hnd;

}  // namespace mb
