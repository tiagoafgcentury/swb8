#include "mb_cas_main.h"
#include "mb_nagra.h"
#include "project_version.h"

#include <common/mb_globals.h>
#include <common/mb_hash.h>
#include <tasks/mb_task.h>
#include <tasks/mb_task_cas.h>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <signal.h>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <fstream>

#include <linux/prctl.h>  /* Definition of PR_* constants */
#include <sys/prctl.h>

extern "C" {
#include "dpt/ca_dpt.h"
}

#ifndef NDEBUG
#include <filesystem>
#endif

#ifdef MBCAS_NAGRA_IRD_TEST
// Ird Test
#include "mb_ird_command.h"
#endif // MBCAS_NAGRA_IRD_TEST

std::atomic<bool> g_mbcas_keep_running = true;
std::atomic<bool> g_mbcas_container_restart = false;

using namespace std::chrono_literals;

constexpr auto LOOP_TIME = 1ms;

extern const char *GIT_VERSION;

namespace {

void sighandler(int _signal)
{
    switch(_signal)
    {
        case SIGTERM:
            g_mbcas_keep_running.store(false, std::memory_order_release);
            g_mbcas_container_restart.store(false, std::memory_order_release);
            break;

        default:
            DEBUG_MSG(CAS, INFO, "Signal not handled\n");
            break;
    }
}

[[maybe_unused]] void get_clear_pk_data(bool _need_pk, bool _need_csc)
{
    auto boot_total_area = open("/dev/mtd0", O_RDONLY);

    if(boot_total_area <= 0)
    {
        std::cerr << "Error reading boot_total_area/mtd0: " << strerror(errno) << "\n";
        return;
    }

    TUnsignedInt8 encrypted_pk_data[NAGRA_MAX_PK_SIZE];
    TUnsignedInt8 csc_data[NAGRA_MAX_CSCD_SIZE];
    MB_ZERO(encrypted_pk_data);
    MB_ZERO(csc_data);
    // READ PK
    {
        if(lseek(boot_total_area, NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA, SEEK_SET) != NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA)
        {
            std::cerr << "Unable to seek to PK_START (" << NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA << "): " << strerror(errno) << "\n";
            goto ERROR_EXIT_0;
        }

        auto read_ptr = &encrypted_pk_data[0];
        auto remain = NAGRA_MAX_PK_SIZE;

        while(remain > 0)
        {
            auto size = read(boot_total_area, read_ptr, remain);

            if(size < 0)
            {
                std::cerr << "Unable to read PK data: " << strerror(errno) << "\n";
                goto ERROR_EXIT_0;
            }

            remain -= size;
            read_ptr += size;
        }
    }
    // READ CSC
    {
        if(lseek(boot_total_area, NAGRA_CSCD_START_ADDR_IN_BOOT_TOTAL_AREA, SEEK_SET) != NAGRA_CSCD_START_ADDR_IN_BOOT_TOTAL_AREA)
        {
            std::cerr << "Unable to seek to CSCD_START (" << NAGRA_CSCD_START_ADDR_IN_BOOT_TOTAL_AREA << "): " << strerror(errno) << "\n";
            goto ERROR_EXIT_0;
        }

        auto read_ptr = &csc_data[0];
        auto remain = NAGRA_MAX_CSCD_SIZE;

        while(remain > 0)
        {
            auto size = read(boot_total_area, read_ptr, remain);

            if(size < 0)
            {
                std::cerr << "Unable to read CSC data: " << strerror(errno) << "\n";
                goto ERROR_EXIT_0;
            }

            remain -= size;
            read_ptr += size;
        }
    }
    close(boot_total_area);
    boot_total_area = 0;

    if(_need_pk)
    {
        unlink(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE);
        auto encrypted_pk_length = std::min<TUnsignedInt32>(dptArrayToUnsignedIntN(encrypted_pk_data, 4), NAGRA_MAX_PK_SIZE);
        auto pk = creat(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE, S_IRUSR);

        if(pk <= 0)
        {
            std::cerr << "Error creating PK: " << strerror(errno) << "\n";
            return;
        }

        auto write_ptr = &encrypted_pk_data[4];
        auto remain = encrypted_pk_length;

        while(remain > 0)
        {
            auto size = write(pk, write_ptr, remain);

            if(size < 0)
            {
                std::cerr << "Unable to write PK data: " << strerror(errno) << "\n";
                close(pk);
                unlink(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE);
                break;
            }

            remain -= size;
            write_ptr += size;
        }

        if(remain == 0)
        {
            mb::create_file_hash(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE);
        }
        else
        {
            DEBUG_MSG(CAS, WARN, "Remain: " << remain << "\n");
        }

        close(pk);
        pk = 0;
    }

    if(_need_csc)
    {
        unlink(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_CSC_FILE);
        auto csc_length = std::min<TUnsignedInt32>(dptArrayToUnsignedIntN(csc_data, 4), NAGRA_MAX_CSCD_SIZE);
        auto csc = creat(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_CSC_FILE, S_IRUSR);

        if(csc <= 0)
        {
            std::cerr << "Error creating CSC: " << strerror(errno) << "\n";
            return;
        }

        auto write_ptr = &csc_data[0];
        auto remain = csc_length;

        while(remain > 0)
        {
            auto size = write(csc, write_ptr, remain);

            if(size < 0)
            {
                std::cerr << "Unable to write CSC data: " << strerror(errno) << "\n";
                unlink(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_CSC_FILE);
                break;
            }

            remain -= size;
            write_ptr += size;
        }

        if(remain == 0)
        {
            mb::create_file_hash(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_CSC_FILE);
        }
        else
        {
            DEBUG_MSG(CAS, WARN, "Remain: " << remain << "\n");
        }

        close(csc);
    }

    /* Decrypt the PK, not needed
    // Scope for encrypted_pk_length
    {
        auto encrypted_pk_length = dptArrayToUnsignedIntN(encrypted_pk_data, 4);

        if (dptCertDecryptData(csc_data, 0x4026, encrypted_pk_data, decrypted_pk_data, encrypted_pk_length) == 0)
        {
            auto clear_pk = creat(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE, S_IRUSR);

            if (clear_pk <= 0)
            {
                std::cerr << "Error creating PK: " << strerror(errno) << "\n";
                return;
            }

            auto write_ptr = &decrypted_pk_data[0];
            auto remain = encrypted_pk_length;

            while (remain > 0)
            {
                auto size = write(clear_pk, write_ptr, remain);
                if (size < 0)
                {
                    std::cerr << "Unable to write PK data: " << strerror(errno) << "\n";
                    close(clear_pk);
                    unlink(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE);
                    break;
                }
                remain -= size;
                write_ptr += size;
            }

            close(clear_pk);
            clear_pk = 0;
        }
        else
        {
            std::cerr << "Error decrypting PK\n";
        }
    }
    */
ERROR_EXIT_0:

    if(boot_total_area)
    {
        close(boot_total_area);
    }
}

void set_process_priority()
{
    [[maybe_unused]] auto _ = nice(10);
}

} // namespace <empty>

