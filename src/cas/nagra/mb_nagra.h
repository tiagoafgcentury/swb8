#pragma once

extern "C" {
#include "cak/ca_cak.h"
#include "cak/ca_sec.h"

}
#undef max
#undef min

#include <common/mb_types.h>
#include <common/mb_globals.h>
#include "mb_events.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <optional>
#include <vector>

#include "mb_pvr.h"

#define MBGUI_TPM 1

/**
 *  @brief
 *    This type represents a Listener used to perform a notification
 *    or exportation. The structure SCaListener is not exported.
 */
typedef struct SCaListener TCaListener;

/**
 *  @ingroup g_listener
 *
 *  @brief
 *    Type used for an exportation. The structure itself is private to the CAK to
 *    prevent external access to its fields.
 */
typedef struct SCaExportation TCaExportation;

namespace mb {

constexpr auto CSC_DATA_SIZE = 3000u;
constexpr auto STB_CA_SN_SIZE = 4u;

class CSC_Data : public std::vector<uint8_t>
{
public:
    CSC_Data()
    {
        resize(CSC_DATA_SIZE);
    }

    bool verify_data() const;
};

class PK_Data : public std::vector<uint8_t>
{
public:
    bool verify_data() const;
};

class Demux;

class Nagra
{
public:
    typedef std::function<void()> Callback_Need_Reset;
    using Popup_Callback = std::function<void(const Event_Display_Message&)>;


    static constexpr TTransportSessionId TRANSPORT_SESSION_ID_PLAY { 42 };
    static constexpr TTransportSessionId TRANSPORT_SESSION_ID_RECORD { 84 };

    enum class Status
    {
        OK = 0,
        Paused,
        ERROR,
        ERROR_MISMATCHED_VALUE,
    };

    enum CAK_EVENT
    {
        CAK_EVT_SMARTCARDS,
        CAK_EVT_PRODUCTS_LOADED,
        CAK_EVT_PRODUCTS_LOADING,
        CAK_EVT_PURCHASE_HISTORY,
        CAK_EVT_RECHARGE_HISTORY,
        CAK_EVT_NEW_RECHARGE,
        CAK_EVT_TERMINATION,
        CAK_EVT_CREDITS,
        CAK_EVT_ACCESS_RIGHTS,
        CAK_EVT_PROGRAMS,
        CAK_EVT_IRD_COMMAND,
        CAK_EVT_NEW_PURCHASE,
        CAK_EVT_SYSTEM,
        CAK_EVT_OPERATORS,
        CAK_EVT_PIN_CODES,
        CAK_EVT_SUBSCRIPTIONS,
        CAK_EVT_DATABASE,
        CAK_EVT_STB_LOCK,
        CAK_EVT_POPUP,
        CAK_EVT_CHIPSET_HARDWARE_RESET,
        CAK_EVT_PULL_EMM,
    };

private:
    static Nagra *s_instance;

    TCaRequest *m_current_descrambling_request = { nullptr };

    enum PID_Position
    {
        PP_Video,
        PP_Audio,
        PP_Subtitle,
        COUNT
    };

    TCaRequest *m_current_emm_filtering_request = { nullptr };

    Callback_Need_Reset m_callback_need_reset;
    Popup_Callback m_callback_popup;

    std::string m_vua;
    std::optional<Zone_ID_t> m_current_zone_id;
    std::optional<Satellite_Operator> m_current_operator;
    std::optional<Satellite_Operator> m_pending_operator;
    std::optional<Zone_ID_t> m_pending_zone_id;
    uint8_t m_pending_counter {0};

    void send_msg_clear();
    void send_msg_blocked(std::string_view msg, TCaAccess access_code, TUnsignedInt32 access_error_code);

    void send_popup_message(const Event_Display_Message& _event)
    {
        if(m_callback_popup)
        {
            m_callback_popup(_event);
        }
    }

#define DECLARE_LISTENER(TYPE) \
    static void __ca_listerner_cb_ ## TYPE (const TCaListener *pxListener, TCaExportation *pxExportation); \
    static TCaListener *__ca_listerner_ ## TYPE;

