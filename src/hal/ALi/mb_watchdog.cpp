#include "../mb_watchdog.h"
#include "common/mb_globals.h"

#include <aui_dog.h>
#include "mb_ali_globals.h"

#include <chrono>
#include <thread>

namespace {

aui_hdl s_hdl_dog { nullptr };

}

namespace mb {

using namespace std::chrono_literals;
constexpr std::chrono::microseconds watch_dog_timeout = 10s;

struct Watchdog::Data
{
};

Watchdog::Watchdog()
{
#ifndef NDEBUG

    if(debuggerIsAttached())
    {
        DEBUG_MSG(HAL, INFO, "WATCHDOG IGNORE - GDB is attached!\n");
        return;
    }

    if(getenv("MBGUI_DISABLE_WATCHDOG"))
    {
        DEBUG_MSG(HAL, INFO, "WATCHDOG DISABLE - ENV Flag is set!\n");
        return;
    }

#endif
    ALI_EXEC(aui_dog_init(nullptr, nullptr));
    aui_attr_dog attr_dog;
    MB_ZERO(attr_dog);
    attr_dog.uc_dev_idx = 0;
    attr_dog.ul_work_mode = AUI_DOG_MODE_WATCHDOG;
    attr_dog.ul_time_us = watch_dog_timeout.count();
    ALI_EXEC(aui_dog_open(&attr_dog, &s_hdl_dog));
}

Watchdog::~Watchdog()
{
#ifndef NDEBUG

    if(!s_hdl_dog)
    {
        DEBUG_MSG(HAL, INFO, "WATCHDOG IGNORE - GDB is attached!\n");
        return;
    }

#endif
    ALI_EXEC(aui_dog_close(s_hdl_dog));
    ALI_EXEC(aui_dog_de_init(nullptr, nullptr));
    s_hdl_dog = nullptr;
}

void Watchdog::enable()
{
#ifndef NDEBUG

    if(!s_hdl_dog)
    {
        DEBUG_MSG(HAL, INFO, "WATCHDOG IGNORE - GDB is attached!\n");
        return;
    }

#endif
    ALI_EXEC(aui_dog_start(s_hdl_dog, nullptr));
    ALI_EXEC(aui_dog_time_set(s_hdl_dog, watch_dog_timeout.count()));
}

void Watchdog::disable()
{
#ifndef NDEBUG

    if(!s_hdl_dog)
    {
        DEBUG_MSG(HAL, INFO, "WATCHDOG IGNORE - GDB is attached!\n");
        return;
    }

#endif
    ALI_EXEC(aui_dog_stop(s_hdl_dog, nullptr));
}

void Watchdog::ping()
{
#ifndef NDEBUG

    if(!s_hdl_dog)
    {
        return;
    }

#endif
    ALI_EXEC(aui_dog_time_set(s_hdl_dog, 0));
}

void Watchdog::system_reset()
{
    DEBUG_MSG(HAL, INFO, "WATCHDOG: System reset\n");
    ALI_EXEC(aui_dog_time_set(s_hdl_dog, 1000000));

    while(true)
    {
        std::this_thread::sleep_for(watch_dog_timeout);
    }
}

} // namespace mb
