#pragma once

#include "mb_task.h"
#include "mb_pg.h"
#include "common/mb_config.h"

namespace mb {

struct SignalInfo;

class Task_Sat_Monitor final: public Task
{
    friend class Task;

public:
    Task_Sat_Monitor();
    virtual ~Task_Sat_Monitor();

private:
    enum State
    {
        Init,
        Started,
        Waiting_Lineup,
        Done_Lineup
    };
    State m_state { Init };

    Config m_config;
    PQConnection m_db_conn;
    int m_scan_id { 0 };
    int m_id_monitor { 1 };

    void open_remote_db();
    void save_lineup();

    void save_tuner_lock_event(Transponder_Id _f, const SignalInfo *_info);
    void save_tuner_lock_event_failure(Transponder_Id _f);

    void transaction_start();
    void transaction_commit();
    virtual void handle_event_lineup_ready(const Event_Lineup_Ready &_event) override;
    virtual void handle_event_transponder_locked(const Event_Tuner_Lock &_event) override;

public:
    virtual void process() override;
};

} // namespace mb
