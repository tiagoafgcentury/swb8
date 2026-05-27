#include "sm_task_sat_monitor.h"
#include "sm_main.h"

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "hal/mb_tuner_signal_info.h"

namespace mb {

Task_Sat_Monitor::Task_Sat_Monitor()
{
    mb_assert(s_task_sat_monitor == nullptr);
    s_task_sat_monitor = this;
}

Task_Sat_Monitor::~Task_Sat_Monitor()
{
    mb_assert(s_task_sat_monitor == this);
    s_task_sat_monitor = nullptr;
}

void Task_Sat_Monitor::process()
{
    m_db_conn.poll();

    switch(m_state)
    {
        case Init:
            open_remote_db();
            transaction_start();
            break;

        case Started:
            post_event_lineup_build({});
            break;

        case Waiting_Lineup:
            break;

        case Done_Lineup:
            transaction_commit();
            g_mbgui_keep_running = false;
            break;
    }
}

void Task_Sat_Monitor::open_remote_db()
{
    if(!m_db_conn)
    {
        // cppcheck-suppress unknownMacro
        m_db_conn.connect("host = " MBGUI_POSTGRES_SERVER " port = 5440 dbname = portal user = sat password = U3t8QYg ");

        if(!m_db_conn)
        {
            std::cerr << m_db_conn.error_message() << std::endl;
            exit(EXIT_FAILURE);
        }

        if(!m_scan_id)
        {
            PQStatement stmt(m_db_conn);
            stmt.execute("select nextval('dvb.seq_scan_id');");
            m_scan_id = atoi(stmt.get_value(0, 0));
        }
    }
}

void Task_Sat_Monitor::save_tuner_lock_event(Transponder_Id _f, const SignalInfo *_info)
{
    auto id_satelite = 1;
    open_remote_db();
    m_db_conn.async_exec(R"sql(insert into dvb.transponders_lock(scan_id, id_satelite, id_monitor,
                                                        frequency, success, tuner_frequency, symbol_rate,
                                                        modulation_type, fec_rate, signal_type, strength,
                                                        quality, snr, ber)
values(%d, %d, %d, %d, true, %d, %d, %d, %d, %d, %d, %d, %d, %d))sql",
                         m_scan_id, id_satelite, m_id_monitor, _f.frequency(), _info->frequency, _info->symbol_rate,
                         (int)_info->mode_type, (int)_info->fec_rate, (int)_info->signal_type, _info->strength,
                         _info->quality, _info->signal_noise_ratio, _info->bit_error_rate);
}

void Task_Sat_Monitor::save_tuner_lock_event_failure(Transponder_Id _f)
{
    auto id_satelite = 1;
    open_remote_db();
    m_db_conn.async_exec(R"sql(insert into dvb.transponders_lock(scan_id, id_satelite, frequency, id_monitor, success)
values(%d, %d, %d, %d, false))sql",
                         m_scan_id, id_satelite, m_id_monitor, _f.frequency());
}

void Task_Sat_Monitor::save_lineup()
{
    const auto json_lineup { serialize(Lineup_Mutex_Ref::get_current_lineup().get()) };
    auto json { m_db_conn.escape_literal(json_lineup) };
    auto id_satelite = 1;
    m_db_conn.async_exec("select dvb.save_lineup(%d, %d, %d, %s ::jsonb)", id_satelite, m_scan_id, m_id_monitor, json.c_str());
    m_db_conn.set_exec_consumer([](PQConnection::Result_Set _result)
    {
        std::stringstream str;
        str << "Lineup Id: ";

        for(const auto &row : _result)
        {
            for(const auto &col : row)
            {
                str << col << "    ";
            }

            DEBUG_MSG(str.str() << "\n");
            str.clear();
        }

        DEBUG_MSG(flush);
    });
}

void Task_Sat_Monitor::transaction_start()
{
    open_remote_db();
    m_db_conn.async_exec("begin");
}

void Task_Sat_Monitor::transaction_commit()
{
    m_db_conn.async_exec("commit");
}

void Task_Sat_Monitor::handle_event_lineup_ready(const Event_Lineup_Ready &_event)
{
    if(_event.origin == Lineup_Origin::LO_SATELLITE)
    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        std::sort(current_lineup->transponders.begin(), current_lineup->transponders.end(),
                  [](const auto & rhs, const auto & lhs)
        {
            return rhs.transponder_id < lhs.transponder_id;
        }
                 );
        std::sort(current_lineup->services.begin(), current_lineup->services.end(),
                  [](const auto & rhs, const auto & lhs)
        {
            return ((rhs.viewer_channel() << 16) + rhs.service_id()) < ((lhs.viewer_channel() << 16) + lhs.service_id());
        }
                 );
        save_lineup();
        m_state = Done_Lineup;
    }
}

void Task_Sat_Monitor::handle_event_transponder_locked(const Event_Tuner_Lock &)
{
    using namespace std::chrono_literals;

    // Check if our clock is too close to 1970
    if(std::chrono::system_clock::now().time_since_epoch() < 72h)
    {
        post_event_clock_need_update();
    }
}

} // namespace mb
