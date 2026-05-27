#pragma once

#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_tables.h"
#include "hal/mb_demux_macros.h"
#include "mb_events.h"

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>

namespace mb {

class Demux;
class Task_Demux;

class Demux_Channel_List
{
public:
    Demux_Channel_List(Task_Demux *_task_demux);
    ~Demux_Channel_List();

    void start();

    void process();
    void transponder_lock(const Event_Tuner_Lock &_event);
    bool is_done() const { return m_current_tp_idx >= m_transponders.size(); }

    const std::vector<Service>& services() const { return m_services; }
    std::vector<Service> move_services() { return std::move(m_services); }

    void set_event_callback(std::weak_ptr<Event_List_Update_Channel_List> _event) { m_event_callback = std::move(_event); }

    // Public: called by Task_Demux to forward PMT data
    void pmt_callback(PID_t _pid, PMT &&_pmt);

private:
    void pat_callback(PID_t _pid, PAT &&_pat);
    void sdt_callback(PID_t _pid, SDT &&_sdt);

    void parse_sdt(SDT &&_sdt);
    void parse_pmt(PMT &&_pmt);

    void start_tp();
    void finish_tp();
    void setup_filters();
    void maybe_emit_partial_update();

    Task_Demux *m_task_demux;
    Demux *m_demux;

    std::vector<Transponder> m_transponders;
    size_t m_current_tp_idx { 0 };
    Transponder_Id m_current_tp_id;
    std::chrono::steady_clock::time_point m_tp_started;

    std::vector<Service> m_services;
    std::vector<Transponder> m_locked_transponders;

    std::unordered_set<Service_ID_t> m_requested_pmt_programs;
    std::unordered_set<Service_ID_t> m_parsed_pmt_programs;
    std::unordered_set<Service_ID_t> m_parsed_sdt_services;
    std::unordered_map<Service_ID_t, PID_t> m_pmt_pids;
    std::unordered_map<Service_ID_t, PMT> m_deferred_pmts;
    std::unordered_set<uint8_t> m_seen_sdt_sections;
    std::optional<uint8_t> m_sdt_last_section;

    ONID_t m_onid { 0 };
    TS_ID_t m_tsid { 0 };
    Viewer_Channel_t m_base_viewer_channel { 0 };

    int m_pending_pmts { 0 };
    bool m_sdt_done { false };
    bool m_pmt_done { false };
    bool m_tp_locked { false };
    bool m_tp_done { false };

    std::weak_ptr<Event_List_Update_Channel_List> m_event_callback;
};

} // namespace mb
