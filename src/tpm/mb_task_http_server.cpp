#include "mb_task_http_server.h"

#include "mb_tpm.h"
#include "common/mb_globals.h"
#include "common/mb_hash.h"
#include "common/mb_config.h"
#include "mb_connection_context.h"
#include "mb_http_utils.h"
#include "../../project_version.h"

#include <lvgl/lvgl.h>

#if defined(MBGUI_APP_GUI)
#include "mb_zone_id.h"
#endif

#if __has_include(<cJSON.h>)
#include <cJSON.h>
#else
#include <cjson/cJSON.h>
#endif

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "mb_task_http_server.h"
#include "mb_tpm.h"

namespace fs = std::filesystem;

extern const char *GIT_VERSION;

namespace {

constexpr size_t POSTBUFFERSIZE = 32 * 1024;

template <typename T, typename... U>
MHD_Result build_response_for(MHD_Connection *connection, T t, U... args)
{
    auto [http_code, result, result_length] = t(args...);
    auto response = MHD_create_response_from_buffer(result_length, (void *) result, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "application/json");
    auto ret = MHD_queue_response(connection, http_code, response);
    MHD_destroy_response(response);
    return ret;
};

}

namespace mb {

std::optional<Remote_Control_Key> Task_HTTP_Server::s_last_key_pressed;
std::optional<NAGRA_NUID_t> Task_HTTP_Server::s_nuid;
std::optional<NAGRA_CAID_t> Task_HTTP_Server::s_caid;
std::optional<NAGRA_SCUA_t> Task_HTTP_Server::s_scua;

void painc_callback(void *,
                    [[maybe_unused]] const char *file,
                    [[maybe_unused]] unsigned int line,
                    [[maybe_unused]] const char *reason)
{
    DEBUG_MSG(TASK, ERROR, "HTTP PANIC: " << reason << "\n");
    mb_assert(false);
}

Task_HTTP_Server::Task_HTTP_Server()
{
    mb_assert(s_task_http_server == nullptr);
    s_task_http_server = this;
    MHD_set_panic_func(painc_callback, nullptr);
    m_daemon = MHD_start_daemon(MHD_NO_FLAG, HTTP_PORT, nullptr, nullptr,
                                Task_HTTP_Server::MHD_AccessHandlerCallback, this,
                                MHD_OPTION_NOTIFY_COMPLETED, &connection_context::request_completed_base, nullptr,
                                MHD_OPTION_END);
}

Task_HTTP_Server::~Task_HTTP_Server()
{
    mb_assert(s_task_http_server == this);
    s_task_http_server = nullptr;
    MHD_stop_daemon(m_daemon);
    m_daemon = nullptr;
}

void Task_HTTP_Server::process()
{
    if(m_daemon)
    {
        MHD_run(m_daemon);
    }
}

MHD_Result Task_HTTP_Server::MHD_AccessHandlerCallback(
    void *cls, struct MHD_Connection *connection, const char *url, const char *method,
    const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    return static_cast<Task_HTTP_Server *>(cls)->MHD_AccessHandlerCallback(
               connection, url, method, version, upload_data,
               upload_data_size, con_cls);
}

MHD_Result Task_HTTP_Server::MHD_AccessHandlerCallback(
    struct MHD_Connection *connection, const char *url, const char *method,
    const char * /*version*/, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    constexpr auto EMPTY_RESPONSE [[maybe_unused]] = [] { return std::make_tuple(nullptr, 0); };
    MHD_Result ret = MHD_NO;

    if(*con_cls == nullptr)
    {
        DEBUG_MSG(TASK, DEBUG, "Start HTTP Request: " << method << " " << url << "\n");
    }

#define IS_POST() (strncasecmp(method, "POST", 4) == 0)
#define BASIC_ROUTE(PATH, FN)                                                   \
case PATH ## _hash:                                                             \
    {                                                                           \
        if (*con_cls == nullptr)                                                \
        {                                                                       \
            *con_cls = new upload_data_context();                               \
            ret = MHD_YES;                                                      \
        }                                                                       \
        else if (*upload_data_size > 0)                                         \
        {                                                                       \
            auto ctx = static_cast<upload_data_context *>(*con_cls);            \
            ctx->upload_data.append(upload_data, *upload_data_size);            \
            *upload_data_size = 0;                                              \
            ret = MHD_YES;                                                      \
        }                                                                       \
        else if (*upload_data_size == 0)                                        \
        {                                                                       \
            auto ctx = static_cast<upload_data_context *>(*con_cls);            \
            ret = build_response_for(connection, FN, ctx);                      \
        }                                                                       \
        break;                                                                  \
    }
#define CONDITIONAL_ROUTE(PATH, CLASS, FN)                                      \
case PATH ## _hash:                                                             \
    {                                                                           \
        if (*con_cls == nullptr)                                                \
        {                                                                       \
            *con_cls = new CLASS();                                             \
            ret = MHD_YES;                                                      \
        }                                                                       \
        else if (*upload_data_size > 0)                                         \
        {                                                                       \
            auto ctx = static_cast<CLASS *>(*con_cls);                          \
            ctx->upload_data.append(upload_data, *upload_data_size);            \
            *upload_data_size = 0;                                              \
            ret = MHD_YES;                                                      \
        }                                                                       \
        else if (*upload_data_size == 0)                                        \
        {                                                                       \
            auto ctx = static_cast<CLASS *>(*con_cls);                          \
            if (ctx->test())                                                    \
            {                                                                   \
                ret = build_response_for(connection, FN, ctx);                  \
            }                                                                   \
            else                                                                \
            {                                                                   \
                ret = MHD_YES;                                                  \
            }                                                                   \
        }                                                                       \
        break;                                                                  \
    }

    if(*con_cls == nullptr && strcmp("/rc-key-pressed", url) == 0)
    {
        if(!m_got_focus)
        {
            m_got_focus = true;
            set_focus();
        }
    }

    switch(strhash(url))
    {
#if defined(MBGUI_APP_TPM)
            BASIC_ROUTE("/nagra/report", tpm::nagra_report);
            BASIC_ROUTE("/system/info", tpm::system_info);
            BASIC_ROUTE("/system/hwcn", tpm::update_hwcn);
            BASIC_ROUTE("/system/caid", tpm::update_caid);
            BASIC_ROUTE("/system/otp", tpm::update_otp_fuse);
            BASIC_ROUTE("/system/bootloader", tpm::update_bootloader);
            BASIC_ROUTE("/system/format-data", tpm::format_data_partition);
            BASIC_ROUTE("/system/tpm", tpm::update_tpm_version);
            BASIC_ROUTE("/system/display-message", tpm::display_message);
            BASIC_ROUTE("/system/copy-bootloader", tpm::copy_bootloader);
            BASIC_ROUTE("/usbs", tpm::get_usb_device_list);
            BASIC_ROUTE("/usb/write", tpm::get_usb_device_list);
            BASIC_ROUTE("/system/burn-bootloader", tpm::burn_bootloader);
            BASIC_ROUTE("/system/erase-license", tpm::erase_license);
            BASIC_ROUTE("/system/reboot", tpm::system_reboot);
            BASIC_ROUTE("/system/update", tpm::system_update);
            BASIC_ROUTE("/system/power-off", tpm::system_power_off);
            BASIC_ROUTE("/post_encrypted_files", tpm::post_encrypted_files);
            BASIC_ROUTE("/rc-keys", tpm::get_rc_key_list);
            BASIC_ROUTE("/tuners", tpm::get_tuner_list);
            BASIC_ROUTE("/buttons", tpm::get_button_list);
            BASIC_ROUTE("/hdcp-key", tpm::update_hdcp_key);
            BASIC_ROUTE("/hdcp-decrypt-key", tpm::update_hdcp_decrypt_key);
            BASIC_ROUTE("/update_cas_vendor_data", tpm::update_cas_vendor_data);
            BASIC_ROUTE("/update_cas_provider_data", tpm::update_cas_provider_data);
            BASIC_ROUTE("/get_nsc_data", tpm::get_nsc_data);
            BASIC_ROUTE("/update_license", tpm::update_license);
            BASIC_ROUTE("/get_pasl_status", tpm::get_pasl_status);
            BASIC_ROUTE("/update_mptool_license", tpm::update_mptool_license);
            BASIC_ROUTE("/get_mptool_license", tpm::get_mptool_license);
            BASIC_ROUTE("/get_mptool_config", tpm::get_mptool_config);
            BASIC_ROUTE("/get_mptool_logs", tpm::get_mptool_logs);
            BASIC_ROUTE("/nagra/fpk", tpm::update_nagra_fpk);
            BASIC_ROUTE("/nagra/csc", tpm::update_nagra_csc);
            BASIC_ROUTE("/nagra/pairing-key", tpm::update_nagra_pairing_key);
            BASIC_ROUTE("/update_nagra_secret_key", tpm::update_nagra_secret_key);
            BASIC_ROUTE("/system/channel-list", tpm::get_channel_list);
            BASIC_ROUTE("/post_channel_list", tpm::post_channel_list);
            CONDITIONAL_ROUTE("/rc-key-pressed", wait_remote_control_context, tpm::get_last_key_pressed);
            CONDITIONAL_ROUTE("/button-pressed", wait_key_panel_context, tpm::get_pressed_button);
#endif // MBGUI_APP_TPM
#if defined(MBGUI_APP_GUI)
            BASIC_ROUTE("/rc-key-send", rc_key_send);
            BASIC_ROUTE("/zone_id", (IS_POST() ? set_zone_id : get_zone_id));
#endif // MBGUI_APP_GUI

        default:
        {
            if(*con_cls == nullptr)
            {
                *con_cls = new upload_data_context();
                ret = MHD_YES;
            }
            else if(*upload_data_size > 0)
            {
                auto ctx = static_cast<upload_data_context *>(*con_cls);
                ctx->upload_data.append(upload_data, *upload_data_size);
                *upload_data_size = 0;
                ret = MHD_YES;
            }
            else if(*upload_data_size == 0)
            {
                [[maybe_unused]] std::string command = url;
                [[maybe_unused]] auto ctx = static_cast<upload_data_context *>(*con_cls);
#if defined(MBGUI_APP_TPM)

                if(command.substr(0, 7) == "/diseqc" && strcmp(method, "POST") == 0)
                {
                    return build_response_for(connection, tpm::set_diseqc_info, ctx);
                }
                else if(command.substr(0, 7) == "/diseqc" && strcmp(method, "GET") == 0)
                {
                    return build_response_for(connection, tpm::get_diseqc_info, ctx);
                }
                else if(command.substr(0, 14) == "/tuner/0/state" && strcmp(method, "POST") == 0)
                {
                    return build_response_for(connection, tpm::set_tuner_state, ctx, url);
                }
                else if(command.substr(0, 7) == "/tuner/" && strcmp(method, "POST") == 0)
                {
                    return build_response_for(connection, tpm::set_tuner_info, ctx, url);
                }
                else if(command.substr(0, 7) == "/tuner/" && strcmp(method, "GET") == 0)
                {
                    return build_response_for(connection, tpm::get_tuner_info, ctx, url);
                }
                else if(command.substr(0, 7) == "/tuner/" && strcmp(method, "DELETE") == 0)
                {
                    return build_response_for(connection, tpm::set_tuner_unlock, ctx);
                }
                else if(command.substr(0, 5) == "/leds")
                {
                    return build_response_for(connection, tpm::get_led_list);
                }
                else if(command.substr(0, 5) == "/led/" && strcmp(method, "GET") == 0)
                {
                    return build_response_for(connection, tpm::get_led_value, url);
                }
                else if(command.substr(0, 5) == "/led/" && strcmp(method, "POST") == 0)
                {
                    return build_response_for(connection, tpm::set_led_value, ctx, url);
                }
                else if(command.substr(0, 5) == "/usb/" && strcmp(method, "GET") == 0)
                {
                    return build_response_for(connection, tpm::usb_list_file, ctx, url);
                }
                else
                {
                    return build_response_for(connection, tpm::default_route, ctx);
                }

#endif // MBGUI_APP_TPM
            }

            break;
        }
    }

    return ret;
}

bool Task_HTTP_Server::handle_event_remote_control(const Event_Remote_Control &event)
{
    s_last_key_pressed = event.key;
    return false;
}

void Task_HTTP_Server::handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event)
{
    s_nuid = _event.nuid;
    s_caid = _event.caid;
    s_scua = _event.scua;
}

