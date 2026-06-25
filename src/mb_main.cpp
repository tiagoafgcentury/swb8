#include "mb_main.h"
#include "project_version.h"

#include <unistd.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "tasks/mb_task_application.h"
#include "tasks/mb_task_cas.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task_demux.h"
#include "tasks/mb_task_osd.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_remote_control.h"
#include "tasks/mb_task_tuner.h"
#include "fw_env.h"

#ifdef MB_USE_MICROHTTPD
#include "tpm/mb_task_http_server.h"
#endif

#include "common/mb_globals.h"
#include "common/mb_state_file.h"
#include "common/mb_version.h"
#include "common/static_string.h"

#include "hal/mb_system.h"
#include "hal/mb_watchdog.h"

#include <signal.h>
#include <sys/stat.h>
#include <fstream>
#include <filesystem>

#include <linux/major.h>

#include <experimental/filesystem>

//#include "/opt/ali/buildroot-10.2.1.29/output/build/jemalloc-5.2.1/include/jemalloc/jemalloc.h"

using namespace std::chrono_literals;

constexpr auto LOOP_TIME = 150ns;

extern const char *GIT_VERSION;

std::atomic<bool> g_mbgui_keep_running = true;
std::atomic<bool> g_mbgui_reboot_after_exit = false;
std::atomic<bool> g_mbgui_restart_on_exit = true;
std::atomic<bool> g_mbgui_do_factory_reset = false;
std::atomic<bool> g_mbgui_pause_low_priority_tasks = false;

namespace {

std::atomic<bool> s_mbgui_usb_plug_event = false;

void sighandler(int _signal)
{
    switch(_signal)
    {
        case SIGUSR1:
        {
            s_mbgui_usb_plug_event.store(true, std::memory_order::release);
            DEBUG_MSG(COMMON, INFO, "Signal: USB\n");
            break;
        }

        case SIGTERM:
            g_mbgui_keep_running.store(false, std::memory_order_release);
            g_mbgui_restart_on_exit.store(false, std::memory_order_release);
            g_mbgui_reboot_after_exit.store(false, std::memory_order_release);
            break;

        default:
            DEBUG_MSG(COMMON, ERROR, "Signal not handled\n");
            break;
    }
}

void erase_all_files()
{
    DEBUG_MSG(COMMON, INFO, "Erase all files\n");

    namespace fs = std::filesystem;
    std::error_code ec;

    auto directory = fs::path(MBGUI_CACHE_PATH);
    for (const auto& entry : fs::directory_iterator(directory, {fs::directory_options::skip_permission_denied}, ec))
    {
        if (entry.is_regular_file())
        {
            if (fs::remove(entry.path(), ec))
            {
                DEBUG_MSG(COMMON, INFO, "Removed: " << entry.path() << "\n");
            }
            else
            {
                DEBUG_MSG(COMMON, ERROR, "Failed to remove: " << entry.path() << "\t" << ec.message() << "\n");
            }
        }
    }
}

std::vector<mb::Static_String> s_usb_devices;

#define DUMP_STRVEC(vec) \
    for (const auto& str : vec) DEBUG_MSG(COMMON, DEBUG, str << "\n")

std::vector<mb::Static_String> get_usb_devices()
{
    std::ifstream stat("/proc/diskstats");

    if(not stat.is_open())
    {
        mb_assert(false);
        DEBUG_MSG(COMMON, ERROR, "Failed to open /proc/diskstats\n");
        return {};
    }

    std::vector<mb::Static_String> result;
    std::string line;

    while(std::getline(stat, line))
    {
        std::istringstream iss(line);
        int major, minor;
        mb::Static_String device;

        if(iss >> major and major == SCSI_DISK0_MAJOR)
        {
            iss >> minor >> device;
            result.push_back(std::move(device));
        }
    }

RESTART_FOR:

    // if we have: sda and sda1, remove sda from the list
    for(auto it = result.begin(); it != result.end() and result.size() > 1; it++)
    {
        auto next = it;
        next++;
        auto found = std::find_if(next, result.end(), [&](const auto & _dev)
        {
            return strncmp(it->data(), _dev.data(), it->size()) == 0;
        });

        if(found != result.end())
        {
            DEBUG_MSG(COMMON, DEBUG, "Erase: " << *it << "\n");
            result.erase(it);
            goto RESTART_FOR;
        }
    }

    DUMP_STRVEC(result);
    return result;
}

struct USB_Device_Info
{
    mb::Event_USB_Plug::Type type;
    mb::Static_String dev;
};

std::vector<USB_Device_Info> check_usb_devices()
{
    using namespace mb;
    auto devices = get_usb_devices();
    std::vector<USB_Device_Info> result;

    for(auto device = devices.begin(); device != devices.end(); device++)
    {
        auto found = std::find_if(s_usb_devices.begin(), s_usb_devices.end(), [&device](const auto & _d)
        {
            return strcmp(device->data(), _d.data()) == 0;
        });

        if(found == s_usb_devices.end())
        {
            s_usb_devices.push_back(*device);
            Task::post_event_usb_plug_event({ .type = Event_USB_Plug::Plugged_In });
        }
    }

RESTART_FOR:

    for(auto device = s_usb_devices.begin(); device != s_usb_devices.end(); device++)
    {
        auto found = std::find_if(devices.begin(), devices.end(), [&device](const auto & _d)
        {
            return strcmp(device->data(), _d.data()) == 0;
        });

        if(found == devices.end())
        {
            s_usb_devices.erase(device);
            Task::post_event_usb_plug_event({ .type = Event_USB_Plug::Removed });
            goto RESTART_FOR;
        }
    }

    return result;
}

void populate_usb_devices()
{
    auto devices = get_usb_devices();
    s_usb_devices.reserve(devices.size());

    for(auto device = devices.begin(); device != devices.end(); device++)
    {
        s_usb_devices.push_back(std::move(*device));
    }
}

namespace fs = std::filesystem;
void remove_file_if_exists(const fs::path& path)
{
    if (fs::exists(path))
    {
        fs::remove(path);
    }
}

}

