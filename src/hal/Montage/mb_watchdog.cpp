#pragma once

#include "../mb_watchdog.h"

#include <chrono>

namespace mb {

Watchdog::Watchdog()
{
    using namespace std::chrono_literals;
    std::chrono::milliseconds watch_dog_timeout = 10s;
    MT_EXEC(mt_unf_wdg_init());
    MT_EXEC(mt_unf_wdg_set_timeout(0, watch_dog_timeout.count()));
}

Watchdog::~Watchdog()
{
    MT_EXEC(mt_unf_wdg_disable(0));
    MT_EXEC(mt_unf_wdg_deinit());
}

void Watchdog::enable()
{
#ifndef NDEBUG

    if(debuggerIsAttached())
    {
        DEBUG_MSG("WATCHDOG IGNORE - GDB is attached!\n");
        return;
    }

#endif
    MT_EXEC(mt_unf_wdg_enable(0));
}

void Watchdog::ping()
{
    mt_unf_wdg_clear(0);
}

} // namespace mb
