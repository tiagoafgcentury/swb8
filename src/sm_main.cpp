#include "sm_main.h"
#include "project_version.h"

#include <unistd.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdlib.h>

#include "tasks/mb_task_demux.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "tasks/sm_task_sat_monitor.h"

#include "common/mb_globals.h"
#include "hal/mb_system.h"
#include "hal/mb_watchdog.h"
#include "common/mb_lineup.h"

using namespace std::chrono_literals;

constexpr auto LOOP_TIME = 250ns;

extern const char *GIT_VERSION;

bool g_mbgui_keep_running = true;

int main(int argc [[maybe_unused]], char **argv [[maybe_unused]])
{
    std::cout << "\n\n******************************************\n\n"
              << "\tSAT Monitor\t" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << "." << PROJECT_VERSION_TWEAK << "." << GIT_VERSION
#if __BYTE_ORDER == __LITTLE_ENDIAN
              << " LE"
#else
              << " BE"
#endif
              << "\n\t" << "Chip Family: " << CENTURY_CHIP_FAMILY
              << "\n\t" << PROJECT_COMPILE_TIMESTAMP
              << "\n\n******************************************\n" << std::endl;
    {
        mb::System system(false);
        mb::Watchdog wd;
        mb::Task_Player task_player;
        mb::Task_Demux task_demux;
        mb::Task_Tuner task_tuner;
        mb::Task_Sat_Monitor task_sat_monitor;
        auto start = mb::now();

        while(g_mbgui_keep_running)
        {
            wd.ping();
            mb::Task::run_processes();
        }

        {
            using namespace std::chrono;
            auto elapsed = mb::now() - start;
            auto sec = static_cast<double>(duration_cast<milliseconds>(elapsed).count()) / 1000.0;
            char buf[36];
            snprintf(buf, sizeof(buf), "Lineup finished in %0.1f seconds", sec);
            mb::Task::post_event_terminal_text(buf);
            DEBUG_MSG(buf << "\n");
        }

        // cppcheck-suppress unknownMacro
        auto wget = popen("wget 'http://" MBGUI_POSTGRES_SERVER ":3011/Process' -O /dev/null 2>&1", "r");

        while(true)
        {
            size_t buffer_size = 0;
            char *buffer = nullptr;
            auto sz = getline(&buffer, &buffer_size, wget);

            if(sz <= 0)
            {
                break;
            }

            buffer[sz - 1] = 0;
            mb::Task::post_event_terminal_text(buffer);
            free(buffer);
        }

        pclose(wget);
        {
            char buf[256];
            auto now = time(0);
            auto tstruct = *localtime(&now);
            strftime(buf, sizeof(buf), "Done SAT Monitor %Y-%m-%d %X", &tstruct);
            mb::Task::post_event_terminal_text(buf);
            DEBUG_MSG(buf << "\n");
        }
    }
    std::cout << std::flush << "Exit" << std::endl;
    return EXIT_SUCCESS;
}

// Default flags for Address-Sanitizer
// @see https://github.com/google/sanitizers/wiki/AddressSanitizerFlags#run-time-flags
#ifdef MBGUI_USE_ASAN
const char *__asan_default_options()
{
    return "verbosity=1:malloc_context_size=20:check_initialization_order=true:detect_stack_use_after_return=true:print_stats=true:atexit=true";
}

#endif // MBGUI_USE_ASAN
