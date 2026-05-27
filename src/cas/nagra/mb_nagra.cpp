extern "C" {
#include <ca_dpt.h>
#include "cak/ca_cak.h"
#include "cak/nv_debug.h"
#include "cak/ngwm_ree.h"
#include "cak/ca_sec.h"
#include "cak/ca_cert.h"
#include "cak/ca_dmx.h"

    // This definition is missing and causes compiler error in C++
    enum ECsdScsSize
    {
        ESCS_NAND_BOOT,
        ESCS_NOR_BOOT,
        ESCS_UNPROGRAMMED_BOOT,
        ESCS_NOT_SUPPORT_BOOT,
    };

#include <nocs_csd_impl.h>
#include <nocs_csd.h>
}

#undef max
#undef min

#include "mb_nagra.h"
#include "mb_ird_command.h"
#include "mb_pvr.h"
#include <aui_dmx.h>

#include "common/mb_assert.h"
#include "tasks/mb_task.h"

#include "ca_sec_util.h"

#include <iostream>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <poll.h>
#include <termios.h>
#include <thread>
#include <bitset>

using namespace std::placeholders;
namespace fs = std::filesystem;

#ifdef MBGUI_TPM
#include "hal/mb_otp.h"
#endif
#include "hal/mb_system.h"

#ifndef NDEBUG
#include <chrono>

using namespace std::chrono_literals;
#endif

#define NAGRA_EXEC(fn) do { [[maybe_unused]] auto ret = fn; mb_assert(CA_NO_ERROR == ret); } while (false)

namespace {

std::atomic<bool> s_nagra_is_paused = true;

int s_nagra_in = STDIN_FILENO;
int s_nagra_out = STDOUT_FILENO;

TCaStatus utc_time_importation_callback(TCalendarTime *pxUtcTime)
{
    auto system_time = mb::System::get_system_time();
    pxUtcTime->year = system_time.year();
    pxUtcTime->month = system_time.month();
    pxUtcTime->day = system_time.day();
    pxUtcTime->hour = system_time.hour();
    pxUtcTime->minute = system_time.minute();
    pxUtcTime->second = system_time.second();
    return CA_NO_ERROR;
}

void nagra_log(const char *pxMessage)
{
    [[maybe_unused]] auto _ = write(s_nagra_out, pxMessage, strlen(pxMessage));
}

uint32_t nagra_get_tick_count()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec * 1'000'000) + (t.tv_nsec / 1000);
}

int nagra_get_char()
{
    while(true)
    {
        struct pollfd readfds =
        {
            .fd = s_nagra_in,
            .events = POLLIN,
            .revents = 0,
        };
        auto pret = poll(&readfds, 1, NAGRA_STDIN_TIMEOUT.count());

        if(pret < 0)
        {
            DEBUG_MSG(CAS, ERROR, "poll() error: " << strerror(errno) << "\n");
        }

        if(readfds.revents & POLLIN)
        {
            char result;
            auto ret = read(s_nagra_in, &result, 1);

            if(ret == 1)
            {
                return result;
            }
            else
            {
                DEBUG_MSG(CAS, ERROR, "read() error: " << strerror(errno) << "\n");
            }
        }

        if(s_nagra_is_paused.load(std::memory_order_relaxed))
        {
            return EOF;
        }
    }
}

INvDebug g_debug_interface =
{
    .version =          DEBUGAPI_VERSION_INT,
    .log =              nagra_log,
    .getTickCount =     nagra_get_tick_count,
    .getChar =          nagra_get_char
};

}

