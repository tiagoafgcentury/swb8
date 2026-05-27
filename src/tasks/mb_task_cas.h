#pragma once

#include <functional>
#include <string_view>
#include <memory>

#include "mb_task.h"

#if defined(MBGUI_APP_CAS)
#include "cas/nagra/mb_nagra.h"
#endif

namespace mb {

class Task_CAS final: public Task
{
    friend class Task;

public:
#if not defined(MBGUI_APP_CAS)
    typedef std::function<void(NAGRA_NUID_t, NAGRA_CAID_t, NAGRA_SCUA_t, CAK_Version_t,
                               Project_Info_t, Chipset_Type_t, Chipset_Revision_t)> Fingerprint_Callback;
#endif

protected:
#if defined(MBGUI_APP_CAS)
    std::unique_ptr<Nagra> m_nagra;
#endif

    // IPC Events
#if defined(MBGUI_APP_CAS)
    virtual void handle_event_cas_fingerprint_get() override;
    virtual void handle_event_cas_request_descramble(const Event_CAS_Request_Descramble &_event) override;
    virtual void handle_event_cas_request_descramble_stop() override;
    virtual void handle_event_cas_request_descramble_pmt_update(const Event_CAS_Request_Descramble &_event) override;
    virtual void handle_event_cas_send_cat_table_section(const Event_CAS_CAT_Table_Section &_event) override;
    virtual void handle_event_player_change_audio(PID_t _new_pid) override;
    virtual void handle_event_system_factory_reset() override;

    virtual void handle_event_cas_pvr_timeshift_start(Event_PVR_Record_Param _param) override;
    virtual void handle_event_cas_pvr_timeshift_play() override;
    virtual void handle_event_cas_pvr_timeshift_stop() override;
    virtual void handle_event_cas_pvr_record_start(Event_PVR_Record_Param _param) override;
    virtual void handle_event_cas_pvr_record_stop() override;
    virtual void handle_event_cas_pvr_record_pause() override;
    virtual void handle_event_cas_pvr_record_resume() override;
    virtual void handle_event_cas_pvr_play_start(std::string url);
    virtual void handle_event_cas_pvr_play_stop() override;
    virtual void handle_event_cas_pvr_play_pause() override;
    virtual void handle_event_cas_pvr_play_resume() override;
    virtual void handle_event_cas_pvr_play_forward(uint16_t _mp_speed) override;
    virtual void handle_event_cas_pvr_play_rewind(uint16_t _mp_speed) override;
    virtual void handle_event_cas_pvr_play_next(std::string url) override;
    virtual void handle_event_cas_exit() override;

#else
    std::weak_ptr<Fingerprint_Callback> m_fingerprint_callback;

    virtual void handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event) override;
#endif

public:
    Task_CAS();
    virtual ~Task_CAS();

    static Task_CAS *get_instance();

    virtual void process() override;

#if not defined(MBGUI_APP_CAS)
    void get_cas_fingerprint(std::shared_ptr<Fingerprint_Callback> _callback);
#endif
};

} // namespace mb
