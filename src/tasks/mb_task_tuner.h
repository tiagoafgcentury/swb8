#pragma once

#include "mb_task.h"
#include "common/mb_types.h"
#include "hal/mb_tuner_signal_info.h"

#include <chrono>

namespace mb {

class Tuner;

class Task_Tuner final: public Task
{
    friend class Task;

private:
    std::unique_ptr<Tuner> m_tuner;

    enum class State
    {
        NORMAL,
        LOCKING,
        UNLOCKED,
        LNBF_AUTODETECT,
    };

    enum class BlindScanStep
    {
        Idle,
        H_Low,
        H_High,
        V_Low,
        V_High,
        Done
    };

    State m_state { State::UNLOCKED };
    Transponder_Id m_tp_locked;
    Transponder_Id m_tp_trying;

    std::chrono::steady_clock::time_point m_lock_started;

    void on_locked();

    struct LNBf_Autodetect_Ctx;
    std::shared_ptr<LNBf_Autodetect_Ctx> m_lnbf_autodetect_ctx;

    struct Blind_Scan_Ctx;
    std::shared_ptr<Blind_Scan_Ctx> m_blind_scan_ctx;

protected:
    virtual void handle_event_transponder_lock(const Transponder *_tp, bool _force = false) override;
    virtual void handle_event_blind_scan_progress(std::weak_ptr<Event_Blind_Scan_Progress> _event) override;
    virtual void handle_event_autodetect_lnbf_start(std::weak_ptr<Event_Autodetect_LNBf> _event) override;
    virtual void handle_event_set_default_lnbf() override;
    virtual void handle_event_set_lnbf_type(LNBF_Type _lnbf_type) override;
    virtual void handle_event_change_lnbf_type(std::function<void(const std::string &result)>);
#ifdef MBGUI_PERIODIC_DUMP
    virtual void handle_event_debbug_dump_status() override;
#endif // MBGUI_PERIODIC_DUMP

public:
    Task_Tuner();
    virtual ~Task_Tuner();

    virtual void process() override;

    typedef bool Is_Locked;
    static std::tuple<Transponder_Id, Is_Locked> get_current_transponder();

    static SignalInfo get_signal_info();
    static bool is_open();
    static bool is_locked();
};

}