int main(int argc [[maybe_unused]], char **argv [[maybe_unused]])
{
    auto version = MB_OSD_Version::get_full_version();
    auto git_version = std::string(GIT_VERSION);
    std::cout << "\n\n******************************************\n\n"
              << "\t" << PROJECT_NAME << "\t" << version << "." << git_version
#if __BYTE_ORDER == __LITTLE_ENDIAN
              << " LE"
#else
              << " BE"
#endif
              << "\n\t" << "Chip Family: " << CENTURY_CHIP_FAMILY
              << "\n\t" << PROJECT_COMPILE_TIMESTAMP << " " << PROJECT_BUILD_TYPE
              << "\n\tUser: " << getuid()
              << "\n\n******************************************\n" << std::endl;
#ifndef NDEBUG
    std::cout << "Args: ";

    for(int i = 0; i < argc; i++)
    {
        std::cout << argv[i] << " ";
    }

    std::cout << "\n";
#endif
    /*
    **  Read environment variablesg_mbgui_do_factory_reset
    */
#define software_version_env "software_version"
    {
        fw_env_open();
        auto version_env = fw_getenv(software_version_env);
        DEBUG_MSG(COMMON, DEBUG, "version_env: " << version_env << "\n");

        if(version_env == nullptr or strlen(version_env) == 0)
        {
            DEBUG_MSG(COMMON, DEBUG, "Software version not set\n");
            fw_env_write(software_version_env, PROJECT_VERSION_HEX);
        }
        else if(strncasecmp(version_env, PROJECT_VERSION_HEX, 8) != 0)
        {
            DEBUG_MSG(COMMON, WARN, "Software version mismatch\n");
            fw_env_write(software_version_env, PROJECT_VERSION_HEX);
        }

        fw_env_close();
    }

    remove_file_if_exists("/tmp/NO_REBOOT.txt");

    umask(MBGUI_NAGRA_UMASK);
    signal(SIGTERM, sighandler);
    signal(SIGUSR1, sighandler);
    {
        bool watchdog_enabled { true };
        {
            opterr = 1;
            int c;

            while((c = getopt(argc, argv, "wfh:")) != -1)
            {
                switch(c)
                {
                    case 'w':
                        watchdog_enabled = false;
                        break;
                }
            }
        }
        DEBUG_MSG(COMMON, DEBUG, "Production Final Test: " << (mb::g_production_final_test ? "ON" : "OFF") << "\n");
        DEBUG_MSG(COMMON, DEBUG, "Watchdog enabled: " << (watchdog_enabled ? "ON" : "OFF") << "\n");
        std::filesystem::create_directory(MBGUI_STORAGE_PATH);
        std::filesystem::create_directory(MBGUI_CACHE_PATH);

        {
            auto fp = fopen(MBGUI_CONFIG_FILE, "rb");

            if(fp)
            {
                uint8_t disk_file_version = 0;
                auto disk_file_version_read = fread(&disk_file_version, sizeof(disk_file_version), 1, fp);
                fclose(fp);

                if(disk_file_version_read == 1 and disk_file_version < mb::State_File::App_State_File().file_version)
                {
                    DEBUG_MSG(COMMON, WARN, "Config file version " << static_cast<int>(disk_file_version) << " is below " << static_cast<int>(mb::State_File::App_State_File().file_version) << ". Removing files.\n");
                    erase_all_files();
                    mb::State_File::App_State_File file;
                    file.stand_by = false;
                    file.write();
                }
            }
        }

        populate_usb_devices();
        mb::System system(mb::g_production_final_test);
        mb::Watchdog wd;
        mb::Task_Player task_player;
        mb::Task_Demux task_demux;
        mb::Task_Tuner task_tuner;
        mb::Task_Remote_Control task_remote_control;
        mb::Task_Database task_database;
        mb::Task_Application task_application;
        mb::Task_OSD task_osd;
        mb::Task_EIT_Events task_eit_events;
        mb::Task_CAS task_cas;

        if(watchdog_enabled)
        {
            wd.enable();
        }

#ifdef MB_USE_MICROHTTPD
        mb::Task_HTTP_Server task_http_server;
#endif
#ifdef PROFILING_MAIN_LOOP
        auto i { 0 };
        auto profile_start { std::chrono::high_resolution_clock::now() };
        auto interval { 300 };
#endif

        while(g_mbgui_keep_running.load(std::memory_order_relaxed))
        {
            wd.ping();
            mb::Task::run_processes();
#ifdef PROFILING_MAIN_LOOP

            if(i++ % interval == 0)
            {
                auto end = std::chrono::high_resolution_clock::now();
                double count = std::chrono::duration_cast<std::chrono::milliseconds>(end - profile_start).count();
                count /= static_cast<double>(interval);
                DEBUG_MSG("Loop: " <<  count << "ms\n");
                profile_start = std::chrono::high_resolution_clock::now();
            }

#endif

            if(not g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed))
            {
                std::this_thread::sleep_for(LOOP_TIME);
            }

            {
                bool expected_usb_state = true;
                s_mbgui_usb_plug_event.compare_exchange_weak(expected_usb_state, false, std::memory_order::release);

                if(expected_usb_state)
                {
                    s_mbgui_usb_plug_event.store(false, std::memory_order::release);
                    auto devices = check_usb_devices();

                    for(const auto &device : devices)
                    {
                        mb::Task::post_event_usb_plug_event({ .type = device.type });
                    }
                }
            }
        }

        if(watchdog_enabled)
        {
            wd.disable();
        }
    }

    if(g_mbgui_do_factory_reset.load(std::memory_order_acquire))
    {
        erase_all_files();
        mb::State_File::App_State_File file;
        file.stand_by = false;
        file.write();
    }

    if(not g_mbgui_reboot_after_exit.load(std::memory_order_acquire))
    {
        /* NÂO PERMITE O CONTAINER REINICIAR O STB */
        auto fp = fopen("/tmp/NO_REBOOT.txt", "wb");

        if(fp)
        {
            uint8_t um = 1;
            [[maybe_unused]] auto ret = fwrite(&um, sizeof(uint8_t), 1, fp);
            DEBUG_MSG(COMMON, DEBUG, "fwrite() failed: " << ret << "\n");
        }

        fclose(fp);
    }

    if(g_mbgui_restart_on_exit.load(std::memory_order_acquire))
    {
        DEBUG_MSG(COMMON, DEBUG, "execve restarting app: \n");
        execve("/proc/self/exe", argv, nullptr);
        return EXIT_FAILURE;
    }

    std::cout << "Exit" << std::endl;
#ifdef JEMALLOC_H_
    const char *fileName = "heap_info.out";
    mallctl("prof.dump", NULL, NULL, &fileName, sizeof(const char *));
    std::cout << "Dump: " << fileName << std::endl;
#endif
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
