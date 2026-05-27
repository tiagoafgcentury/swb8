#pragma once

#include "common/mb_globals.h"

#include <microhttpd.h>

#include <chrono>

namespace mb {

struct connection_context
{
    connection_context()
    {
    }

    virtual ~connection_context()
    {
    }

    static void request_completed_base(void */*cls*/, struct MHD_Connection *connection,
                                       void **con_cls, enum MHD_RequestTerminationCode toe)
    {
        auto thiz = static_cast<struct connection_context *>(*con_cls);
        thiz->request_completed(connection, toe);
        delete thiz;
    }

    virtual void request_completed(struct MHD_Connection */*connection*/,
                                   enum MHD_RequestTerminationCode /*toe*/)
    {
    };
};

struct delayed_context
{
    const std::chrono::steady_clock::time_point start;
    std::chrono::milliseconds time_out = std::chrono::milliseconds{ 20'000 }; // 20s
    bool timed_out = false;

    delayed_context():
        start(decltype(start)::clock::now())
    {
    }

    virtual ~delayed_context()
    {
    }

    virtual bool test()
    {
        auto now = decltype(start)::clock::now();

        if(now - start > time_out)
        {
            timed_out = true;
            return true;
        }

        return false;
    }
};

struct upload_data_context: public connection_context
{
    std::string upload_data;
};

struct wait_remote_control_context: public upload_data_context, public delayed_context
{
    wait_remote_control_context()
    {
    }

    virtual bool test() override;
};

struct wait_key_panel_context: public upload_data_context, public delayed_context
{
    wait_key_panel_context()
    {
    }

    virtual bool test() override;
};

struct wait_get_system_info: public upload_data_context, public delayed_context
{
    wait_get_system_info();
    virtual bool test() override;
};

} // namespace mb
