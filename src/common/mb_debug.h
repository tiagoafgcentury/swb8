#pragma once

#define TERM_RESET "\033[0m"
#define TERM_RED_BOLD "\033[1;31m"
#define TERM_GREEN_BOLD "\033[1;32m"
#define TERM_YELLOW_BOLD "\033[1;33m"
#define TERM_BLUE_BOLD "\033[1;34m"

#ifndef NDEBUG

#include <iostream>
#include <iomanip>

namespace mb {

namespace log {

    enum LOG_LEVEL
    {
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR
    };

#ifndef MBD_DEFAULT_LOG_LEVEL
    #define MBD_DEFAULT_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef MBD_CAS
    #define MBD_CAS MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_COMMON
    #define MBD_COMMON MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_DEMUX
    #define MBD_DEMUX MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_EVENTS
    #define MBD_EVENTS MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_DVB
    #define MBD_DVB MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_EPG
    #define MBD_EPG MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_HAL
    #define MBD_HAL MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_LINEUP
    #define MBD_LINEUP MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_OSD
    #define MBD_OSD LOG_LEVEL_DEBUG
#endif
#ifndef MBD_PLAYER
    #define MBD_PLAYER MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_TASK
    #define MBD_TASK MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_TUNER
    #define MBD_TUNER MBD_DEFAULT_LOG_LEVEL
#endif
#ifndef MBD_DB
    #define MBD_DB LOG_LEVEL_DEBUG
#endif


} // namespace log

} // namespace mb

constexpr const char* DEBUG_COLOUR(const ::mb::log::LOG_LEVEL LVL)
{
    if (LVL >= ::mb::log::LOG_LEVEL::LOG_LEVEL_ERROR)
        return TERM_RED_BOLD;
    else if (LVL >= ::mb::log::LOG_LEVEL::LOG_LEVEL_WARN)
        return TERM_YELLOW_BOLD;
    else if (LVL >= ::mb::log::LOG_LEVEL::LOG_LEVEL_INFO)
        return TERM_RESET;
    else
        return TERM_GREEN_BOLD;
}

#define DEBUG_MSG(PREFIX, LVL, MSG) do { using namespace std; using namespace mb::log;  \
    if constexpr (::mb::log::LOG_LEVEL_ ## LVL >= MBD_ ## PREFIX) { cout << "[" << # PREFIX << "][" << DEBUG_COLOUR(::mb::log::LOG_LEVEL_ ## LVL) << # LVL << TERM_RESET << "][" << basename((char*)__FILE__) << ":" << dec << __LINE__ << "][" << __FUNCTION__ << "] " << MSG << TERM_RESET; } \
    } while(false)
#define DEBUG_MSG_NL(PREFIX, LVL, MSG) do { using namespace std; using namespace mb::log;  \
    if constexpr (::mb::log::LOG_LEVEL_ ## LVL >= MBD_ ## PREFIX) { cout << MSG; }                 \
    } while(false)
#define HEXBYTE(CHAR) hex << setw(2) << setfill('0') << static_cast<unsigned int>(CHAR)
#define DEBUG(MSG) do { using namespace std; cerr << "[" << basename((char*)__FILE__) << ":" << dec <<  __LINE__ << "] " << MSG; } while(false)

#else

#define DEBUG_MSG(MSG) do { } while(false)
#define DEBUG_MSG_NL(MSG) do { } while(false)
#define DEBUG(MSG) do { } while(false)

#endif
