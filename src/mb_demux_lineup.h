#pragma once

#include "mb_demux_table_manager.h"
#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_tables.h"
#include "hal/mb_demux_macros.h"
#include "mb_table_map.h"
#include "mb_events.h"

#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <map>
#include <limits>
#include <unordered_set>

namespace mb {

class Demux;
class Task_Demux;

enum Parse_Tables
{
    ptNIT = 1,
    ptSDT = 2,
    ptPMT = 4,
    ptBAT = 8,
    ptPAT = 16,
};

class Demux_Lineup
{
public:
    std::chrono::steady_clock::time_point lineup_started;
    std::chrono::steady_clock::time_point tp_started;
    Lineup                              *lineup { nullptr };
    Transponder_List                    start_list;
    unsigned int                        start_list_idx { 0 };
    std::set<Transponder_Id>            locked_frequencies;
    std::set<Transponder_Id>            failed_frequencies;
    Table_Map<Bouquet_ID_t, BAT>        bats;
    std::vector<bool>                   nit_sections;
    Table_Map<Program_Number_t, PMT>    pmts;
    Table_Map<TS_ID_t, SDT>             sdts;
    uint8_t                             done_tables { 0 };
    std::set<std::pair<Program_Number_t, Service_ID_t>> update_pmts;

    std::unordered_map<EIT_Service_ID_t, uint8_t> m_eit_services_sections;
    std::unordered_set<EIT_Service_ID_t> m_service_filter;

    std::weak_ptr<Event_List_Update> event_callback;

protected:
    typedef uint16_t DVB_SECTION_NUMBER;
    static constexpr auto EIT_INVALID_SECTION_NUMBER
    {
        std::numeric_limits<DVB_SECTION_NUMBER>::max()
    };

    uint8_t m_time_set_retries { 0 };
    int m_process_passes_with_no_tables { 0 };
    size_t m_last_partial_services_size { 0 };
    uint16_t m_current_satellite_id { 0 };
    uint16_t m_scan_sat_count { 1 };
    uint16_t m_scan_sat_index { 0 };
    Viewer_Channel_t m_base_viewer_channel { 0 };

    Task_Demux *m_task_demux = nullptr;

    void maybe_emit_partial_update();

    inline Demux *get_demux();

    template<typename T, typename C>
    auto set_satellite_descriptor(Lineup *_lineup, const NIT &_nit, const Transport_Stream &_ts, T _sat, const C &_config);

    void bat_callback(PID_t _pid, BAT &&_bat);
    void sdt_callback(PID_t _pid, SDT &&_sdt);
    void nit_callback(PID_t _pid, NIT &&_nit);

    //virtual void build_start_list();
    virtual void start_tp();
    virtual void finish_tp();

    virtual void parse_bat(BAT &&_bat, Lineup *_lineup) = 0;
    virtual void parse_nit(NIT &&_nit, Lineup *_lineup) = 0;
    virtual void parse_sdt(SDT &&_sdt, Lineup *_lineup) = 0;
    virtual void parse_tables(Lineup *_lineup) = 0;

    virtual bool check_bats_is_done(Bouquet_ID_t _bat_id) = 0;
    virtual bool check_sdts_is_done() = 0;

    void check_sdt_ts(TS_ID_t _ts_id);

    static void basic_parse_pmt(PMT &&_pmt, Lineup *_lineup, Transponder_Id _tp_id, Viewer_Channel_t _base_viewer_channel = 0);

    template <typename T, typename F>
    void do_parse(T &_table, F _fn, Lineup *_lineup);

public:
    Demux_Lineup(Task_Demux *_task_demux);
    virtual ~Demux_Lineup();

    virtual void transponder_lock(const Event_Tuner_Lock &_event);
    void pat_program_callback(const PAT::Program &_program);
    virtual void parse_pmt(PMT &&_pmt, Lineup *_lineup);

    void start()
    {
        start_tp();
    }
    void process();
    void finish();

    void set_current_satellite_id(uint16_t _id)
    {
        m_current_satellite_id = _id;
    }
};

// Helpers for the set_satellite_descriptor template
Transponder &operator<<(Transponder &tp, const Satellite_Delivery_System_Descriptor &sat);
Transponder &operator<<(Transponder &tp, const S2_Satellite_Delivery_System_Descriptor &sat);

template<typename T, typename C>
auto Demux_Lineup::set_satellite_descriptor(Lineup *_lineup, const NIT &_nit, const Transport_Stream &_ts, T _sat, const C &_config)
{
    auto polarity { Polarity::UNDEFINED };

    switch(_sat.polarization())
    {
        case 0b00:
            polarity = Polarity::Horizontal;
            break;

        case 0b01:
            polarity = Polarity::Vertical;
            break;

        case 0b10:
            polarity = Polarity::Left;
            break;

        case 0b11:
            polarity = Polarity::Right;
            break;
    }

    auto it = std::find_if(_lineup->transponders.begin(), _lineup->transponders.end(),
                           [this, &_sat, polarity](const Transponder & v)
    {
        return v.transponder_id.frequency() ==  _sat.frequency() && v.transponder_id.polarity() == polarity && v.satellite_id == m_current_satellite_id;
    });

    // If hard-coded home-channel
    if(it != _lineup->transponders.end() && (it->network_id == 0 || it->original_network_id == 0))
    {
        DEBUG_MSG(DEMUX, DEBUG, "Replace: " << dec << _sat.frequency() << "\n");
        *it  << _sat;
    }
    else if(it == _lineup->transponders.end())
    {
        auto freq = _sat.frequency();

        if(freq >= _config.frequency_min && freq <= _config.frequency_max)
        {
            DEBUG_MSG(DEMUX, DEBUG, "Adding frequency: " << dec << _sat.frequency() << " TSID: " << (int)_ts.transport_stream_id() << " ONID: " << (int)_ts.original_network_id() << "\n");
            _lineup->transponders.emplace_back();
            it = _lineup->transponders.begin();
            std::advance(it, _lineup->transponders.size() - 1);
            it->set_satellite_id(m_current_satellite_id);
            *it << _sat;
        }
        else
        {
            DEBUG_MSG(DEMUX, DEBUG, "Ignore frequency: " << dec << _sat.frequency() << "\n");
            return _lineup->transponders.end();
        }
    }

    it->transport_stream_id = _ts.transport_stream_id();
    it->original_network_id = _ts.original_network_id();
    it->network_id = _nit.network_id();
    it->set_satellite_id(m_current_satellite_id);

    return it;
}

template <typename T, typename F>
void Demux_Lineup::do_parse(T &_table, F _fn, Lineup *_lineup)
{
    for(auto vec = _table.begin(); vec != _table.end(); vec++)
    {
        for(auto it = vec->second.begin(); it != vec->second.end(); it++)
        {
            if(it->has_value() && it->value().is_parsed == false)
            {
                _fn(std::move(it->value().table), _lineup);
                it->value().is_parsed = true;
            }
        }
    }
}

}