int serial_open(char *serial_name, speed_t baud)
{
    struct termios params;
    auto tty = open(serial_name, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(tty < 0)
    {
        DEBUG_MSG(CAS, ERROR, "Error opening " << serial_name << " - " << strerror(errno));
        return tty;
    }

    if(tcgetattr(tty, &params) != 0)
    {
        DEBUG_MSG(CAS, ERROR, "Error tcgetattr() on " << serial_name << " - " << strerror(errno));
        return -1;
    }

    params.c_cflag &= ~PARENB; // Clear parity bit
    params.c_cflag |= CS8; // 8 bits per byte
    params.c_cflag &= ~CRTSCTS; // Disable RTS/CTS
    params.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    params.c_lflag &= ~ICANON; // Disable canonical mode - disable reading line-by-line, i.e. read byte-per-byte
    params.c_lflag &= ~ECHO; // Disable echo
    params.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    params.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    //params.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    //params.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    params.c_cc[VTIME] = 0; // Wait for up to X deciseconds, returning as soon as any data is received.
    params.c_cc[VMIN] = 1; // Min number of bytes until read() returns
    cfsetospeed(&params, baud);
    cfsetispeed(&params, baud);

    if(tcflush(tty, TCIFLUSH) == -1)
    {
        return -1;
    }

    if(tcflush(tty, TCOFLUSH) == -1)
    {
        return -1;
    }

    // Save tty settings, also checking for error
    if(tcsetattr(tty, TCSANOW, &params) != 0)
    {
        DEBUG_MSG(CAS, ERROR, "Error tcsetattr() on " << serial_name << " - " << strerror(errno));
        return -1;
    }

    return tty;
}

extern "C" {

    const INvDebug *nvGetDebugInterface(void)
    {
        return &g_debug_interface;
    }

}

namespace mb {

Nagra *Nagra::s_instance = nullptr;

#define CHECK_IS_PAUSED(RESULT...) \
    do { if (s_nagra_is_paused.load(std::memory_order_acquire)) { DEBUG_MSG(CAS, WARN, "CAK is paused: ignore " << __FUNCTION__ << "\n"); return RESULT; } } while (false)

#define DEFINE_LISTENER(TYPE) \
    TCaListener *Nagra::__ca_listerner_ ## TYPE = { nullptr };                                                                              \
    void Nagra::__ca_listerner_cb_ ## TYPE (const TCaListener *pxListener [[maybe_unused]], TCaExportation *pxExportation [[maybe_unused]]) \
    { DEBUG_LISTENER(TYPE); s_instance->push_event(CAK_EVT_ ## TYPE, pxExportation); }                                                  \

#define DEBUG_LISTENER(TYPE) \
    DEBUG_MSG(CAS, INFO, "CAK Event: " # TYPE << "\n")

#define REQUEST_CHECK(FN) \
    do { auto __ret = FN; if (CA_REQUEST_NO_ERROR != __ret){ DEBUG_MSG(CAS, WARN, "CA-REQUEST Error " << __FUNCTION__ << "\n");return Status::ERROR;} } while (false)


DEFINE_LISTENER(SMARTCARDS);
//DEFINE_LISTENER(PRODUCTS_LOADED);
//DEFINE_LISTENER(PRODUCTS_LOADING);
/*
* Ref.: Ativação SKY:
* Fazendo os testes, o CA_LISTENER_TYPE_PURCHASE_HISTORY é o que o box deve esperar.
* Reagindo só a partir deste ponto, e não apenas quando detecta a nova segmentação, o box conseguiu abrir os canais no primeiro ciclo.
*/
DEFINE_LISTENER(PURCHASE_HISTORY)
//DEFINE_LISTENER(RECHARGE_HISTORY);
//DEFINE_LISTENER(NEW_RECHARGE);
//DEFINE_LISTENER(TERMINATION);
//DEFINE_LISTENER(CREDITS);
DEFINE_LISTENER(ACCESS_RIGHTS);
DEFINE_LISTENER(PROGRAMS);
DEFINE_LISTENER(IRD_COMMAND);
DEFINE_LISTENER(NEW_PURCHASE);
DEFINE_LISTENER(SYSTEM);
DEFINE_LISTENER(OPERATORS);
//DEFINE_LISTENER(PIN_CODES);
DEFINE_LISTENER(SUBSCRIPTIONS);
//DEFINE_LISTENER(DATABASE);
//DEFINE_LISTENER(STB_LOCK);
DEFINE_LISTENER(POPUP);
DEFINE_LISTENER(CHIPSET_HARDWARE_RESET);
//DEFINE_LISTENER(PULL_EMM);

Nagra::Status Nagra::wait_for_request(TCaRequest *_request, std::chrono::milliseconds timeout)
{

    auto start_time = std::chrono::steady_clock::now();

    while(true)
    {
        TCaProcessingStatus processing_status;
        auto ret = caRequestGetProcessingStatus(_request, &processing_status);

        switch(ret)
        {
            case CA_REQUEST_NOT_PROCESSED:
            case CA_REQUEST_PROCESSING:
            {
                auto elapsed = std::chrono::steady_clock::now() - start_time;
                if (elapsed > timeout)
                {
                    DEBUG_MSG(CAS, ERROR, "Nagra Request TIMEOUT\n");
                    return Status::ERROR;
                }
                std::this_thread::sleep_for(1ms);
                continue;
            }
            case CA_REQUEST_NO_ERROR:
            case CA_REQUEST_PROCESSED:
                break;

            default:
                DEBUG_MSG(CAS, ERROR, "Processing request failed with: " << (int)ret << "\n");
                return Status::ERROR;
        }

        if (processing_status == CA_PROCESSING_NO_ERROR)
        {
            return Status::OK;
        }
        else
        {
            DEBUG_MSG(CAS, ERROR, "Processing request failed with status: " << (int)processing_status << "\n");
            return Status::ERROR;
        }
    }
}

void Nagra::send_msg_clear()
{
    send_popup_message(
    {
        .message = "",
        .category = Message_Categories::Program_Access
    });
}

void Nagra::send_msg_blocked(std::string_view msg, TCaAccess access_code, TUnsignedInt32 access_error_code)
{
    char buffer[1000];
    snprintf(buffer, sizeof(buffer), msg.data(), access_code, access_error_code);
    send_popup_message(
    {
        .message = buffer,
        .category = Message_Categories::Program_Access_Denied
    });
}

void Nagra::check_program_access(void *_program)
{
    TCaAccess access_code;
    caProgramGetAccess(static_cast<TCaProgram *>(_program), &access_code);
    TUnsignedInt32 access_error_code;
    caProgramGetAccessErrorCode(static_cast<TCaProgram *>(_program), &access_error_code);

    switch(access_code)
    {
        case CA_ACCESS_CLEAR: // 0
            DEBUG_MSG(CAS, INFO, "Conteúdo não codificado.\n");
            send_msg_clear();
            break;

        case CA_ACCESS_GRANTED: // 1
            DEBUG_MSG(CAS, INFO, "Acesso liberado pelo smartcard.\n");
            send_msg_clear();
            break;

        case CA_ACCESS_FREE: // 2
            DEBUG_MSG(CAS, INFO, "Acesso liberado pelo smartcard, o serviço é gratuito.\n");
            send_msg_clear();
            break;

        case CA_ACCESS_DENIED: // 100
            DEBUG_MSG(CAS, INFO, "Acesso negado pelo smartcard.\n");
            send_msg_blocked("Acesso negado pelo smartcard.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_NO_VALID_SECURE_DEVICE: // 101
            DEBUG_MSG(CAS, INFO, "The secure device (e.g. smartcard) is not inserted, or the CAK is\n"
                      "temporarily unable to communicate with it. it may also be incompatible\n"
                      "with the program to be descrambled (e.g. program operated by an\n"
                      "operator not managed by the secure device). Therefore, access is not\n"
                      "granted. In a card-based solution, it is advised to make a smart card\n"
                      "information request to get further information about the state of the\n"
                      "smart card.\n");
            send_msg_blocked("Acesso negado: smartcard ausente ou incompatível.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_SMARTCARD_BLACKLISTED: // 102
            DEBUG_MSG(CAS, INFO, "Deprecated. The smartcard is blacklisted. Access is not granted.\n");
            send_msg_blocked("Acesso negado: o smartcard não é válido.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_SMARTCARD_SUSPENDED: // 103
            DEBUG_MSG(CAS, INFO, "Deprecated. The smartcard is suspended. Access is not granted.\n");
            send_msg_blocked("O smartcard está suspenso. \nAcesso não foi autorizado.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_BLACKED_OUT: // 104
            DEBUG_MSG(CAS, INFO, "The related content (event, asset, ...) is blacked out in the user's area. Access is not granted.\n");
            send_msg_blocked("Este programa não está disponível para esta região.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_NO_VALID_CREDIT: // 105
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized, because there is not enough credit remaining.\n");
            send_msg_blocked("Acesso não autorizado: saldo insuficiente.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_COPY_PROTECTED:   // 106
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized, because it is copy-protected.\n");
            send_msg_blocked("Acesso não autorizado: o programa é protegido contra cópia.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_PARENTAL_CONTROL: // 107
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized, because of parental control settings\n");
            send_msg_blocked("Acesso não autorizado devido às configurações de controle parental.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_DIALOG_REQUIRED: // 108
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized and requires a dialog popup\n");
            send_msg_blocked("Acesso negado.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_PAIRING_REQUIRED: // 109
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized, because the smartcard is not paired.\n");
            send_msg_blocked("Acesso não autorizado: o smartcard não está pareado.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_CHIPSET_PAIRING_REQUIRED: // 110
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized, because the chipset is not paired.\n");
            send_msg_blocked("Acesso não autorizado: o smartcard não está pareado.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_EMI_UNSUPPORTED: // 111
            DEBUG_MSG(CAS, INFO, "The program is scrambled with an algorithm (EMI) that is not supported by the STB\n");
            send_msg_blocked("O programa é incompatível com este receptor.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_TSIO: // 112
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized, because TSIO mode is enforced on this program and the card does not support it.\n");
            send_msg_blocked("Acesso negado: este programa não é suportado pelo receptor.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_USAGE_RULES_VALIDATION_REQUIRED: // 113
            DEBUG_MSG(CAS, INFO, "The access to the program is granted but usages rules must be validated by the middleware before potentially setting keys in the descrambler.\n");
            send_msg_blocked("Acesso será liberado. \nPor favor, aguarde alguns instantes.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_USAGE_RULES_VIOLATION:    // 114
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized due to usage rules violation\n");
            send_msg_blocked("Acesso ao programa não foi autorizado \ndevido a uma violação das regras de uso.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_DEVICE_ACTIVATION_REQUIRED:   // 115
            DEBUG_MSG(CAS, INFO, "Access is denied: Activation is required.\n");
            send_msg_blocked("Acesso negado: é necessário ativar o receptor.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_CORRUPTED_ECM:   // 116
            DEBUG_MSG(CAS, INFO, "The access to the program is unknown since last ECM received by CAK is corrupted.\n"
                      "No keys have been set into the descrambler.\n");
            send_msg_blocked("Acesso negado.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_SOFTWARE_UPGRADE_REQUIRED:    // 117
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized since current device software has to be updated.\n");
            send_msg_blocked("Acesso negado. O software do aparelho precisa ser atualizado.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_USAGE_RULES_TRANSITION:  // 118
            DEBUG_MSG(CAS, INFO, "The access to the program is not authorized since a transition to new usage rules does not allow current descrambling.\n");
            send_msg_blocked("Acesso ao programa não é permitido devido a novas regras de uso.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_GRANTED_PPT: // 1000
            DEBUG_MSG(CAS, INFO, "Deprecated. The access is granted by the smartcard for a PPT service.\n");
            send_msg_blocked("Acesso é concedido pelo smartcard para um serviço PPT.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_FOR_PARENTAL_RATING: // 1001
            DEBUG_MSG(CAS, INFO, "Deprecated. The access is not authorized, because parental rating settings prevent it.\n");
            send_msg_blocked("Acesso bloqueado pelo controle parental.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_RIGHT_SUSPENDED: // 1002
            DEBUG_MSG(CAS, INFO, "Deprecated. The related entitlement is suspended. Access is not granted.\n");
            send_msg_blocked("Acesso negado: direito de acesso suspenso.\nCódigo: %d - %d.", access_code, access_error_code);
            break;

        case CA_ACCESS_DENIED_BUT_PPT: // 1003
            DEBUG_MSG(CAS, INFO, "Deprecated. The access to the program is not authorized, because it is a pay per time for which no time slice is currently activated.\n");
            send_msg_blocked("Acesso negado: nenhum período ativo para o conteúdo Pay Per Time.\nCódigo: %d - %d.", access_code, access_error_code);
            break;
    }
}

auto listener_status_to_str(const TCaListenerStatus _status)
{
#define RETURN_STAUTS_STR(STATUS) \
case CA_LISTENER_ ## STATUS: return #STATUS;

    switch(_status)
    {
            RETURN_STAUTS_STR(NO_ERROR);
            RETURN_STAUTS_STR(ERROR);
            RETURN_STAUTS_STR(INVALID_TYPE);
            RETURN_STAUTS_STR(INVALID);
            RETURN_STAUTS_STR(NO_MORE_RESOURCES);
            RETURN_STAUTS_STR(PARAMETER_INVALID);
            RETURN_STAUTS_STR(NOT_AVAILABLE);
            RETURN_STAUTS_STR(PARAMETER_MISSING);
            RETURN_STAUTS_STR(ALREADY_REGISTERED);
            RETURN_STAUTS_STR(NUM_STATUS);
    };

    return "{UNKNOWN}";
}

Nagra::Nagra()
{
    // Set serial port, if available
    auto serial_port = serial_open(const_cast<char *>("/dev/ttyUSB0"), B115200);

    if(serial_port > 0)
    {
        s_nagra_out = serial_port;
        s_nagra_in = serial_port;
    }

#ifndef NDEBUG
    {
        using namespace std;
        csdInitialize(nullptr);
        TCsd4BytesVector stb_ca_sn;
        csdGetStbCaSn(stb_ca_sn);
        cout << "CASN: '" << hex
             << setfill('0') << setw(2) << (int)stb_ca_sn[0]
             << setfill('0') << setw(2) << (int)stb_ca_sn[1]
             << setfill('0') << setw(2) << (int)stb_ca_sn[2]
             << setfill('0') << setw(2) << (int)stb_ca_sn[3] << "'\n";
    }
#endif
    mb_assert(s_instance == nullptr);
    s_instance = this;
    DEBUG_MSG(CAS, DEBUG, "Check: " MBGUI_NAGRA_PERSO_DATA_PATH "\n");
    const fs::path perso_data_path { MBGUI_NAGRA_PERSO_DATA_PATH };

    if(!fs::exists(perso_data_path))
    {
        fs::create_directories(perso_data_path);
    }

    if(std::string_view(MBGUI_NAGRA_STORAGE_PATH).compare(MBGUI_NAGRA_PERSO_DATA_PATH) != 0)
    {
        DEBUG_MSG(CAS, DEBUG, "Check: " MBGUI_NAGRA_STORAGE_PATH "\n");
        const fs::path storage_path { MBGUI_NAGRA_STORAGE_PATH };

        if(!fs::exists(storage_path))
        {
            fs::create_directories(storage_path);
        }
    }

    if(std::string_view(MBGUI_NAGRA_TRUSTED_STORAGE_PATH).compare(MBGUI_NAGRA_PERSO_DATA_PATH) != 0 &&
            std::string_view(MBGUI_NAGRA_TRUSTED_STORAGE_PATH).compare(MBGUI_NAGRA_STORAGE_PATH)
      )
    {
        DEBUG_MSG(CAS, DEBUG, "Check: " MBGUI_NAGRA_TRUSTED_STORAGE_PATH "\n");
        const fs::path trusted_storage_path { MBGUI_NAGRA_TRUSTED_STORAGE_PATH };

        if(!fs::exists(trusted_storage_path))
        {
            fs::create_directories(trusted_storage_path);
        }
    }

    /*
     * Step 1 - Initialization.
     */
    constexpr TUnsignedInt8 max_number_of_smartcards_ca = 0;
    DEBUG_MSG(CAS, INFO, "Initialization...\n");
    auto ret = caInitialization(nullptr, utc_time_importation_callback, max_number_of_smartcards_ca);

    if(CA_NO_ERROR != ret)
    {
        DEBUG_MSG_NL(CAS, INFO, "caInitialization failed: " << ret << endl);
    }
    else
    {
        DEBUG_MSG_NL(CAS, INFO, "caInitialization OK: " << ret << endl);
    }

    /*
     * Step 2 – listeners’ registration.
     */
    register_listeners();
    ret = caSetPersoDataPath(MBGUI_NAGRA_PERSO_DATA_PATH);

    if(CA_NO_ERROR != ret)
    {
        DEBUG_MSG(CAS, ERROR, "caSetPersoDataPath failed: " << ret << endl);
    }

    ret = caSetStoragePath(MBGUI_NAGRA_STORAGE_PATH);

    if(CA_NO_ERROR != ret)
    {
        DEBUG_MSG(CAS, ERROR, "caSetStoragePath failed: " << ret << endl);
    }

    ret = caSetTrustedStoragePath(MBGUI_NAGRA_TRUSTED_STORAGE_PATH);

    if(CA_NO_ERROR != ret)
    {
        DEBUG_MSG(CAS, ERROR, "caSetTrustedStoragePath failed: " << ret << endl);
    }

    //caSetWmkSwInterface();
    /*
     * Step 3 - Logging
     */
#ifdef NDEBUG
    {
        caLogDisable();
    }
#else
    {
        caLogEnable();
        TSize number_of_objects = 0;
        TChar **objects = nullptr;
        caLogGetCategories(&number_of_objects, &objects);
        DEBUG_MSG(CAS, DEBUG, "Debug Categories: ");

        for(TSize i = 0; i < number_of_objects; i++)
        {
            DEBUG_MSG_NL(CAS, DEBUG, objects[i] << " ");
        }

        DEBUG_MSG_NL(CAS, DEBUG, "\n");
        caLogGetRoles(&number_of_objects, &objects);
        DEBUG_MSG(CAS, DEBUG, "Debug Roles: ");

        for(TSize i = 0; i < number_of_objects; i++)
        {
            DEBUG_MSG_NL(CAS, DEBUG, objects[i] << " ");
        }

        DEBUG_MSG_NL(CAS, DEBUG, "\n");
        auto role = "INT";
        caLogSetRole("API", role);
        caLogSetRole("CAT", role);
        caLogSetRole("EMM", role);
        caLogSetRole("INIT", role);
        caLogSetRole("PROCESS_EMM", role);
        caLogSetRole("SMARTCARD", role);
        caLogSetRole("SYS", role);
        caLogSetRole("OPERATOR", role);
    }
#endif // NDEBUG
    /*
     * Step 4 – Start-up.
     */
    ret = caStartUp();

    if(CA_NO_ERROR != ret)
    {
        DEBUG_MSG(CAS, ERROR, "caStartUp failed: " << ret << endl);
        return;
    }
    else
    {
        DEBUG_MSG(CAS, INFO, "caStartUp OK: " << ret << endl);
        s_nagra_is_paused.store(false, std::memory_order_release);
    }

    // Init Some other suff
    TCaRequest *system_request { nullptr };
    auto req = caRequestCreate(CA_REQUEST_TYPE_SYSTEM, &system_request);

    if(CA_REQUEST_NO_ERROR != req)
    {
        return;
    }

    caRequestSend(system_request);
    wait_for_request(system_request);
    // Get System Objects
    TUnsignedInt32 number_of_objects = 0;
    TCaSystem **object_array = nullptr;
    req = caRequestGetObjects(system_request, &number_of_objects, (void ***)&object_array);

    if(CA_REQUEST_NO_ERROR != req)
    {
        return;
    }

    for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
    {
        // General System Stuff
        const char *cak_version = nullptr;
        caSystemGetCakVersion(object_array[i], &cak_version);
        DEBUG_MSG(CAS, INFO, "CAK Version: " << (cak_version ? cak_version : "<null>") << "\n");
    }

    caRequestDispose(system_request);

    init();

}

Nagra::~Nagra()
{
    pause();
    NAGRA_EXEC(caTermination());
    if (s_nagra_in != STDIN_FILENO && s_nagra_in != STDOUT_FILENO)
    {
        close(s_nagra_in);
    }
    if (s_nagra_out != STDOUT_FILENO && s_nagra_out != STDIN_FILENO && s_nagra_out != s_nagra_in)
    {
        close(s_nagra_out);
    }
    s_nagra_in = STDIN_FILENO;
    s_nagra_out = STDOUT_FILENO;

    mb_assert(s_instance == this);
    s_instance = nullptr;
}

void Nagra::init()
{
    set_popup_handler(
        [this](const Event_Display_Message& event)
        {
            send_popup_message(event);
        }
    );
}

void Nagra::push_event(CAK_EVENT _event_type, TCaExportation *_exportation)
{
    const std::lock_guard lock{m_events_lock};
    m_events.emplace_back(_event_type, _exportation);

    [[unlikely]] if(m_event_need_hardware_reset.load(std::memory_order_release))
    {
        m_event_burst_limiter = EVENT_BURST_CLOCK::now();
    }
}

[[maybe_unused]] static std::string_view to_str(Nagra::CAK_EVENT _event)
{
    switch(_event)
    {
#define ETS(EVT) case Nagra::CAK_EVT_ ## EVT : return # EVT;
            ETS(SMARTCARDS);
            ETS(PRODUCTS_LOADED);
            ETS(PRODUCTS_LOADING);
            ETS(PURCHASE_HISTORY);
            ETS(RECHARGE_HISTORY);
            ETS(NEW_RECHARGE);
            ETS(TERMINATION);
            ETS(CREDITS);
            ETS(ACCESS_RIGHTS);
            ETS(PROGRAMS);
            ETS(IRD_COMMAND);
            ETS(NEW_PURCHASE);
            ETS(SYSTEM);
            ETS(OPERATORS);
            ETS(PIN_CODES);
            ETS(SUBSCRIPTIONS);
            ETS(DATABASE);
            ETS(STB_LOCK);
            ETS(POPUP);
            ETS(CHIPSET_HARDWARE_RESET);
            ETS(PULL_EMM);
#undef ETS
    }

    return {};
}

void Nagra::process()
{
    process_ready_requests();
    process_ready_events();
    if (m_pvr)
    {
        process_ready_pvr_send_status();
    }
}

Event_PVR_Status Nagra::get_pvr_status()
{
    Event_PVR_Status _status = {};
    auto _f_state = m_pvr->function_state();
    _status.seq = ++seq;
    _status.state = static_cast<uint8_t>(m_pvr->state());
    _status.mount_point = m_pvr->pvr_mount_point();
    _status.filesystem_type = m_pvr->pvr_filesystem_type();
    switch(_f_state)
    {
        case PVR_Cas::Function_State::Record:
            _status.record_current_time = m_pvr->get_pvr_record_current_time();
            break;

        case PVR_Cas::Function_State::Play:
            _status.player_total_time = m_pvr->get_pvr_player_total_time();
            _status.player_current_time = m_pvr->get_pvr_player_current_time();
            break;

        case PVR_Cas::Function_State::Timeshift:
            _status.timeshift_record_curr_time = m_pvr->get_pvr_timeshift_rec_current_time();
            _status.timeshift_play_current_time = m_pvr->get_pvr_timeshift_play_current_time();
            break;

        case PVR_Cas::Function_State::None:
            _status.record_current_time = 0;
            _status.player_total_time = 0;
            _status.player_current_time = 0;
            _status.timeshift_record_curr_time = 0;
            _status.timeshift_play_current_time = 0;
            break;
    }

    return _status;
}

void Nagra::process_ready_pvr_send_status()
{

    using namespace std::chrono;
    auto now = steady_clock::now();

    if (m_last_pvr_status_time.time_since_epoch().count() != 0)
    {
        if (now - m_last_pvr_status_time < seconds(1))
            return;
    }

    m_last_pvr_status_time = now;


    auto status = get_pvr_status();

    if (status.state == static_cast<uint8_t>(PVR_Cas::State::Idle))
    {
        DEBUG_MSG(CAS, DEBUG, "status.state: " << static_cast<uint8_t>(PVR_Cas::State::Idle) << "\n");
        return;
    }

    Task::post_event_cas_pvr_get_status(status);
}

void Nagra::process_ready_events()
{
    decltype(m_events) local_events;
    {
        const std::lock_guard<std::mutex> lock(m_events_lock);

        [[likely]] if(m_events.empty() and not m_event_need_hardware_reset.load(std::memory_order_relaxed))
        {
            return;
        }
        else
        {
            std::swap(local_events, m_events);
        }
    }

    for(const auto &event : local_events)
    {
        DEBUG_MSG(CAS, INFO, "Got EVENT: " << to_str(event.event_type) << "\n");

        switch(event.event_type)
        {
            case CAK_EVT_SMARTCARDS:
            {
                if(event.exportation)
                {
                    TUnsignedInt32 number_of_objects = 0;
                    TCaSmartcard **object_array = nullptr;
                    auto ret = caExportationGetObjects(event.exportation, &number_of_objects, (void ***)&object_array);

                    if(CA_OBJECT_NO_ERROR == ret)
                    {
                        for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
                        {
                            check_smartcard(object_array[i]);
                        }
                    }
                }
                else
                {
                    check_for_smatcards();
                }

                break;
            }

            case CAK_EVT_PRODUCTS_LOADED:
                break;

            case CAK_EVT_PRODUCTS_LOADING:
            {
                /*
                * Ref.: Ativação SKY:
                * Fazendo os testes, o CA_LISTENER_TYPE_PURCHASE_HISTORY é o que o box deve esperar.
                * Reagindo só a partir deste ponto, e não apenas quando detecta a nova segmentação, o box conseguiu abrir os canais no primeiro ciclo.
                */
                break;
            }

            case CAK_EVT_PURCHASE_HISTORY:
                break;

            case CAK_EVT_RECHARGE_HISTORY:
                break;

            case CAK_EVT_NEW_RECHARGE:
                break;

            case CAK_EVT_TERMINATION:
                break;

            case CAK_EVT_CREDITS:
                break;

            case CAK_EVT_ACCESS_RIGHTS:
                break;

            case CAK_EVT_PROGRAMS:
            {
                if(event.exportation)
                {
                    TUnsignedInt32 number_of_objects = 0;
                    TCaProgram **object_array = nullptr;
                    auto ret = caExportationGetObjects(event.exportation, &number_of_objects, (void ***)&object_array);

                    if(CA_OBJECT_NO_ERROR == ret)
                    {
                        for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
                        {
                            check_program_access(object_array[i]);
                        }
                    }
                }
                else
                {
                    check_program_information();
                }

                break;
            }

            case CAK_EVT_IRD_COMMAND:
            {
                if(event.exportation)
                {
                    TUnsignedInt32 number_of_objects = 0;
                    TCaIrdCommand **object_array = nullptr;
                    auto ret = caExportationGetObjects(event.exportation, &number_of_objects, (void ***)&object_array);

                    if(CA_OBJECT_NO_ERROR == ret)
                    {
                        for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
                        {
                            TSize data_length = 0;
                            TUnsignedInt8 *data = nullptr;
                            auto ird_status = caIrdCommandGetData(object_array[i], &data_length, &data);

                            if(CA_OBJECT_NO_ERROR == ird_status)
                            {
                                process_ird_command(m_current_operator.value_or(Satellite_Operator::Generic), data, data_length);
                            }
                        }
                    }
                }

                break;
            }

            case CAK_EVT_NEW_PURCHASE:
                break;

            case CAK_EVT_SYSTEM:
            {
                if(event.exportation)
                {
                    TUnsignedInt32 number_of_objects = 0;
                    TCaSystem **object_array = nullptr;
                    auto ret = caExportationGetObjects(event.exportation, &number_of_objects, (void ***)&object_array);

                    if(CA_OBJECT_NO_ERROR == ret)
                    {
                        for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
                        {
                            TUnsignedInt32 number_of_filter_configs = 0;
                            const TCaMpegFilterCfg  **filter_configs = nullptr;
                            caSystemGetEcmMpegFilterCfgs(object_array[i], &number_of_filter_configs, &filter_configs);

                            for(TUnsignedInt32 i = 0; i < number_of_filter_configs; i++)
                            {
                                //auto cfg = &filter_configs[i];
                            }
                        }
                    }
                }

                break;
            }

            case CAK_EVT_OPERATORS:
            {
                // Get operator list
                TCaRequest *operators_request { nullptr };
                auto req = caRequestCreate(CA_REQUEST_TYPE_OPERATORS, &operators_request);

                if(CA_REQUEST_NO_ERROR != req)
                {
                    return;
                }

                caRequestSend(operators_request);

                TUnsignedInt32 number_of_objects = 0;
                TCaOperator **operator_array = nullptr;
                req = caRequestGetObjects(operators_request, &number_of_objects, (void ***)&operator_array);

                for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
                {
                    TSize size = 0;
                    const TUnsignedInt8* name = nullptr;
                    caOperatorGetName(operator_array[i], &size, &name);
                    DEBUG_MSG(CAS, INFO, "Got operator: " << std::string_view(reinterpret_cast<const char*>(name), size) << "\n");
                }

                caRequestDispose(operators_request);
                break;
            }

            case CAK_EVT_PIN_CODES:
                break;

            case CAK_EVT_SUBSCRIPTIONS:
                break;

            case CAK_EVT_DATABASE:
                break;

            case CAK_EVT_STB_LOCK:
                break;

            case CAK_EVT_POPUP:
            {
                if(event.exportation)
                {
                    TUnsignedInt32 number_of_objects = 0;
                    TCaPopup **object_array = nullptr;
                    auto ret = caExportationGetObjects(event.exportation, &number_of_objects, (void ***)&object_array);

                    if(CA_OBJECT_NO_ERROR == ret)
                    {
                        for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
                        {
                            TSize size = 0;
                            const TUnsignedInt8 *text = nullptr;
                            caPopupGetText(object_array[i], &size, &text);

                            if(m_callback_popup)
                            {
                                DEBUG_MSG(CAS, INFO, "Popup message: '" << std::string_view(reinterpret_cast<const char *>(text), size) << "'\n");
                                m_callback_popup(
                                {
                                    .message = std::string(reinterpret_cast<const char *>(text), size),
                                    .category = Message_Categories::Event_Popup
                                });
                            }
                            else
                            {
                                DEBUG_MSG(CAS, INFO, "Popup message ignored: '" << std::string_view(reinterpret_cast<const char *>(text), size) << "'\n");
                            }
                        }
                    }
                    else if(CA_OBJECT_NOT_AVAILABLE == ret)
                    {
                        // Clear the message
                        if(m_callback_popup)
                        {
                            DEBUG_MSG(CAS, INFO, "Clear Popup message.\n");
                            m_callback_popup(
                            {
                                .message = "",
                                .category = Message_Categories::Event_Popup
                            });
                        }
                    }
                }

                break;
            }

            case CAK_EVT_CHIPSET_HARDWARE_RESET:
            {
                m_event_need_hardware_reset.store(true, std::memory_order_release);
                DEBUG_MSG(CAS, WARN, "CAK Needs Reset: " << event.event_type << "\n");
                break;
            }

            default:
                DEBUG_MSG(CAS, WARN, "Event not handled: " << event.event_type << "\n");
                break;
        }
    }

    // TODO: This needs testing
    // HARDWARE RESET must only be sent after ALL events have been processed
    // by the CAK.
    if(m_event_need_hardware_reset.load(std::memory_order_acquire)
            and EVENT_BURST_CLOCK::now() - m_event_burst_limiter > EVENT_BURST_WAIT)
    {
        m_event_need_hardware_reset.store(false, std::memory_order_release);
        m_callback_need_reset();
    }
}

namespace {

constexpr auto __check_exportation()
{
    return false;
}

constexpr auto __check_exportation(bool _value)
{
    return _value;
}

constexpr auto ENABLE_EXPORTATION_MODE = true;

}

void Nagra::register_listeners()
{
#define REGISTER_LISTENER(TYPE, EXPORTATION...) \
    do {                                                                                                                                \
        auto listenStatus = caListenerCreate(CA_LISTENER_TYPE_ ## TYPE,                                                                 \
                                             __ca_listerner_cb_ ## TYPE,                                                                \
                                             &__ca_listerner_ ## TYPE);                                                                 \
        if (CA_LISTENER_NO_ERROR != listenStatus)                                                                                       \
        {                                                                                                                               \
            DEBUG_MSG(CAS, ERROR, "Failed to register: " # TYPE " = " << listenStatus << " - " << listener_status_to_str(listenStatus) << endl);    \
            caListenerDispose(__ca_listerner_ ## TYPE);                                                                                 \
        }                                                                                                                               \
        else                                                                                                                            \
        {                                                                                                                               \
            DEBUG_MSG(CAS, DEBUG, "Registered " # TYPE " OK!\n");                                                                                   \
        }                                                                                                                               \
        if ( __check_exportation( EXPORTATION ) )                                                                                       \
        {                                                                                                                               \
            DEBUG_MSG(CAS, DEBUG, "Set Exportation Mode " # TYPE " OK!\n");                                                                         \
            caListenerSetExportationMode(__ca_listerner_ ## TYPE);                                                                      \
        }                                                                                                                               \
        caListenerRegister(__ca_listerner_ ## TYPE);                                                                                    \
    } while(false)
    REGISTER_LISTENER(SMARTCARDS);
    /**<  Smartcards notification.
     */
    //REGISTER_LISTENER(PRODUCTS_LOADED);
    /**<  Notifies the application when a new product list is fully loaded.
     */
    //REGISTER_LISTENER(PRODUCTS_LOADING);
    /**<  Notifies the application when a new product list is available but
     *    not complete yet.
     */
    REGISTER_LISTENER(PURCHASE_HISTORY);
    /**<  Purchase history notification.
     */
    //REGISTER_LISTENER(RECHARGE_HISTORY);
    /**<  Notification occurring whenever the recharge history changes,
     *    that is to say when a recharge is added, removed or updated.
     *    This notification is also triggered by the smart card insertion
     *    in order to inform the application that the recharge history is
     *    available.
     */
    //REGISTER_LISTENER(NEW_RECHARGE);
    /**<  Notification occurring whenever the smart card receives a new
     *    recharge. Unlike the recharge history notification, this notification
     *    only occurs when the credit balance of the smart card is increased.
     */
    //REGISTER_LISTENER(TERMINATION);
    /**<  Termination notification.
     */
    //REGISTER_LISTENER(CREDITS);
    /**<  Smartcard credits notification.
     */
    REGISTER_LISTENER(ACCESS_RIGHTS);
    /**<  Notifies the application that rights managed by the smart card have
     *    changed. It is advised to re-compute the access status of any events
     *    or services upon such a notification.
     */
    REGISTER_LISTENER(PROGRAMS);
    /**<  Programs notification.
     */
    REGISTER_LISTENER(IRD_COMMAND, ENABLE_EXPORTATION_MODE);
    /**<  IRD command exportation.
     */
    REGISTER_LISTENER(NEW_PURCHASE);
    /**<  New purchase record.
     */
    REGISTER_LISTENER(SYSTEM);
    /**<  System notification.
     */
    //REGISTER_LISTENER(DEPRECATED_13);
    /**<  Deprecated.
     */
    REGISTER_LISTENER(OPERATORS);
    /**<  Operators notification.
     */
    //REGISTER_LISTENER(PIN_CODES);
    /**<  Pincodes notification.
     */
    REGISTER_LISTENER(SUBSCRIPTIONS);
    /**<  Subscription rights notification.
     */
    //REGISTER_LISTENER(DATABASE);
    /**<  Notifies the application whenever CAK database changes.
     */
    //REGISTER_LISTENER(STB_LOCK);
    /**<  StbLock notification.
     */
    REGISTER_LISTENER(POPUP, ENABLE_EXPORTATION_MODE);
    /**<  Popup notification
     */
    REGISTER_LISTENER(CHIPSET_HARDWARE_RESET);
    /**<  Notifies the middleware that a full hardware reset of the chipset is
     *    required. The middleware must call caPause() before resetting the
     *    chipset. It is recommended to make these operations (caPause + reset)
     *    as soon as possible, although the middleware is free to choose the
     *    most suitable time.
     */
    //REGISTER_LISTENER(PULL_EMM);
    /**<  Notifies the middleware that a connection to the backend is required
     *    to verify if out-of-band EMM needs to be processed by CAK.
     */
}

void Nagra::unregister_listeners()
{
#define UNREGISTER_LISTENER(TYPE)                                                                                                       \
    do {                                                                                                                                \
        if (__ca_listerner_ ## TYPE)                                                                                                    \
        {                                                                                                                               \
            auto listenStatus = caListenerDispose(__ca_listerner_ ## TYPE);                                                             \
            if (CA_LISTENER_NO_ERROR != listenStatus)                                                                                   \
            {                                                                                                                           \
                DEBUG_MSG(CAS, ERROR, "Failed to dispose: " # TYPE " = " << listenStatus << " - " << listener_status_to_str(listenStatus) << endl); \
            }                                                                                                                           \
            else                                                                                                                        \
            {                                                                                                                           \
                DEBUG_MSG(CAS, DEBUG, "Unegistered " # TYPE " OK!\n");                                                                              \
            }                                                                                                                           \
        }                                                                                                                               \
    } while(false)
    UNREGISTER_LISTENER(SMARTCARDS);
    //UNREGISTER_LISTENER(PRODUCTS_LOADED);
    //UNREGISTER_LISTENER(PRODUCTS_LOADING);
    UNREGISTER_LISTENER(PURCHASE_HISTORY);
    //UNREGISTER_LISTENER(RECHARGE_HISTORY);
    //UNREGISTER_LISTENER(NEW_RECHARGE);
    //UNREGISTER_LISTENER(TERMINATION);
    //UNREGISTER_LISTENER(CREDITS);
    UNREGISTER_LISTENER(ACCESS_RIGHTS);
    UNREGISTER_LISTENER(PROGRAMS);
    UNREGISTER_LISTENER(IRD_COMMAND);
    UNREGISTER_LISTENER(NEW_PURCHASE);
    UNREGISTER_LISTENER(SYSTEM);
    //UNREGISTER_LISTENER(DEPRECATED_13);
    UNREGISTER_LISTENER(OPERATORS);
    //UNREGISTER_LISTENER(PIN_CODES);
    UNREGISTER_LISTENER(SUBSCRIPTIONS);
    //UNREGISTER_LISTENER(DATABASE);
    //UNREGISTER_LISTENER(STB_LOCK);
    UNREGISTER_LISTENER(POPUP);
    UNREGISTER_LISTENER(CHIPSET_HARDWARE_RESET);
    //UNREGISTER_LISTENER(PULL_EMM);
}

void Nagra::pause()
{
    s_nagra_is_paused.store(true, std::memory_order_release);

    if(m_current_descrambling_request)
    {
        caRequestDispose(m_current_descrambling_request);
        m_current_descrambling_request = nullptr;
    }

    if(m_current_emm_filtering_request)
    {
        caRequestDispose(m_current_emm_filtering_request);
        m_current_emm_filtering_request = nullptr;
    }

    s_instance->unregister_listeners();
    DEBUG_MSG(CAS, DEBUG, "caPause\n");
    [[maybe_unused]] auto ret = caPause();
    DEBUG_MSG(CAS, INFO, "caPause = " << ret << " \n");
}

void Nagra::resume()
{
    s_instance->register_listeners();
    DEBUG_MSG(CAS, DEBUG, "caResume\n");
    auto ret = caResume();
    DEBUG_MSG(CAS, INFO, "caResume = " << ret << "\n");

    if(CA_NO_ERROR == ret)
    {
        s_nagra_is_paused.store(false, std::memory_order_release);
    }
}

std::tuple<NAGRA_NUID_t, NAGRA_CAID_t, NAGRA_SCUA_t, CAK_Version_t,
    Project_Info_t, Chipset_Type_t, Chipset_Revision_t> Nagra::get_fingerprint()
{
    CHECK_IS_PAUSED({});
    NAGRA_NUID_t nuid {};
    NAGRA_CAID_t caid {};
    CAK_Version_t cak_version;
    Project_Info_t project_info;
    Chipset_Type_t chipset_type;
    Chipset_Revision_t chipset_revision;
    {
        TCaRequest *request { nullptr };
        auto req = caRequestCreate(CA_REQUEST_TYPE_SYSTEM, &request);
        auto request_deleter = scoped_var(request, caRequestDispose);

        if(CA_REQUEST_NO_ERROR == req)
        {
            caRequestSend(request);
            if (wait_for_request(request) != Status::OK)
            {
                goto EXIT_GET_FINGERPRINT_SYSTEM;
            }

            TUnsignedInt32 xNumberOfObjects { 0 };
            void **ppxObjectArray { nullptr };
            caRequestGetObjects(request, &xNumberOfObjects, &ppxObjectArray);
            const TChar *buffer { nullptr };

            if(caSystemGetNuid(static_cast<TCaSystem *>(ppxObjectArray[0]), &buffer) == CA_OBJECT_NO_ERROR)
            {
                nuid = buffer;
            }

            if(caSystemGetIrdSerialNumber(static_cast<TCaSystem *>(ppxObjectArray[0]), &buffer) == CA_OBJECT_NO_ERROR)
            {
                caid = buffer;
            }

            if(caSystemGetCakVersion(static_cast<TCaSystem *>(ppxObjectArray[0]), &buffer) == CA_OBJECT_NO_ERROR)
            {
                cak_version = buffer;
            }

            if(caSystemGetProjectInformation(static_cast<TCaSystem *>(ppxObjectArray[0]), &buffer) == CA_OBJECT_NO_ERROR)
            {
                project_info = buffer;
            }

            if(caSystemGetChipsetType(static_cast<TCaSystem *>(ppxObjectArray[0]), &buffer) == CA_OBJECT_NO_ERROR)
            {
                chipset_type = buffer;
            }

            if(caSystemGetChipsetRevision(static_cast<TCaSystem *>(ppxObjectArray[0]), &buffer) == CA_OBJECT_NO_ERROR)
            {
                chipset_revision = buffer;
            }
        }
    }

    if(m_vua.empty())
    {
        check_for_smatcards();
    }

EXIT_GET_FINGERPRINT_SYSTEM:
    return {nuid, caid, m_vua, cak_version, project_info, chipset_type, chipset_revision};
}

class CASN
{
private:
    uint8_t m_data[STB_CA_SN_SIZE];

public:
    int operator[](int index) const
    {
        return m_data[index];
    }

    bool operator==(const uint8_t *_other)
    {
        return memcmp(m_data, _other, STB_CA_SN_SIZE) == 0;
    }

    bool operator!=(const uint8_t *_other)
    {
        return memcmp(m_data, _other, STB_CA_SN_SIZE) != 0;
    }

    void copy(const uint8_t *_other)
    {
        memcpy(m_data, _other, STB_CA_SN_SIZE);
    }

    uint8_t *data()
    {
        return m_data;
    }
};

std::ostream &operator<<(std::ostream &_str, const CASN &stb_ca_sn)
{
    using namespace std;
    _str << hex << setw(2) << setfill('0') << stb_ca_sn[0]
         << setw(2) << setfill('0') << stb_ca_sn[1]
         << setw(2) << setfill('0') << stb_ca_sn[2]
         << setw(2) << setfill('0') << stb_ca_sn[3];
    return _str;
}

Nagra::Status Nagra::request_program_descrambling(NID_t _original_network_id, TS_ID_t _transport_stream_id,
        DVB_Table_Section _pmt_section_data, PID_t _video_pid, PID_t _audio_pid, PID_t _subtitle_pid)
{
    CHECK_IS_PAUSED(Status::Paused);

    if(m_current_descrambling_request)
    {
        request_program_descrambling_stop();
    }

    TCaRequest *request { nullptr };
    REQUEST_CHECK(caRequestCreate(CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING, &request));
    m_current_descrambling_request = request;

    REQUEST_CHECK(caRequestSetTransportSessionId(request, TRANSPORT_SESSION_ID_PLAY));

    const TCaSourceId source_id =
        (static_cast<TCaSourceId>(_original_network_id) << 16) |
        static_cast<TCaSourceId>(_transport_stream_id);

    REQUEST_CHECK(caRequestSetSourceId(request, source_id));
    REQUEST_CHECK(caRequestSetExportationCallback(request, nagra_ca_request_exportation_callback));
    REQUEST_CHECK(caRequestSetPmtSection(request, _pmt_section_data.size(), _pmt_section_data.data()));

    TPid ca_pids[PID_Position::COUNT] = {0};
    int pid_count = 0;

    auto add_pid = [&](PID_t pid)
    {
        if (pid != 0 && pid != 0x1FFF)
            ca_pids[pid_count++] = pid;
    };

    // Monta lista real de PIDs
    add_pid(_video_pid);
    add_pid(_audio_pid);
    add_pid(_subtitle_pid);

    secSetPid2Sess(TRANSPORT_SESSION_ID_PLAY, SEC_DECRYPT, ca_pids, pid_count);
    REQUEST_CHECK(caRequestSetElemStreamPids(request, pid_count, ca_pids));

    return send_program_request(request);
}

Nagra::Status Nagra::change_audio_pid(PID_t _new_pid)
{
    secSetPid2Sess(TRANSPORT_SESSION_ID_PLAY, SEC_DECRYPT, &_new_pid, 1);
    REQUEST_CHECK(caRequestSetElemStreamPids(m_current_descrambling_request, 1, &_new_pid));
    return Status::OK;
}

Nagra::Status Nagra::request_update_pmt_section(DVB_Table_Section _pmt_section_data)
{
    if(m_current_descrambling_request)
    {
        DEBUG_MSG(CAS, INFO, "Update PMT\n");
        REQUEST_CHECK(caRequestSetPmtSection(m_current_descrambling_request, _pmt_section_data.size(), _pmt_section_data.data()));
        return send_program_request(m_current_descrambling_request);
    }
    else
    {
        DEBUG_MSG(CAS, ERROR, "Update PMT - NO CURRENT REQUEST\n");
        return Status::ERROR;
    }
}

Nagra::Status Nagra::send_program_request(TCaRequest *_request)
{
    REQUEST_CHECK(caRequestSend(_request));
    auto start_time = std::chrono::steady_clock::now();

    while(true)
    {
        TCaProcessingStatus processing_status;
        auto ret = caRequestGetProcessingStatus(_request, &processing_status);

        switch(ret)
        {
            case CA_REQUEST_NOT_PROCESSED:
            case CA_REQUEST_PROCESSING:
            {
                auto elapsed = std::chrono::steady_clock::now() - start_time;
                if (elapsed > 5s)
                {
                    DEBUG_MSG(CAS, ERROR, "Nagra Request TIMEOUT\n");
                    return Status::ERROR;
                }
                std::this_thread::sleep_for(1ms);
                continue;
            }

            case CA_REQUEST_NO_ERROR:
            case CA_REQUEST_PROCESSED:
                break;

            default:
                DEBUG_MSG(CAS, ERROR, "Processing request failed with: " << (int)ret << "\n");
                return Status::ERROR;
        }

        switch(processing_status)
        {
            case CA_PROCESSING_NO_ERROR:
                /**< the request has been processed successfully */
                return Status::OK;

            case CA_PROCESSING_ERROR:

            /**< an unspecified error occurred during the request processing */
            case CA_PROCESSING_NO_VALID_SMARTCARD:

            /**< Deprecated and renamed CA_PROCESSING_NO_VALID_SECURE_DEVICE to be compatible with card-less CAS */
            case CA_PROCESSING_PARAMETER_INVALID:

            /**< a parameter of the request is invalid */
            case CA_PROCESSING_PARAMETER_OUT_OF_RANGE:

            /**< a parameter of the request is out of range */
            case CA_PROCESSING_PARAMETER_MISSING:

            /**< a required parameter of the request is missing */
            case CA_PROCESSING_PARAMETER_INCOMPATIBLE:

            /**< one or more parameters are incompatible with this request */
            case CA_PROCESSING_TERMINATION:

            /**< the process stopped due to the CAK termination */
            case CA_PROCESSING_NO_MORE_RESOURCES:

            /**< the request processing stopped due to unavailability of a system resource (allocated memory, semaphore) */
            case CA_PROCESSING_LOW_CREDIT:

            /**< The smartcard does not contain enough credit to process the request */
            case CA_PROCESSING_NO_CREDIT:

            /**< The smartcard does not contain any credit at all and this prevents the CAK from processing the request */
            case CA_PROCESSING_CREDIT_SUSPENDED:

            /**< The smartcard credit is suspended and this prevents the CAK from processing the request. */
            case CA_PROCESSING_CREDIT_EXPIRED:

            /**< The smartcard credit is expired and this prevents the CAK from processing the request. */
            case CA_PROCESSING_MEMORY_FULL:

            /**< The smartcard memory is full and this prevents the CAK from processing the request. */
            case CA_PROCESSING_CONNECTION_ERROR:

            /**< The CAK faced a connection error and this prevents the CAK from processing the request. */
            case CA_PROCESSING_OUT_OF_PURCHASE_WINDOW:

            /**< The request did not complete successfully because of a purchase window problem. */
            case CA_PROCESSING_SUBSCRIBER_UNAUTHORIZED:

            /**< The request did not complete successfully because the subscriber is not authorized. */
            case CA_PROCESSING_PRODUCT_UNPURCHASABLE:

            /**< The request did not complete successfully because the product is not purchasable. */
            case CA_PROCESSING_BLACKOUT:

            /**< The request did not complete successfully because the subscriber is in blackout area. */
            case CA_PROCESSING_WRONG_PIN_CODE:

            /**< The request did not complete successfully because the given pincode was wrong. */
            case CA_PROCESSING_CONNECTION_INFO_MISSING:

            /**< The request did not complete successfully because some information (e.g. server address) is missing to establish a connection */
            case CA_PROCESSING_CONNECTION_IN_PROGRESS:
                /**< The request is rejected because a connection is already in progress.
                    T his tak*es into account connections triggered by the application and
                    the CAK itself. */
                return Status::ERROR;

            case CA_PROCESSING_NUM_STATUS: // Added only to silence compiler warning.
                break;
        }
    }
}

Nagra::Status Nagra::request_program_descrambling_stop()
{
    DEBUG_MSG(CAS, INFO, "Request program descrambling stop\n");
    CHECK_IS_PAUSED(Status::Paused);

    if(m_current_descrambling_request)
    {
        secFreeSessPID(TRANSPORT_SESSION_ID_PLAY, SEC_DECRYPT);
        auto req = caRequestDispose(m_current_descrambling_request);
        m_current_descrambling_request = nullptr;

        if(CA_REQUEST_NO_ERROR != req)
        {
            DEBUG_MSG(CAS, ERROR, "Dispose descrambling failed with: " << req << "\n");
        }

        return CA_REQUEST_NO_ERROR == req ? Status::OK : Status::ERROR;
    }

    return Status::OK;
}

void Nagra::request_is_ready_callback(TCaRequest *_request)
{
    const std::lock_guard lock{s_instance->m_requests_lock};
    s_instance->m_requests_ready.push_back(_request);
}

void Nagra::send_generic_nagra_request(TCaRequestType _request_type, std::function<void(void *)> _process_object, bool _async_call, TUnsignedInt32 _max_number_of_objects)
{
    TCaRequest *request { nullptr };
    auto req = caRequestCreate(_request_type, &request);

    if(CA_REQUEST_NO_ERROR != req)
    {
        return;
    }

    if(_async_call)
    {
        req = caRequestSetAsynchronousResponse(request, request_is_ready_callback);

        if(CA_REQUEST_NO_ERROR != req)
        {
            return;
        }

        {
            // Scope for mutex lock
            const std::lock_guard<std::mutex> lock(m_requests_lock);
            m_requests.emplace_back(
                request,
                _max_number_of_objects,
                std::move(_process_object)
            );
        }
    }

    caRequestSend(request);

    if(not _async_call)
    {
        wait_for_request(request);
        process_generic_nagra_request(request, _process_object, _max_number_of_objects);
        caRequestDispose(request);
    }
}

void Nagra::process_generic_nagra_request(TCaRequest *_request_object, std::function<void(void *)> _process_object, TUnsignedInt32 _max_number_of_objects)
{
    DEBUG_MSG(CAS, INFO, "Got REQUEST\n");
    /* Check whether the request has been processed successfully */
    TCaProcessingStatus proc_status = CA_PROCESSING_NUM_STATUS;
    auto req_status = caRequestGetProcessingStatus(_request_object, &proc_status);

    if(req_status != CA_REQUEST_NO_ERROR or proc_status != CA_PROCESSING_NO_ERROR)
    {
        DEBUG_MSG(CAS, ERROR, "Req_status: " << req_status << " Proc_status: " << proc_status << "\n");
        return;
    }

    TUnsignedInt32 number_of_objects = 0;
    void **objects = nullptr;
    req_status = caRequestGetObjects(_request_object, &number_of_objects, (void ***)&objects);

    if(req_status != CA_REQUEST_NO_ERROR)
    {
        DEBUG_MSG(CAS, ERROR, "Req_status: " << req_status << "\n");
        return;
    }

    number_of_objects = std::min(number_of_objects, _max_number_of_objects);

    for(TUnsignedInt32 i = 0; i < number_of_objects; i++)
    {
        _process_object(objects[i]);
    }
}

void Nagra::process_ready_requests()
{
    decltype(m_requests_ready) local_requests_ready;
    local_requests_ready.reserve(3); // Allocate some memory so when we swap the m_requests_ready already has memory allocated
    {
        const std::lock_guard<std::mutex> lock(m_requests_lock);

        if(m_requests_ready.empty())
        {
            return;
        }
        else
        {
            std::swap(local_requests_ready, m_requests_ready);
        }
    }

    for(const auto &request_ready : local_requests_ready)
    {
        if(request_ready)
        {
            Requests request;
            {
                // Limit scope for lock
                const std::lock_guard<std::mutex> lock(m_requests_lock);

                for(auto it = m_requests.begin(); it != m_requests.end(); it++)
                {
                    if(it->request_object == request_ready)
                    {
                        request = std::move(*it);
                        m_requests.erase(it);
                        break;
                    }
                }
            }
            process_generic_nagra_request(request_ready, request.process_object, request.max_number_of_objects);
            caRequestDispose(request_ready);
        }
    }
}

void Nagra::check_program_information()
{
    send_generic_nagra_request(CA_REQUEST_TYPE_PROGRAMS, std::bind(&Nagra::check_program_access, this, _1), false, 1);
}
#if 0
static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string geocode48ToBase64(const uint8_t bytes[6])
{
    uint64_t value = 0;
    for (int i = 0; i < 6; ++i)
    {
        value = (value << 8) | (uint64_t)bytes[i];
    }

    std::string out;
    out.reserve(8);
    for (int i = 0; i < 8; ++i)
    {
        int shift = (7 - i) * 6;
        uint8_t index = (value >> shift) & 0x3F;
        out.push_back(base64_table[index]);
    }
    return out;
}
#endif

void Nagra::check_smartcard(void *_smartcard)
{
    TSmartcardFlags flags = 0;
    auto ret = caSmartcardGetFlags(static_cast<TCaSmartcard *>(_smartcard), &flags);

    if(CA_OBJECT_NO_ERROR != ret)
    {
        DEBUG_MSG(CAS, ERROR, "Error reading Smartcard flags");
        return;
    }

    if((flags & CA_SMARTCARD_REMOVABLE) == 0)
    {
        TSize zip_code_size = 0;
        const TUnsignedInt8 *zip_code = nullptr;
        caSmartcardGetZipCode(static_cast<TCaSmartcard *>(_smartcard), &zip_code_size, &zip_code);
        const TChar *vua = nullptr;
        caSmartcardGetSerialNumber(static_cast<TCaSmartcard *>(_smartcard), &vua);
        const TChar *scver = nullptr;
        caSmartcardGetVersion(static_cast<TCaSmartcard *>(_smartcard), &scver);

        if(vua and strlen(vua))
        {
            m_vua = vua;
        }

        if(zip_code_size)
        {
            Zone_ID_t zone_id;
            Satellite_Operator oper;

            if (zip_code[0])
            {
                TSize segment_size = 0;
                const TUnsignedInt8 *segment_code = nullptr;
                caSmartcardGetSegments(static_cast<TCaSmartcard *>(_smartcard), &segment_size, &segment_code);
                const auto buf_size = (segment_size * 2) + 1;
                char buf[buf_size];
                auto p_e = buf;
                memset(buf, 0, buf_size);

                for(auto i = 0u; i < segment_size; i++, p_e += 2)
                {
                    snprintf(p_e, 3, "%.02x", (int)segment_code[i]);
                }

                DEBUG_MSG(CAS, INFO, "Got Seg Code: " << segment_size << " " << ret << " '" << buf << "'\n");

                uint16_t city_code = (segment_code[0] << 2) | (segment_code[1] >> 6);
                city_code = city_code & 0x3FF; // 0x3FF é 10 bits de máscara

                //std::string geocode = geocode48ToBase64(zip_code);
                //DEBUG_MSG(CAS, INFO,"\nGeocode: " << geocode << "\n");
                DEBUG_MSG(CAS, INFO,"\nCityCode: " << city_code << "\n");
                zone_id = city_code;
                oper = Satellite_Operator::Sky;
            }
            else
            {
                // Claro
                zone_id = zip_code[zip_code_size - 1];
                oper = Satellite_Operator::Claro;
            }

            bool changed =
                (!m_current_zone_id.has_value()) ||
                (!m_current_operator.has_value()) ||
                (m_current_zone_id.value() != zone_id) ||
                (m_current_operator.value() != oper);

            if (changed)
            {
                bool same_pending = false;

                if (m_pending_operator.has_value() &&
                    m_pending_zone_id.has_value())
                {
                    same_pending =
                        (m_pending_operator.value() == oper) &&
                        (m_pending_zone_id.value() == zone_id);
                }

                if (!same_pending)
                {
                    // First packet of possible transition
                    m_pending_operator = oper;
                    m_pending_zone_id = zone_id;
                    m_pending_counter = 1;

                    DEBUG_MSG(CAS, WARN,
                            "Pending operator change detected. "
                            "Waiting confirmation...\n");

                    return;
                }

                m_pending_counter++;

                // Require 2 identical packets
                if (m_pending_counter < 2)
                {
                    DEBUG_MSG(CAS, WARN,
                            "Waiting second confirmation packet...\n");

                    return;
                }

                DEBUG_MSG(CAS, INFO,
                        "Operator transition confirmed\n");

                Task::post_event_lineup_save_zone_id(oper, zone_id);

                m_current_zone_id = zone_id;
                m_current_operator = oper;

                m_pending_operator.reset();
                m_pending_zone_id.reset();
                m_pending_counter = 0;
            }

#ifndef NDEBUG
            const auto buffer_size = (zip_code_size * 2) + 1;
            char buffer[buffer_size];
            auto p = buffer;
            memset(buffer, 0, buffer_size);

            for(auto i = 0u; i < zip_code_size; i++, p += 2)
            {
                snprintf(p, 3, "%.02x", (int)zip_code[i]);
            }

            DEBUG_MSG(CAS, INFO, "Got Zip Code: " << zip_code_size << " " << ret << " '" << buffer << "'\n");
#endif
        }
        else
        {
            DEBUG_MSG(CAS, INFO, "Got Zip Code: <null>\n");
        }

        DEBUG_MSG(CAS, INFO, "Got vUA: '" << (vua ? vua : "<null>") << "' Version: '" << (scver ? scver : "<null>") << "'\n");
    }
}

void Nagra::check_for_smatcards()
{
    send_generic_nagra_request(CA_REQUEST_TYPE_SMARTCARDS, std::bind(&Nagra::check_smartcard, this, _1), false, 1);
}

void Nagra::nagra_ca_request_exportation_callback(const TCaRequest *_request, TCaExportation *_exportation)
{
    CHECK_IS_PAUSED();
    DEBUG_MSG(CAS, DEBUG, "CA Request Exportation Callback\n");
    (void)_request;
    scoped_var exportation_deleter(_exportation, caExportationDispose);

#if 0
        TCaObjectStatus objstatus;
    TUnsignedInt32 pxNumberOfObjects;
    TCaProgram** programObjectArray;
    TCaAccess pxAccess;
    if(_exportation != NULL)
    {
        printf("====== Thsi is Descrambling Request Exportation Callback!  ======\n");
        objstatus = caExportationGetObjects(_exportation,&pxNumberOfObjects,(void***)&programObjectArray);
        printf("[%s][%d] caExportationGetObjects = %d  , NumberOfObjects = %d\n",__FUNCTION__,__LINE__,objstatus,pxNumberOfObjects);
        if(CA_OBJECT_NO_ERROR == objstatus)
        {
            //objstatus = caProgramGetTransportSessionId((const TCaProgram*)programObjectArray,&pxTransportSessionId);
            //printf("[%s][%d] caProgramGetAccess = %d  ,  TransportSessionId = %d \n",__FUNCTION__,__LINE__,objstatus,pxTransportSessionId);
            objstatus = caProgramGetAccess(*programObjectArray,&pxAccess);
            printf("[%s][%d] caProgramGetAccess = %d  ,  pxAccess = %d \n",__FUNCTION__,__LINE__,objstatus,pxAccess);
            if(CA_OBJECT_NO_ERROR == objstatus)
            {
	         switch(pxAccess)
	         {
	             case CA_ACCESS_CLEAR:
                        printf("\n======   CA_ACCESS_CLEAR  by DeacrambleExportationCB  ======\n");
                        printf("======   The given content (event, asset, ...) is not scrambled.The CA is not required.   ======\n\n");
	                 break;

	             case CA_ACCESS_GRANTED:
                        printf("\n======   CA_ACCESS_GRANTED  by DeacrambleExportationCB   ======\n");
                        printf("======   Access is granted by the smartcard.   ======\n\n");
	                 break;

	             case CA_ACCESS_FREE:
                        printf("\n======   CA_ACCESS_FREE  by DeacrambleExportationCB   ======\n");
                        printf("======   Access is granted and the content is scrambled in free access mode.   ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED:
                        printf("\n======   CA_ACCESS_DENIED  by DeacrambleExportationCB   ======\n");
                        printf("======   Access is denied by the smartcard.  ======\n\n");
	                 break;

	             case CA_ACCESS_NO_VALID_SECURE_DEVICE:
                        printf("\n======   CA_ACCESS_NO_VALID_SECURE_DEVICE  by DeacrambleExportationCB   ======\n");
                        printf("======   The secure device(e.g. smartcard) is not inserted,or the CAK is temporarily unable to communicate with it.  ======\n\n");
	                 break;

	             case CA_ACCESS_SMARTCARD_BLACKLISTED:
                        printf("\n======   CA_ACCESS_SMARTCARD_BLACKLISTED  by DeacrambleExportationCB   ======\n");
                        printf("======   Deprecated. The smartcard is blacklisted. Access is not granted.  ======\n\n");
	                 break;

	             case CA_ACCESS_SMARTCARD_SUSPENDED:
                        printf("\n======   CA_ACCESS_SMARTCARD_SUSPENDED  by DeacrambleExportationCB   ======\n");
                        printf("======   Deprecated. The smartcard is suspended. Access is not granted.  ======\n\n");
	                 break;

	             case CA_ACCESS_BLACKED_OUT:
                        printf("\n======   CA_ACCESS_BLACKED_OUT  by DeacrambleExportationCB    ======\n");
                        printf("======   The related content(event, asset, ...) is blacked out in the user'sarea. Access is not granted.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_NO_VALID_CREDIT:
                        printf("\n======   CA_ACCESS_DENIED_NO_VALID_CREDIT  by DeacrambleExportationCB   ======\n");
                        printf("======   The access to the program is not authorized, because there is not enough credit remaining.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_COPY_PROTECTED:
                        printf("\n======   CA_ACCESS_DENIED_COPY_PROTECTED  by DeacrambleExportationCB   ======\n");
                        printf("======   The access to the program is not authorized, because it is copy-protected.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_PARENTAL_CONTROL:
                        printf("\n======   CA_ACCESS_DENIED_PARENTAL_CONTROL  by DeacrambleExportationCB   ======\n");
                        printf("======   The access to the program is not authorized, because of parental control settings.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_DIALOG_REQUIRED:
                        printf("\n======   CA_ACCESS_DENIED_DIALOG_REQUIRED   by DeacrambleExportationCB  ======\n");
                        printf("======  The access to the program is not authorized and requires a dialog popup.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_PAIRING_REQUIRED:
                        printf("\n======   CA_ACCESS_DENIED_PAIRING_REQUIRED  by DeacrambleExportationCB   ======\n");
                        printf("======  The access to the program is not authorized, because the smartcard is not paired.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_CHIPSET_PAIRING_REQUIRED:
                        printf("\n======   CA_ACCESS_DENIED_CHIPSET_PAIRING_REQUIRED  by DeacrambleExportationCB   ======\n");
                        printf("======  The access to the program is not authorized, because the chipset is not paired.  ======\n\n");
	                 break;

	             case CA_ACCESS_EMI_UNSUPPORTED:
                        printf("\n======   CA_ACCESS_EMI_UNSUPPORTED  by DeacrambleExportationCB   ======\n");
                        printf("======  The program is scrambled with an algorithm (EMI) that is not supported by the STB  ======\n\n");
	                 break;

	             case CA_ACCESS_GRANTED_PPT:
                        printf("\n======   CA_ACCESS_GRANTED_PPT  by DeacrambleExportationCB   ======\n");
                        printf("======  The access is not authorized, because parental rating settings prevent it.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_FOR_PARENTAL_RATING:
                        printf("\n======   CA_ACCESS_DENIED_FOR_PARENTAL_RATING  by DeacrambleExportationCB   ======\n");
                        printf("======  The access is not authorized, because parental rating settings prevent it.  ======\n\n");
	                 break;

	             case CA_ACCESS_RIGHT_SUSPENDED:
                        printf("\n======   CA_ACCESS_RIGHT_SUSPENDED  by DeacrambleExportationCB   ======\n");
                        printf("======  The related entitlement is suspended. Access is not granted.  ======\n\n");
	                 break;

	             case CA_ACCESS_DENIED_BUT_PPT:
                        printf("\n======   CA_ACCESS_DENIED_BUT_PPT  by DeacrambleExportationCB   ======\n");
                        printf("======  The access to the program is not authorized, because it is a pay per time for which no time slice is currently activated.  ======\n\n");
	                 break;

                    default:
                        printf("\n======   Access is another status!!!!!   ======\n\n");
		          break;
	         }
           printf("======  Descrambling callback execuction sucess !  ======\n");
           objstatus = caExportationDispose(_exportation);
           #ifdef CA_PPV
           MApi_CAK_GetPayPerViewDialog(currAccessStatus);
           #endif
           printf("[%s][%d] caExportationDispose = %d  \n",__FUNCTION__,__LINE__,objstatus);
           }
           else
           {
              printf("======  Descrambling callback fail !  ======\n");
              objstatus = caExportationDispose(_exportation);
              printf("[%s][%d] caExportationDispose = %d  \n",__FUNCTION__,__LINE__,objstatus);
           }
      }
      else
      {
          printf("======  Descrambling callback fail !  ======\n");
          objstatus = caExportationDispose(_exportation);
          printf("[%s][%d] caExportationDispose = %d  \n",__FUNCTION__,__LINE__,objstatus);
      }
   }
   else
   {
      printf("======  Descrambling callback fail !  ======\n");
      objstatus = caExportationDispose(_exportation);
      printf("[%s][%d] caExportationDispose = %d  \n",__FUNCTION__,__LINE__,objstatus);
   }


   scoped_var exportation_deleter(_exportation, caExportationDispose);
#endif

}

Nagra::Status Nagra::set_cat_table_section(const uint8_t *_data, size_t _size, bool /*_is_last_section*/)
{
    CHECK_IS_PAUSED(Status::Paused);

    // Start EMM filtering
    if(m_current_emm_filtering_request == nullptr)
    {
        TCaRequest *emm_request = nullptr;
        REQUEST_CHECK(caRequestCreate(CA_REQUEST_TYPE_EMM_FILTERING, &emm_request));
        m_current_emm_filtering_request = emm_request;
        REQUEST_CHECK(caRequestSetTransportSessionId(emm_request, TRANSPORT_SESSION_ID_PLAY));
    }

    constexpr auto CAT_HEADER = 8u;
    constexpr auto CRC_SIZE = 4u;

    struct CASID
    {
        uint16_t id1;
        uint16_t id2;
    };
    const CASID valid_casid[] = {
#include "cak/nv_casid_cak.TVD.h"
        , {0,0}
    };

    auto p = _data + CAT_HEADER;
    const auto end = _data + (_size - CRC_SIZE);
    while (p +2 < end)
    {
        auto sec_size = p[1];
        if (p + 2 + sec_size > end)
        {
            DEBUG_MSG(CAS, ERROR, "CAT parsing error: descriptor too large\n");
            break;
        }

        if (sec_size >= 2)
        {

            auto oper = __builtin_bswap16(*reinterpret_cast<const uint16_t*>(p + 2));

            for(const CASID* it = valid_casid; it->id1 and it->id2; it++)
            {
                if (oper == it->id1 or oper == it->id2)
                {
                    DEBUG_MSG(CAS, DEBUG, "CAT Oper: " << hex << oper << dec << "\n");
                    REQUEST_CHECK(caRequestSetDescriptors(m_current_emm_filtering_request, sec_size + 2, p));
                    break;
                }
            }
        }
        p += sec_size + 2;
    }

    REQUEST_CHECK(caRequestSend(m_current_emm_filtering_request));
    TCaProcessingStatus proc;
    caRequestGetProcessingStatus(m_current_emm_filtering_request, &proc);

    if(CA_PROCESSING_NO_ERROR != proc)
    {
#ifndef NDEBUG
        DEBUG_MSG(CAS, ERROR, "CAT processing error: " << proc << "\nData was: ");
        auto p = _data;
        auto end = p + (_size - CRC_SIZE);

        for(; p < end; p++)
        {
            DEBUG_MSG_NL(CAS, ERROR, setw(2) << setfill('0') << hex << (int)*p);
        }

        DEBUG_MSG_NL(CAS, ERROR, "\n");
#endif
    }
    else
    {
        DEBUG_MSG(CAS, INFO, "CAT processing OK\n");
    }

    return Status::OK;
}

void Nagra::request_cas_pvr_timeshift_start(Event_PVR_Record_Param _param)
{

    if(!m_pvr)
    {
        m_pvr = std::make_unique<PVR_Cas>();
    }

    if(m_pvr->pvr_init())
    {
        DEBUG_MSG(CAS, ERROR, "Failed to start start the pvr\n");
        return;
    }

    std::string folder = _param.url;
    DEBUG_MSG(CAS, DEBUG, "FOLDER: " << folder << "\n");

    if(m_pvr->pvr_attach(folder, PVR_Cas::PVR_Record_Mode::PVR_REC_AND_TMS_DISK))
    {
        DEBUG_MSG(CAS, ERROR, "Failed to attach disk!\n");
        return;
    }

    PVR_Cas::pvr_record_param p;
    p.pcr_pid         = _param.pcr_pid;
    p.video_pid       = _param.video_pid;
    p.video_type      = _param.video_type;
    p.audio_pid_count = _param.audio_pid_count;
    for (auto i = 0; i < p.audio_pid_count; i++)
    {
        p.audio_pid[i]  = _param.audio_pid[i];
        p.audio_type[i] = _param.audio_type[i];
    }
    p.audio_desc_pid = _param.audio_desc_pid;

    p.filename       = _param.filename;
    p.dmx_id          =  AUI_DMX_ID_DEMUX0;
    p.encrypt_type    = 0; //FTA
    p.is_ca_mode      = 0;
    p.do_decrypt      = 1; //do_decrypt
    p.key_fixed       = 1; //key_fixed
    p.rec_mode        = AUI_REC_MODE_TMS;
    p.record_flow     = 0; //ts

    printf("\nTS PID: v=%d a=%d pcr=%d\n", p.video_pid, p.audio_pid[0], p.pcr_pid);
    if (0 != m_pvr->pvr_record(&p))
    {
        DEBUG_MSG(CAS, ERROR, "pvr_record failed\n");
        return;
    }
    m_pvr->set_function_state(PVR_Cas::Function_State::Timeshift);
}

void Nagra::request_cas_pvr_timeshift_play()
{
    if(m_pvr)
    {
        DEBUG_MSG(CAS, DEBUG, "************************PVR player playback****************************\n");
        PVR_Cas::pvr_playback_param ply_param = {};

        std::string filename = m_pvr->pvr_record_filename();
        std::string mount = m_pvr->pvr_mount_point();

        if (!filename.empty() && !mount.empty())
        {
            char path_buf[1024];
            std::string full_path = mount;
            if (full_path.back() == '/')
            {
                full_path.pop_back();
            }
            full_path += "/PVR";
            if (filename.front() != '/')
            {
                full_path += "/";
            }
            full_path += filename;

            snprintf(path_buf, sizeof(path_buf), "%s", full_path.c_str());

            unsigned int index = 0;
            if (AUI_RTN_SUCCESS == aui_pvr_get_index_by_path(path_buf, &index))
            {
                ply_param.record_index = index;
            }
            else
            {
                DEBUG_MSG(CAS, ERROR, "Failed to get index for path: " << path_buf << "\n");
                return;
            }
            ply_param.filename = path_buf;
        }
        else
        {
            DEBUG_MSG(CAS, ERROR, "Missing filename or mount point for timeshift play\n");
            return;
        }

        ply_param.start_mode   = AUI_P_OPEN_FROM_HEAD;
        ply_param.encrypt_type = 0;
        ply_param.key_fixed    = 1;

        if(!m_pvr->pvr_play_open(&ply_param))
        {
            DEBUG_MSG(CAS, ERROR, "ali_pvr_play_open failed\n");
            return;
        }
        m_pvr->set_function_state(PVR_Cas::Function_State::Timeshift);
    }
}

void Nagra::request_cas_pvr_timeshift_stop()
{
    if(m_pvr)
    {
        m_pvr->set_function_state(PVR_Cas::Function_State::None);
        m_pvr->pvr_play_close();
        m_pvr->pvr_record_close();
        m_pvr->pvr_clear_tms();
        m_pvr.reset();
        m_pvr = nullptr;
    }
    Event_PVR_Status _status = {};
    _status.seq = ++seq;
    Task::post_event_cas_pvr_get_status(_status);

}


void Nagra::request_cas_pvr_record_start(Event_PVR_Record_Param _param)
{

    if(!m_pvr)
    {
        m_pvr = std::make_unique<PVR_Cas>();
    }

    if(m_pvr->pvr_init())
    {
        DEBUG_MSG(CAS, ERROR, "Failed to start start the pvr\n");
        return;
    }

    std::string folder = m_pvr->search_pvr_mount_point();
    DEBUG_MSG(CAS, DEBUG, "FOLDER: " << folder << "\n");

    if(m_pvr->pvr_attach(folder, PVR_Cas::PVR_Record_Mode::PVR_REC_AND_TMS_DISK))
    {
        DEBUG_MSG(CAS, ERROR, "Failed to attach disk!\n");
        return;
    }

    PVR_Cas::pvr_record_param p;

    p.video_pid       = _param.video_pid;
    p.video_type      = _param.video_type;
    p.audio_pid_count = _param.audio_pid_count;
    for (auto i = 0; i < p.audio_pid_count; i++)
    {
        p.audio_pid[i]  = _param.audio_pid[i];
        p.audio_type[i] = _param.audio_type[i];
    }
    p.audio_desc_pid = _param.audio_desc_pid;
    p.pcr_pid        = _param.pcr_pid;
    p.filename       = _param.filename;

    p.dmx_id          =  AUI_DMX_ID_DEMUX0;
    p.encrypt_type    = 0; //FTA
    p.is_ca_mode      = 0;
    p.do_decrypt      = 1; //do_decrypt
    p.key_fixed       = 1; //key_fixed
    p.rec_mode        = AUI_REC_MODE_NORMAL;
    p.record_flow     = 0; //ts

    m_pvr->pvr_record(&p);
    m_pvr->set_function_state(PVR_Cas::Function_State::Record);
}

void Nagra::request_cas_pvr_record_stop()
{
    if(m_pvr)
    {
        m_pvr->set_function_state(PVR_Cas::Function_State::None);
        m_pvr->pvr_record_close();
        m_pvr.reset();
        m_pvr = nullptr;
    }
    Event_PVR_Status _status = {};
    _status.seq = ++seq;
    Task::post_event_cas_pvr_get_status(_status);
}

void Nagra::request_cas_pvr_record_pause()
{
    if(m_pvr)
    {
        m_pvr->pvr_record_pause();
    }
}

void Nagra::request_cas_pvr_record_resume()
{
    if(m_pvr)
    {
        m_pvr->pvr_record_resume();
    }
}

void Nagra::request_cas_pvr_play_start(std::string url)
{
    if(!m_pvr)
    {
        m_pvr = std::make_unique<PVR_Cas>();
    }

    if(m_pvr->pvr_init())
    {
        DEBUG_MSG(CAS, ERROR, "Failed to start start the pvr\n");
        return;
    }

    auto pos = url.find("/PVR");

    if(pos != std::string::npos)
    {
        std::string mount_point = url.substr(0, pos);
        DEBUG_MSG(CAS, DEBUG, "Ponto de montagem: " << mount_point << "\n");

        if(m_pvr->pvr_attach(mount_point, PVR_Cas::PVR_Record_Mode::PVR_REC_AND_TMS_DISK))
        {
            DEBUG_MSG(CAS, ERROR, "Failed to attach disk!\n");
            return;
        }
    }
    else
    {
        DEBUG_MSG(CAS, ERROR, "Diretório 'PVR' não encontrado!\n");
    }

    unsigned int  index = 0;

    if(not url.empty())
    {
        char buf[255];
        sprintf(buf, "%s", url.c_str());

        if(AUI_RTN_SUCCESS != aui_pvr_get_index_by_path(buf, &index))
        {
            DEBUG_MSG(CAS, ERROR, "The file " << url << "doesn't exist!\n");
            return;
        }
    }

    PVR_Cas::pvr_playback_param ply_param = {};
    ply_param.record_index = index;
    ply_param.filename     = url.c_str();
    ply_param.start_mode   = AUI_P_OPEN_FROM_HEAD;
    ply_param.encrypt_type = 0;
    ply_param.key_fixed    = 1;
    ply_param.ply_flow     = 0;

    if(!m_pvr->pvr_play_open(&ply_param))
    {
        DEBUG_MSG(CAS, ERROR, "ali_pvr_play_open failed\n");
        return;
    }
    m_pvr->set_function_state(PVR_Cas::Function_State::Play);
}

void Nagra::request_cas_pvr_play_stop()
{
    if(m_pvr)
    {
        m_pvr->set_function_state(PVR_Cas::Function_State::None);
        m_pvr->pvr_play_close();
        m_pvr.reset();
        m_pvr = nullptr;
    }
    Event_PVR_Status _status = {};
    _status.seq = ++seq;
    Task::post_event_cas_pvr_get_status(_status);
}

void Nagra::request_cas_pvr_play_pause()
{
    if(m_pvr)
    {
        m_pvr->pvr_play_pause();
    }
}

void Nagra::request_cas_pvr_play_resume()
{
    if(m_pvr)
    {
        m_pvr->pvr_play_play();
    }
}

void Nagra::request_cas_pvr_play_forward(uint16_t _mp_speed)
{
    if(m_pvr)
    {
        m_pvr->pvr_play_forward(static_cast<PVR_Cas::PVR_Speed_Forward>(_mp_speed));
    }
}

void Nagra::request_cas_pvr_play_rewind(uint16_t _mp_speed)
{
    if(m_pvr)
    {
        m_pvr->pvr_play_rewind(static_cast<PVR_Cas::PVR_Speed_Rewind>(_mp_speed));
    }
}

void Nagra::request_cas_pvr_play_next(std::string url)
{
    if(m_pvr)
    {
        m_pvr->pvr_play_close();
        request_cas_pvr_play_start(url);
    }
}

bool CSC_Data::verify_data() const
{
    auto ret = dptVerifyCscData(data());
    return ret == 0;
}

} // namespace mb