    DECLARE_LISTENER(SMARTCARDS);
    /**<  Smartcards notification.
     */
    //DECLARE_LISTENER(PRODUCTS_LOADED);
    /**<  Notifies the application when a new product list is fully loaded.
     */
    //DECLARE_LISTENER(PRODUCTS_LOADING);
    /**<  Notifies the application when a new product list is available but
     *    not complete yet.
     */
    DECLARE_LISTENER(PURCHASE_HISTORY);
    /**<  Purchase history notification.
     */
    //DECLARE_LISTENER(RECHARGE_HISTORY);
    /**<  Notification occurring whenever the recharge history changes,
     *    that is to say when a recharge is added, removed or updated.
     *    This notification is also triggered by the smart card insertion
     *    in order to inform the application that the recharge history is
     *    available.
     */
    //DECLARE_LISTENER(NEW_RECHARGE);
    /**<  Notification occurring whenever the smart card receives a new
     *    recharge. Unlike the recharge history notification, this notification
     *    only occurs when the credit balance of the smart card is increased.
     */
    //DECLARE_LISTENER(TERMINATION);
    /**<  Termination notification.
     */
    //DECLARE_LISTENER(CREDITS);
    /**<  Smartcard credits notification.
     */
    DECLARE_LISTENER(ACCESS_RIGHTS);
    /**<  Notifies the application that rights managed by the smart card have
     *    changed. It is advised to re-compute the access status of any events
     *    or services upon such a notification.
     */
    DECLARE_LISTENER(PROGRAMS);
    /**<  Programs notification.
     */
    DECLARE_LISTENER(IRD_COMMAND);
    /**<  IRD command exportation.
     */
    DECLARE_LISTENER(NEW_PURCHASE);
    /**<  New purchase record.
     */
    DECLARE_LISTENER(SYSTEM);
    /**<  System notification.
     */
    //DECLARE_LISTENER(DEPRECATED_13);
    /**<  Deprecated.
     */
    DECLARE_LISTENER(OPERATORS);
    /**<  Operators notification.
     */
    //DECLARE_LISTENER(PIN_CODES);
    /**<  Pincodes notification.
     */
    DECLARE_LISTENER(SUBSCRIPTIONS);
    /**<  Subscription rights notification.
     */
    //DECLARE_LISTENER(DATABASE);
    /**<  Notifies the application whenever CAK database changes.
     */
    //DECLARE_LISTENER(STB_LOCK);
    /**<  StbLock notification.
     */
    DECLARE_LISTENER(POPUP);
    /**<  Popup notification
     */
    DECLARE_LISTENER(CHIPSET_HARDWARE_RESET);
    /**<  Notifies the middleware that a full hardware reset of the chipset is
     *    required. The middleware must call caPause() before resetting the
     *    chipset. It is recommended to make these operations (caPause + reset)
     *    as soon as possible, although the middleware is free to choose the
     *    most suitable time.
     */
    // DECLARE_LISTENER(PULL_EMM);
    /**<  Notifies the middleware that a connection to the backend is required
     *    to verify if out-of-band EMM needs to be processed by CAK.
     */

#undef DECLARE_LISTENER

    struct Nagra_Event
    {
        CAK_EVENT event_type;
        TCaExportation *exportation = nullptr;

        Nagra_Event()
        {
        }

        Nagra_Event(CAK_EVENT _event_type, TCaExportation *_exportation):
            event_type(_event_type),
            exportation(_exportation)
        {
        }

        ~Nagra_Event()
        {
            if(exportation)
            {
                caExportationDispose(exportation);
            }
        }

        Nagra_Event(Nagra_Event &&_other):
            event_type(_other.event_type),
            exportation(_other.exportation)
        {
            _other.exportation = nullptr;
        }

        void operator=(Nagra_Event &&_other)
        {
            event_type = _other.event_type;
            exportation = _other.exportation;
            _other.exportation = nullptr;
        }
    };
    std::vector<Nagra_Event> m_events;
    std::mutex m_events_lock;

    typedef std::chrono::steady_clock EVENT_BURST_CLOCK;
    EVENT_BURST_CLOCK::time_point m_event_burst_limiter;
    static constexpr auto EVENT_BURST_WAIT = std::chrono::milliseconds
    {
        //Alterado tempo de 3000 para 15000 para dar tempo
        //do CAK receber todos os comandos via satelite
        15000
    };
    std::atomic<bool> m_event_need_hardware_reset = { false };