int main(int, char **)
{
#ifndef NDEBUG
    prctl(PR_SET_PTRACER, 0);
#endif
    std::cout << "\n\n******************************************\n\n"
              << "\tMBCAS\t" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << "." << PROJECT_VERSION_TWEAK << "." << GIT_VERSION
#if __BYTE_ORDER == __LITTLE_ENDIAN
              << " LE"
#else
              << " BE"
#endif
              << "\n\t" << "Chip Family: " << CENTURY_CHIP_FAMILY
              << "\n\t" << PROJECT_COMPILE_TIMESTAMP << " " << PROJECT_BUILD_TYPE
              << "\n\n******************************************\n" << std::endl;
    umask(MBGUI_NAGRA_UMASK);
    signal(SIGTERM, sighandler);
    DEBUG_MSG(CAS, INFO, "NAGRA PERSO_DATA PATH: " MBGUI_NAGRA_PERSO_DATA_PATH "\n"
              "NAGRA STORAGE PATH: "  MBGUI_NAGRA_STORAGE_PATH "\n"
              "NAGRA TRUSTED STORAGE PATH: " MBGUI_NAGRA_TRUSTED_STORAGE_PATH "\n");
    std::error_code ec;
    std::filesystem::create_directories(MBGUI_NAGRA_PERSO_DATA_PATH, ec);

    if(ec)
    {
        std::cerr << "Error creating: " MBGUI_NAGRA_PERSO_DATA_PATH << ec.message() << "\n";
    }

    std::filesystem::create_directories(MBGUI_NAGRA_STORAGE_PATH, ec);

    if(ec)
    {
        std::cerr << "Error creating: " MBGUI_NAGRA_STORAGE_PATH << ec.message() << "\n";
    }

#ifndef MBGUI_NO_PK_CSC_CHECK
    auto need_pk = not mb::check_file_hash(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_PK_FILE);
    auto need_csc = not mb::check_file_hash(MBGUI_NAGRA_PERSO_DATA_PATH MBGUI_NAGRA_CSC_FILE);
    DEBUG_MSG(CAS, INFO, "PK Hash: " << (need_pk ? "FAILED" : "OK") << "\n");
    DEBUG_MSG(CAS, INFO, "CSC Hash: " << (need_csc ? "FAILED" : "OK") << "\n");

    if(need_pk or need_csc)
    {
        DEBUG_MSG(CAS, INFO, "Get clear PK\n");
        get_clear_pk_data(need_pk, need_csc);
    }

#endif
    DEBUG_MSG(CAS, INFO, "Set process priority\n");
    set_process_priority();
    DEBUG_MSG(CAS, INFO, "Start Task CAS\n");
    mb::Task_CAS task_cas;
#ifdef MBCAS_NAGRA_IRD_TEST
#define RUN_IRD_TEST(SIZE, ARRAY...)                        \
    do {                                                    \
        auto data = std::array<uint8_t, SIZE>{ ARRAY };     \
        mb::process_ird_command(data.data(), data.size());  \
    } while (false)
    RUN_IRD_TEST(15, 0x64, 0x0D, 0x00, 0x00, 0x01, 0x24, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x25); // ZONE ID
    RUN_IRD_TEST(15, 0x64, 0x0D, 0x00, 0x00, 0x01, 0x24, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x25); // ZONE ID
    RUN_IRD_TEST(15, 0x64, 0x0D, 0x00, 0x00, 0x01, 0x24, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x25); // ZONE ID
    RUN_IRD_TEST(9, 0x64, 0x07, 0x00, 0x00, 0x00, 0x07, 0x12, 0x01, 0xED); // reset_pin_code
    RUN_IRD_TEST(14, 0x64, 0x0C, 0x00, 0x00, 0x00, 0x07, 0xC8, 0x01, 0x04, 0x31, 0x32, 0x33, 0x34, 0x69); // set_pin_code
    RUN_IRD_TEST(9, 0x64, 0x07, 0x00, 0x00, 0x16, 0x30, 0xC2, 0x01, 0x3D); // force identification 01
    RUN_IRD_TEST(15, 0x64, 0x0D, 0x00, 0x00, 0x16, 0x30, 0xC2, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x42); // force_identification 02
    RUN_IRD_TEST(9, 0x64, 0x07, 0x00, 0x00, 0x16, 0x30, 0xC9, 0x01, 0x36); // force_stand_by 01
    RUN_IRD_TEST(15, 0x64, 0x07, 0x00, 0x00, 0x16, 0x30, 0xC9, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x3B); // force_stand_by 02
#endif //MBCAS_NAGRA_IRD_TEST
    DEBUG_MSG(CAS, INFO, "Start Main loop\n");

    while(g_mbcas_keep_running.load(std::memory_order_relaxed))
    {
        mb::Task::run_processes();
        std::this_thread::sleep_for(LOOP_TIME);
    }

#if 0

    if(g_mbcas_container_restart)
    {
        DEBUG("Restarting container MBCAS. \n");
        std::ofstream file("/tmp/mbcas_status.txt");

        if(file.is_open())
        {
            file << "1";
            file.close();
        }
    }

#endif
    return EXIT_SUCCESS;
}
