#include "mb_dfb_terminal_task.h"

#include "common/mb_globals.h"
#include "hal/mb_tuner.h"

#include <lvgl.h>

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std::chrono_literals;

namespace mb {

auto givemetime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

DFB_Terminal_Task::DFB_Terminal_Task()
{
    //lv_init();
}

DFB_Terminal_Task::~DFB_Terminal_Task()
{
}

void DFB_Terminal_Task::clear()
{
}

void DFB_Terminal_Task::print_message(const std::string &str)
{
    m_message_queue.push(str);
    m_has_update = true;

    if ((std::chrono::system_clock::now() - m_last_clock_update) > 100ms)
    {
        process();
    }
}

void DFB_Terminal_Task::print_clock()
{
    static std::string ip_addr;

    if (ip_addr.empty())
    {
        struct ifaddrs *ifAddrStruct = nullptr;
        struct ifaddrs *ifa = nullptr;
        void *tmpAddrPtr = nullptr;
        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr)
            {
                continue;
            }

            if (ifa->ifa_addr->sa_family == AF_INET) // check it is IP4
            {
                // is a valid IP4 Address
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                ip_addr = ifa->ifa_name;
                ip_addr += "/";
                ip_addr += addressBuffer;
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6) // check it is IP6
            {
                // is a valid IP6 Address
                tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                ip_addr = ifa->ifa_name;
                ip_addr += "/";
                ip_addr += addressBuffer;
            }
        }

        if (ifAddrStruct != NULL)
        {
            freeifaddrs(ifAddrStruct);
        }
    }

    char buf[80];
    auto now = time(0);
    auto tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    m_last_clock_update = std::chrono::system_clock::now();
}

void DFB_Terminal_Task::process()
{
    if (m_has_update)
    {
        auto num_lines = m_message_queue.size();

        if (num_lines)
        {
        }

        print_clock();
        m_has_update = false;
    }
    else if ((std::chrono::system_clock::now() - m_last_clock_update) > 750ms)
    {
        m_has_update = true;
    }
}

}
