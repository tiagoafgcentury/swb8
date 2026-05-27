#include "mb_demux_lineup_generic.h"
#include "mb_demux_lineup.h"
#include "mb_demux_table_manager.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "dvb/mb_dvb_types.h"
#include "hal/mb_demux.h"
#include "hal/mb_tuner.h"
#include "tasks/mb_task_demux.h"
#include "tasks/mb_task_tuner.h"
#include "mb_events.h"

#include <algorithm>
#include <chrono>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

using namespace std::chrono_literals;

namespace mb {

Demux_Lineup_Generic::Demux_Lineup_Generic(Task_Demux *_task_demux):
    Demux_Lineup(_task_demux)
{
}

Demux_Lineup_Generic::~Demux_Lineup_Generic()
{
}

void Demux_Lineup_Generic::parse_bat(BAT &&, Lineup *)
{
}

void Demux_Lineup_Generic::parse_nit(NIT &&_nit, Lineup *_lineup)
{
auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    auto transport_streams = _nit.move_transport_streams();

    if(sdts.size() < transport_streams.size())
    {
        sdts.reserve(transport_streams.size());
    }

    for(auto &ts : transport_streams)
    {
        auto it = _lineup->transponders.end();
        auto satellite_delivery_system_descriptor = ts.move_satellite_delivery_system_descriptor();

        if(satellite_delivery_system_descriptor.frequency())
        {
            it = set_satellite_descriptor(_lineup, _nit, ts, std::move(satellite_delivery_system_descriptor), sat_config);
        }
        else
        {
            auto s2_satellite_delivery_system_descriptor = ts.move_s2_satellite_delivery_system_descriptor();

            if(s2_satellite_delivery_system_descriptor.frequency())
            {
                it = set_satellite_descriptor(_lineup, _nit, ts, std::move(s2_satellite_delivery_system_descriptor), sat_config);
            }
        }

        if(it == _lineup->transponders.end())
        {
            continue;
        }

        check_sdt_ts(ts.transport_stream_id());
    }
}

void Demux_Lineup_Generic::parse_sdt(SDT &&_sdt, Lineup *_lineup)
{
    Transponder *tp { nullptr };
    const bool is_current { _sdt.table_id() == SDT_TABLE_ID_ACTUAL };
    auto transport_streams { _sdt.move_transport_streams() };

    if(is_current)
    {
        auto [current_transponder, _] = Task_Tuner::get_current_transponder();
        tp = _lineup->get_transponder(current_transponder);

        if(not tp)
        {
            DEBUG_MSG(DEMUX, ERROR, "[Generic][SDT] Current transponder not in lineup: freq="
                      << dec << current_transponder.frequency() / 1000 << " kHz — cannot map SDT services\n");
            return;
        }

        if(tp->original_network_id == 0) tp->original_network_id = _sdt.original_network_id();
        if(tp->transport_stream_id == 0) tp->transport_stream_id = _sdt.transport_stream_id();
    }
    else if(_sdt.transport_stream_id())
    {
        for(auto i = 0u; i < _lineup->transponders.size(); ++i)
        {
            tp = &_lineup->transponders[i];

            if(tp->original_network_id == _sdt.original_network_id() &&
                tp->transport_stream_id == _sdt.transport_stream_id() &&
                tp->satellite_id == m_current_satellite_id)
            {
                goto FOUND_TP;
            }
        }

        DEBUG_MSG(DEMUX, ERROR, "[Generic][SDT] No transponder in lineup matching ONID="
                  << dec << _sdt.original_network_id() << " TSID=" << _sdt.transport_stream_id()
                  << " SatID=" << m_current_satellite_id << " — services will be dropped\n");
        return;
    }

FOUND_TP:
    DEBUG_MSG(DEMUX, INFO, "[Generic][SDT] Matched tp=" << dec << tp->transponder_id.frequency() / 1000
              << " kHz  ONID=" << tp->original_network_id << " TSID=" << tp->transport_stream_id
              << " — SDT ONID=" << _sdt.original_network_id() << " TSID=" << _sdt.transport_stream_id()
              << "\n");
   
    // Get viewer channel starting point
    auto config = Config::get_config();
    auto sat_id = config->get_current_satellite();
    auto lineup = Lineup_Mutex_Ref::get_current_lineup();
    Viewer_Channel_t viewer_channel = m_base_viewer_channel + 1;

    for (const auto &s : lineup->services)
    {
        if (s.satellite_id() == sat_id)
        {
            if (s.viewer_channel() >= viewer_channel)
            {
                viewer_channel = s.viewer_channel() + 1;
            }
        }
    }

    for(auto &s : transport_streams)
    {
        auto sit = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                                [&](const auto & service)
        {
            return service.service_id() == s.service_id && service.transponder_id() == tp->transponder_id;
        });

        if(sit == _lineup->services.end())
        {
            auto basic_type = to_basic_type(s.service_type);
            if(basic_type != Basic_Service_Type::TV && basic_type != Basic_Service_Type::Radio)
            {
                DEBUG_MSG(DEMUX, DEBUG, "[Generic][SDT] SKIP service 0x" << hex << s.service_id
                          << " — not TV/Radio (type=0x" << (int)s.service_type << ")\n");
                continue;
            }

            _lineup->services.emplace_back(tp->transponder_id, s.service_id);
            sit = _lineup->services.begin();
            std::advance(sit, _lineup->services.size() - 1);
            sit->set_satellite_id(sat_id);
            sit->set_viewer_channel(viewer_channel++);
            DEBUG_MSG(DEMUX, INFO, "[Generic][SDT] ADD service 0x" << hex << s.service_id
                      << " ch=" << dec << sit->viewer_channel()
                      << " tp=" << tp->transponder_id.frequency() / 1000 << " kHz\n");
        }
        else
        {
            DEBUG_MSG(DEMUX, DEBUG, "[Generic][SDT] UPDATE service 0x" << hex << sit->service_id()
                      << " already in lineup\n");
        }

        if(sit->name().empty())
        {
            sit->set_name(s.move_service_name());
        }

        if(sit->service_type() == Service_Type::none)
        {
            sit->set_service_type(s.service_type);
        }
        DEBUG_MSG(DEMUX, INFO, "[Generic][SDT] Service 0x" << hex << sit->service_id()
                  << " '" << sit->name() << "' type=0x" << (int)sit->service_type()
                  << " satID=" << dec << sit->satellite_id()
                  << " ch=" << sit->viewer_channel() << "\n");
    }
}