    struct Requests
    {
        Requests()
        {}

        Requests(TCaRequest *_request_object, TUnsignedInt32 _max_number_of_objects, std::function<void(void *)> _process_object):
            request_object(_request_object),
            max_number_of_objects(_max_number_of_objects),
            process_object(_process_object)
        {}

        TCaRequest *request_object = nullptr;
        TUnsignedInt32 max_number_of_objects = 0;
        std::function<void(void *)> process_object;
    };
    std::vector<Requests> m_requests;
    std::vector<TCaRequest *> m_requests_ready;
    std::mutex m_requests_lock;

    std::unique_ptr<PVR_Cas> m_pvr;

    void push_event(CAK_EVENT _event_type, TCaExportation *_exportation);

    static constexpr TUnsignedInt32 DEFAULT_MAX_NUMBER_OF_OBJECTS = 1000;

    void send_generic_nagra_request(TCaRequestType _request_type, std::function<void(void *)> _process_object, bool _async_call = true, TUnsignedInt32 _max_number_of_objects = DEFAULT_MAX_NUMBER_OF_OBJECTS);
    void process_generic_nagra_request(TCaRequest *_request_object, std::function<void(void *)> _process_object, TUnsignedInt32 _max_number_of_objects = DEFAULT_MAX_NUMBER_OF_OBJECTS);

    static void request_is_ready_callback(TCaRequest *_request);

    void process_ready_requests();
    void process_ready_events();

    void check_program_access(void *_program);
    static void nagra_ca_request_exportation_callback(const TCaRequest *_request, TCaExportation *_exportation);

    void check_smartcard(void *_smartcard);
    void check_for_smatcards();

    Status send_program_request(TCaRequest *_request);
    Status wait_for_request(TCaRequest *_request, std::chrono::milliseconds timeout = std::chrono::seconds(5));

    void process_ready_pvr_send_status();
    Event_PVR_Status get_pvr_status();
    std::chrono::steady_clock::time_point m_last_pvr_status_time{};
    uint32_t seq = 0;

public:
    Nagra();
    ~Nagra();

    void init();

    void process();

    void register_listeners();
    void unregister_listeners();

    void check_program_information();

    void pause();
    void resume();

    std::tuple<NAGRA_NUID_t, NAGRA_CAID_t, NAGRA_SCUA_t, CAK_Version_t,
        Project_Info_t, Chipset_Type_t, Chipset_Revision_t> get_fingerprint();

    Status request_program_descrambling(NID_t _original_network_id, TS_ID_t _transport_stream_id,
                                        DVB_Table_Section _pmt_section_data,
                                        PID_t _video_pid, PID_t _audio_pid, PID_t _subtitle_pid);

    Status request_update_pmt_section(DVB_Table_Section _pmt_section_data);

    Status request_program_descrambling_stop();

    Status change_audio_pid(PID_t _new_pid);

    Status set_cat_table_section(const uint8_t *_data, size_t _size, bool _is_last_section);

    void set_callback_need_reset(Callback_Need_Reset _callback_need_reset)
    {
        m_callback_need_reset = _callback_need_reset;
    }

    void set_popup_callback(Popup_Callback _popup_callback)
    {
        m_callback_popup = _popup_callback;
    }

    PVR_Cas::pvr_record_param to_pvr_record_param(const Event_PVR_Record_Param &src);

    void print_pvr_record_param(const PVR_Cas::pvr_record_param &p);

    void print_event_pvr_record_param(const Event_PVR_Record_Param &p);

    void request_cas_pvr_timeshift_start(Event_PVR_Record_Param _param);

    void request_cas_pvr_timeshift_play();

    void request_cas_pvr_timeshift_stop();

    void request_cas_pvr_record_start(Event_PVR_Record_Param _param);

    void request_cas_pvr_record_stop();

    void request_cas_pvr_record_pause();

    void request_cas_pvr_record_resume();

    void request_cas_pvr_play_start(std::string url);

    void request_cas_pvr_play_stop();

    void request_cas_pvr_play_pause();

    void request_cas_pvr_play_resume();

    void request_cas_pvr_play_forward(uint16_t _mp_speed);

    void request_cas_pvr_play_rewind(uint16_t _mp_speed);

    void request_cas_pvr_play_next(std::string url);

};

} // namespace mb