#if defined(MBGUI_APP_GUI)
std::tuple<int, const char *, int> Task_HTTP_Server::rc_key_send(connection_context *ctx)
{
    auto upload = dynamic_cast<upload_data_context *>(ctx);
    auto *json = cJSON_Parse(upload->upload_data.c_str());

    if(json)
    {
        auto count = cJSON_GetArraySize(json);

        for(int i = 0; i < count; i++)
        {
            auto row = cJSON_GetArrayItem(json, i);
            auto key = cJSON_GetObjectItem(row, "key");

            if(cJSON_IsString(key) && (key->valuestring != nullptr))
            {
                DEBUG_MSG(TASK, DEBUG, "HTTP Got: " << key->valuestring << "\n");

                switch(strhash(static_cast<const char *>(key->valuestring)))
                {
#define POST_KEY(KEY) case # KEY ## _hash: Remote_Control_Handler::post_event_remote_control({ .key = Remote_Control_Key:: KEY }); break
                        POST_KEY(KEY_0);
                        POST_KEY(KEY_1);
                        POST_KEY(KEY_2);
                        POST_KEY(KEY_3);
                        POST_KEY(KEY_4);
                        POST_KEY(KEY_5);
                        POST_KEY(KEY_6);
                        POST_KEY(KEY_7);
                        POST_KEY(KEY_8);
                        POST_KEY(KEY_9);
                        POST_KEY(KEY_BLUE);
                        POST_KEY(KEY_CC);
                        POST_KEY(KEY_CHDOWN);
                        POST_KEY(KEY_CHUP);
                        POST_KEY(KEY_FWD);
                        POST_KEY(KEY_RED);
                        POST_KEY(KEY_GREEN);
                        POST_KEY(KEY_ORANGE);
                        POST_KEY(KEY_YELLOW);
                        POST_KEY(KEY_REV);
                        POST_KEY(KEY_REC);
                        POST_KEY(KEY_SELECT);
                        POST_KEY(KEY_INFO);
                        POST_KEY(KEY_LAST);
                        POST_KEY(KEY_LR);
                        POST_KEY(KEY_MENU);
                        POST_KEY(KEY_MUTE);
                        POST_KEY(KEY_NEXT);
                        POST_KEY(KEY_PAUSE);
                        POST_KEY(KEY_PLAY);
                        POST_KEY(KEY_POWER);
                        POST_KEY(KEY_PREV);
                        POST_KEY(KEY_GUIDE);
                        POST_KEY(KEY_SLEEP);
                        POST_KEY(KEY_STOP);
                        POST_KEY(KEY_TVRADIO);
                        POST_KEY(KEY_VOLDOWN);
                        POST_KEY(KEY_VOLTAR);
                        POST_KEY(KEY_VOLUP);
                        POST_KEY(KEY_PLUS);
                        POST_KEY(KEY_OK);
                        POST_KEY(FRONT_PANEL_0);
                }
            }
        }
    }

    constexpr auto result = R"json({"Tecla":"Tecla processada com sucesso"})json";
    return {200, strdup(result), strlen(result)};
}

std::tuple<int, const char *, int> Task_HTTP_Server::get_zone_id(connection_context *ctx)
{
    (void)ctx;
    auto config = Config::get_config();
    Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    Zone_ID_t zone_id = Zone_ID::get_zone_id(_oper);
    return memsnprintf(200, R"json({"zone_id":%d})json", zone_id);
}

std::tuple<int, const char *, int> Task_HTTP_Server::set_zone_id(connection_context *ctx)
{
    GET_JSON_INT_VALUE(zone_id);
    auto config = Config::get_config();
    Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    Task::post_event_lineup_save_zone_id(_oper, zone_id.first.value());
    return memsnprintf(200, R"json({"zone_id":%d})json", zone_id.first.value());
}

#endif // MBGUI_APP_GUI

} // namespace mb