void Demux_Lineup_Generic::parse_pmt(PMT &&_pmt, Lineup *_lineup)
{
    auto [current_tp, is_locked] = Task_Tuner::get_current_transponder();

    DEBUG_MSG(DEMUX, INFO, "[Generic][PMT] program=" << dec << _pmt.program_number()
              << " pcr_pid=0x" << hex << _pmt.pcr_pid()
              << " tp=" << dec << current_tp.frequency() / 1000 << " kHz\n");

    // If the service is not yet in the lineup (SDT may not have arrived yet),
    // pre-populate it from the PMT so that PID info is not lost.
    auto it = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                           [&](const auto &s)
    {
        return s.service_id() == _pmt.program_number() && s.transponder_id() == current_tp;
    });

    if(it != _lineup->services.end())
    {
        DEBUG_MSG(DEMUX, DEBUG, "[Generic][PMT] program=" << dec << _pmt.program_number()
                  << " already in lineup — forwarding to base parse_pmt\n");
    }
    // Only pre-add services from PMT when the PCR PID is valid.
    // PCR PID 0x1fff is the null PID, used by data-only programs
    // that basic_parse_pmt would ignore (no useful A/V streams).
    else if(_pmt.pcr_pid() == 0x1fff)
    {
        DEBUG_MSG(DEMUX, WARN, "[Generic][PMT] SKIP pre-add for program=" << dec << _pmt.program_number()
                  << " — PCR PID is null (0x1fff), data-only program\n");
    }
    else
    {
        auto config = Config::get_config();
        auto sat_id = config->get_current_satellite();

        Viewer_Channel_t viewer_channel = m_base_viewer_channel + 1;
        for (const auto &s : _lineup->services)
        {
            if (s.satellite_id() == sat_id && s.viewer_channel() >= viewer_channel)
            {
                viewer_channel = s.viewer_channel() + 1;
            }
        }

        _lineup->services.emplace_back(current_tp, _pmt.program_number());
        it = std::prev(_lineup->services.end());
        it->set_satellite_id(sat_id);
        it->set_viewer_channel(viewer_channel);
        DEBUG_MSG(DEMUX, INFO, "[Generic][PMT] PRE-ADD service 0x" << hex << _pmt.program_number()
                  << " satID=" << dec << sat_id
                  << " ch=" << viewer_channel
                  << " tp=" << current_tp.frequency() / 1000 << " kHz (SDT not yet received)\n");
    }

    Demux_Lineup::parse_pmt(std::move(_pmt), _lineup);
}

void Demux_Lineup_Generic::parse_tables(Lineup *_lineup)
{
    using namespace std::placeholders;

#define DO_PARSE(TABLE) do_parse(TABLE ## s, bind(&Demux_Lineup_Generic::parse_ ## TABLE, this, _1, _2), _lineup)

    // Generic lineup: services are discovered from PAT+PMT or SDT.
    // parse_pmt() pre-adds services if they are not yet in the lineup,
    // so PMTs can be processed at any time independently of SDT.
    DO_PARSE(pmt);

    // Parse SDT when fully received to fill in service names/types.
    // This will update services already pre-added by PMT parsing.
    if(done_tables & ptSDT)
    {
        DO_PARSE(sdt);
    }
    else
    {
        DEBUG_MSG(DEMUX, DEBUG, "[Generic][parse_tables] SDT not done yet (done_tables=0x"
                  << hex << done_tables << ") — skipping SDT parse\n");
    }

#undef DO_PARSE
}

bool Demux_Lineup_Generic::check_bats_is_done(Bouquet_ID_t)
{
    // For Generic, we don't strictly require BAT to consider the phase done unless we want to wait for it.
    // If we have it, we process it. If not, we don't block forever.
    // However, the caller 'bat_callback' calls this to see if it can stop requiring the table.
    if(bats.empty())
    {
        return false;
    }

    for(const auto &b : bats)
    {
        if (b.second.empty())
        {
            return false;
        }
        for(const auto &s : b.second)
        {
            if (not s.has_value())
            {
                return false;
            }
        }
    }
    return true;
}

bool Demux_Lineup_Generic::check_sdts_is_done()
{
    if (sdts.empty())
    {
        // If we have NIT but no SDTs found yet, maybe we are still waiting.
        // But if NIT says 0 TS? Unlikely.
        return false;
    }

    for(const auto &s : sdts)
    {
        if(s.second.empty())
        {
            return false;
        }

        for(const auto &v : s.second)
            if(!v.has_value())
            {
                return false;
            }
    }

    return true;
}

} // namespace mb
