#pragma once

#include "../tasks/mb_task.h"
#include "mb_remote_control_handler.h"
#include "hal/mb_remote_control_keys.h"
#include "cas/nagra/mb_nagra.h"

#include <unistd.h>

#include <microhttpd.h>
#include <optional>

namespace mb {

struct connection_context;

class Task_HTTP_Server final : public Task, public Remote_Control_Handler
{
    friend class Task;

public:
    Task_HTTP_Server();
    ~Task_HTTP_Server();

    static std::optional<Remote_Control_Key> s_last_key_pressed;
    static std::optional<NAGRA_NUID_t> s_nuid;
    static std::optional<NAGRA_CAID_t> s_caid;
    static std::optional<NAGRA_SCUA_t> s_scua;

private:
    static constexpr auto HTTP_PORT = 5501;

    struct MHD_Daemon *m_daemon
    {
        nullptr
    };

    static MHD_Result MHD_AccessHandlerCallback(
        void *cls,
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *version,
        const char *upload_data,
        size_t *upload_data_size,
        void **con_cls);

    MHD_Result MHD_AccessHandlerCallback(
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *version,
        const char *upload_data,
        size_t *upload_data_size,
        void **con_cls);

protected:
    bool m_got_focus = false;

    virtual void process() override;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event) override;

#if defined(MBGUI_APP_GUI)
    static std::tuple<int, const char *, int> rc_key_send(connection_context *ctx);
    static std::tuple<int, const char *, int> get_zone_id(connection_context *ctx);
    static std::tuple<int, const char *, int> set_zone_id(connection_context *ctx);
#endif // MBGUI_APP_GUI
};

} // namespace mb
