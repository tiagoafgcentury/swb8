#include "mb_demux_channel_list.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "dvb/mb_dvb_types.h"
#include "hal/mb_demux.h"
#include "tasks/mb_task_demux.h"
#include "tasks/mb_task_tuner.h"

#include <algorithm>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

using namespace std::chrono_literals;

namespace mb {

Demux_Channel_List::Demux_Channel_List(Task_Demux *_task_demux):
    m_task_demux(_task_demux),
    m_demux(_task_demux->get_demux())
{
}

Demux_Channel_List::~Demux_Channel_List()
{
    auto config = Config::get_config();
    auto sat_id = config->get_current_satellite();
    std::cout << "\n=== Channel List (SatID: " << sat_id << "): " << std::dec << m_services.size() << " services found ===\n";
    std::cout << std::setfill(' ') <<std::left << std::setw(30) << "Name"
              << std::setfill(' ') << std::left << std::setw(10) << "SvcID"
              << std::setfill(' ') << std::left << std::setw(10) << "VideoPID"
              << std::setfill(' ') << std::left << std::setw(10) << "PmtPID"
              << std::setfill(' ') << std::left << std::setw(10) << "Type"
              << std::setfill(' ') << std::left << std::setw(10) << "SatID"
              << "\n";

    for (const auto &svc : m_services)
    {
        std::cout << std::setfill(' ') << std::left << std::setw(30) << svc.name()
                  << "0x" << std::setfill(' ') << std::left << std::setw(8) << std::dec << svc.service_id()
                  << "0x" << std::setfill(' ') << std::left << std::setw(8) << std::hex << svc.video_pid()
                  << "0x" << std::setfill(' ') << std::left << std::setw(8) << std::hex << svc.pmt_pid()
                  << std::setfill(' ') << std::left << std::setw(10) << to_str(svc.service_type())
                  << std::dec << svc.satellite_id()
                  << "\n";
    }
    std::cout << "=== Channel List end ===\n\n";

    finish_tp();

    // Get last viewer channel from lineup for the current satellite
    auto lineup = Lineup_Mutex_Ref::get_current_lineup();

    // Remove old services for the current satellite so the new scan fully replaces them
    auto old_srv_count = lineup->services.size();
    lineup->services.erase(
        std::remove_if(lineup->services.begin(), lineup->services.end(),
                        [sat_id](const auto &s) { return s.satellite_id() == sat_id; }),
        lineup->services.end());

    // Also remove hidden services for this satellite
    lineup->hidden_services.erase(
        std::remove_if(lineup->hidden_services.begin(), lineup->hidden_services.end(),
                        [sat_id](const auto &s) { return s.satellite_id() == sat_id; }),
        lineup->hidden_services.end());

    // Remove old transponders for the current satellite
    lineup->transponders.erase(
        std::remove_if(lineup->transponders.begin(), lineup->transponders.end(),
                        [sat_id](const auto &tp) { return tp.satellite_id == sat_id; }),
        lineup->transponders.end());

    DEBUG_MSG(DEMUX, INFO, "Channel List: removed " << (old_srv_count - lineup->services.size())
              << " old services for SatID " << sat_id << "\n");

    Viewer_Channel_t viewer_channel = m_base_viewer_channel + 1;

    DEBUG_MSG(DEMUX, INFO, "Channel List: setting viewer channel starting at " << viewer_channel << " (SatID: " << sat_id << ")\n");
   // Ensure all successfully scanned transponders are in the lineup
    for (const auto &ltp : m_locked_transponders)
    {
        if (lineup->get_transponder(ltp.transponder_id) == nullptr)
        {
            lineup->transponders.push_back(ltp);
            DEBUG_MSG(DEMUX, INFO, "Channel List: added transponder " << ltp.transponder_id << " to lineup\n");
        }
        else
        {
            auto tp = lineup->get_transponder(ltp.transponder_id);
            if (tp)
            {
                if (ltp.original_network_id != 0) tp->original_network_id = ltp.original_network_id;
                if (ltp.transport_stream_id != 0) tp->transport_stream_id = ltp.transport_stream_id;
                if (tp->network_id == 0) tp->network_id = ltp.network_id;
            }
        }
    }

    Order_In_Full_t max_order_in_full = lineup->get_max_order_in_full();
    Order_In_Favorite_t max_order_in_favorite = lineup->get_max_order_in_favorite();

    // Add each service to current lineup
    for (const auto &svc : m_services)
    {
        auto it = std::find_if(lineup->services.begin(), lineup->services.end(),
                               [&](const auto & s)
        {
            return s.service_id() == svc.service_id() &&
                   s.transponder_id() == svc.transponder_id() &&
                   s.satellite_id() == svc.satellite_id();
        });

        if (it == lineup->services.end()) {
            lineup->services.emplace_back(svc.transponder_id(), svc.service_id());
            it = lineup->services.end() - 1;
            it->set_satellite_id(svc.satellite_id());
            it->set_is_favorite(true);
            it->set_regionalizacao(Regionalizacao::NaoRegionalizado);
        }

        it->set_name(svc.name());
        it->set_service_type(svc.service_type());
        it->set_pcr_pid(svc.pcr_pid());
        it->set_video_pid(svc.video_pid());
        it->set_video_codec(svc.video_codec());
        it->set_pmt_pid(svc.pmt_pid());
        it->set_epg_pid(svc.epg_pid());
        it->set_ca_types(std::vector<CA_Info>(svc.ca_types()));
        it->clear_audio_pids();
        for (const auto& ap : svc.audio_pids()) {
            it->add_audio_pid(ap);
        }
        it->sort_audio_pids();
        it->set_subtitle_pid(svc.subtitle_pid());
        it->set_dvb_subtitle_present(svc.has_dvb_subtitle());
        it->set_dvb_subtitle(std::vector<DVB_Subtitle_Info>(svc.dvb_subtitle()));

        if (svc.regionalizacao() != Regionalizacao::Undefined) {
            it->set_regionalizacao(svc.regionalizacao());
        }

        if (svc.viewer_channel() != 0) {
            it->set_viewer_channel(svc.viewer_channel());
        } else if (it->viewer_channel() == 0) {
            it->set_viewer_channel(static_cast<Viewer_Channel_t>(viewer_channel++));
        }

        if (it->viewer_channel() >= viewer_channel) {
            viewer_channel = it->viewer_channel() + 1;
        }

        uint64_t order = (static_cast<uint64_t>(it->satellite_id()) << 48u) + (static_cast<uint64_t>(it->viewer_channel()) << 16u) + static_cast<uint32_t>(it->service_id());

        if (it->get_order_in_full() == 0)
        {
            it->set_order_in_full(order);
        }

        if (it->get_order_in_favorite() == 0)
        {
            it->set_order_in_favorite(order);
        }
    }

    lineup->filter_lineup();
    Task::post_event_lineup_save();

    if (auto evt = m_event_callback.lock())
    {
        if (evt->callback)
        {
            evt->callback(true);
        }
    }
}

void Demux_Channel_List::start()
{
    if (auto evt = m_event_callback.lock())
    {
        m_transponders = evt->transponders;

        auto lineup = Lineup_Mutex_Ref::get_current_lineup();
        auto config = Config::get_config();
        auto current_sat_id = config->get_current_satellite();
        bool is_sky = (current_sat_id == 2);
        bool is_claro = (current_sat_id == 1);

        bool has_sky = evt->has_sky || std::any_of(lineup->services.begin(), lineup->services.end(), [](const auto& s) { return s.satellite_id() == 2; });
        bool has_claro = evt->has_claro || std::any_of(lineup->services.begin(), lineup->services.end(), [](const auto& s) { return s.satellite_id() == 1; });

        if (is_sky)
        {
            m_base_viewer_channel = 0;
        }
        else if (is_claro)
        {
            m_base_viewer_channel = has_sky ? 10000 : 0;
        }
        else // Generic
        {
            if (has_sky || has_claro)
            {
                m_base_viewer_channel = std::max<Viewer_Channel_t>(10000, lineup->get_max_viewer_channel());
            }
            else
            {
                m_base_viewer_channel = lineup->get_max_viewer_channel();
            }
        }
        DEBUG_MSG(DEMUX, INFO, "Channel List Session: Count=" << evt->scan_sat_count << " Index=" << evt->scan_sat_index << " BaseChannel=" << m_base_viewer_channel << "\n");
    }

    if (m_transponders.empty())
    {
        auto [tp_id, is_locked] = Task_Tuner::get_current_transponder();
        auto signal_info = Task_Tuner::get_signal_info();
        Transponder tp;
        tp.transponder_id = tp_id;
        tp.symbol_rate = signal_info.symbol_rate;
        tp.dvb_mode = signal_info.signal_type;
        tp.set_satellite_id(Config::get_config()->get_current_satellite());
        m_transponders.push_back(tp);
    }

    m_current_tp_idx = 0;
    start_tp();
}

void Demux_Channel_List::start_tp()
{
    if (m_current_tp_idx >= m_transponders.size())
    {
        return;
    }

    m_current_tp_id = m_transponders[m_current_tp_idx].transponder_id;
    DEBUG_MSG(DEMUX, INFO, "Scanning TP " << (m_current_tp_idx + 1) << "/" << m_transponders.size()
              << " frequency " << dec << m_current_tp_id.frequency() << "\n");

    m_sdt_done = false;
    m_pmt_done = false;
    m_tp_locked = false;
    m_tp_done = false;
    m_pending_pmts = 0;
    m_onid = 0;
    m_tsid = 0;

    m_requested_pmt_programs.clear();
    m_parsed_pmt_programs.clear();
    m_parsed_sdt_services.clear();
    m_pmt_pids.clear();
    m_deferred_pmts.clear();
    m_seen_sdt_sections.clear();
    m_sdt_last_section.reset();

    Task::post_event_transponder_lock(POST_CALLER &m_transponders[m_current_tp_idx]);
    m_tp_started = std::chrono::steady_clock::now();
}

void Demux_Channel_List::finish_tp()
{
    m_demux->clear_pat_callback();
    m_demux->clear_sdt_callback();
    m_demux->clear_pmt_callback();

    // Clear need tables
    auto pat_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(PAT_TABLE_ID);
    m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(PAT) PAT_TSPID, pat_table_id, Demux::Data_Type::SECTION);

    auto sdt_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
    m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id, Demux::Data_Type::SECTION);
}

void Demux_Channel_List::process()
{
    if (is_done())
    {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (m_tp_locked)
    {
        if (m_sdt_done && m_pmt_done)
        {
            DEBUG_MSG(DEMUX, INFO, "TP " << m_current_tp_id.frequency() << " done\n");

            // Update current transponder info and add to locked list
            auto &tp = m_transponders[m_current_tp_idx];
            tp.original_network_id = m_onid;
            tp.transport_stream_id = m_tsid;
            if (tp.network_id == 0) tp.network_id = Config::get_config()->selected_satellite_config().network_id;
            m_locked_transponders.push_back(tp);

            m_tp_done = true;
        }
    }

    if (!m_tp_done && (now - m_tp_started) > 15s)
    {
        DEBUG_MSG(DEMUX, WARN, "TP " << m_current_tp_id.frequency() << " timeout\n");
        m_tp_done = true;
    }

   if (m_tp_done)
    {
        finish_tp();
        m_current_tp_idx++;
        maybe_emit_partial_update();
        if (!is_done())
        {
            start_tp();
        }
    }
}

void Demux_Channel_List::transponder_lock(const Event_Tuner_Lock &_event)
{
    if (is_done()) return;

    if (_event.success && _event.tp == m_current_tp_id)
    {
        DEBUG_MSG(DEMUX, INFO, "TP " << m_current_tp_id.frequency() << " locked\n");
        m_tp_locked = true;
        setup_filters();
    }
    else
    {
        DEBUG_MSG(DEMUX, WARN, "TP " << m_current_tp_id.frequency() << " failed to lock\n");
        m_tp_done = true;
    }
}

void Demux_Channel_List::setup_filters()
{
    using namespace std::placeholders;
    m_demux->clear_pat_callback();
    m_demux->set_pat_callback(std::bind(&Demux_Channel_List::pat_callback, this, _1, _2));

    m_demux->clear_sdt_callback();
    m_demux->set_sdt_callback(std::bind(&Demux_Channel_List::sdt_callback, this, _1, _2));

    m_task_demux->pat_table_require();

    auto sdt_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
    m_task_demux->table_require(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id);
}

void Demux_Channel_List::maybe_emit_partial_update()
{
    if (auto evt = m_event_callback.lock())
    {
        if (evt->partial_callback)
        {
            evt->partial_callback(m_current_tp_idx, m_services);
        }
    }
}

void Demux_Channel_List::pat_callback(PID_t _pid, PAT &&_pat)
{
    DEBUG_MSG(DEMUX, DEBUG, "Channel List: PAT received\n");

    auto programs = _pat.move_programs();
    for (const auto &prg : programs)
    {
        if (prg.program != 0 && prg.pid > 0)
        {
            m_pmt_pids[prg.program] = prg.pid;

            // If the service was already parsed from SDT, update its PMT PID
            auto it = std::find_if(m_services.begin(), m_services.end(),
                                   [&](const auto & svc)
            {
                return svc.service_id() == prg.program && svc.transponder_id() == m_current_tp_id;
            });

            if (it != m_services.end())
            {
                it->set_pmt_pid(prg.pid);
            }

            if (m_requested_pmt_programs.insert(prg.program).second)
            {
                m_pending_pmts++;
                DEBUG_MSG(DEMUX, DEBUG, "Channel List: PAT program "
                          << dec << prg.program << " PID " << prg.pid << "\n");

                m_task_demux->pmt_table_require(prg.pid, prg.program);
            }
        }
    }

    if (m_pending_pmts == 0)
    {
        m_pmt_done = true;
    }

    auto pat_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(PAT_TABLE_ID);
    m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(PAT) _pid, pat_table_id, Demux::Data_Type::SECTION);
    m_demux->clear_pat_callback();
}

void Demux_Channel_List::sdt_callback(PID_t _pid, SDT &&_sdt)
{
    const auto section_number = _sdt.section_number();
    const auto last_section_number = _sdt.last_section_number();

    DEBUG_MSG(DEMUX, DEBUG, "Channel List: SDT received, table_id=0x"
              << hex << (int)_sdt.table_id()
              << " section " << dec << (int)section_number
              << "/" << (int)last_section_number << "\n");

    parse_sdt(std::move(_sdt));

    m_seen_sdt_sections.insert(section_number);
    m_sdt_last_section = last_section_number;

    bool all_sections_received = false;
    if (m_sdt_last_section.has_value())
    {
        all_sections_received = true;
        for (uint16_t i = 0; i <= m_sdt_last_section.value(); ++i)
        {
            if (m_seen_sdt_sections.count(static_cast<uint8_t>(i)) == 0)
            {
                all_sections_received = false;
                break;
            }
        }
    }

    if (!all_sections_received)
    {
        return;
    }

    m_sdt_done = true;

    auto sdt_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
    m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(SDT) _pid, sdt_table_id, Demux::Data_Type::SECTION);
    m_demux->clear_sdt_callback();
}

void Demux_Channel_List::pmt_callback(PID_t _pid, PMT &&_pmt)
{
    auto program = _pmt.program_number();

    DEBUG_MSG(DEMUX, DEBUG, "Channel List: PMT received for program "
              << dec << program << "\n");

    parse_pmt(std::move(_pmt));

    if (m_parsed_pmt_programs.insert(program).second)
    {
        if (m_pending_pmts > 0)
        {
            m_pending_pmts--;
        }
    }

    auto pmt_table_id = std::make_shared<Demux::TS_Filter_PMT>(PMT_TABLE_ID, program);
    m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(PMT) _pid, pmt_table_id, Demux::Data_Type::SECTION);

    if (m_parsed_pmt_programs.size() >= m_requested_pmt_programs.size() && !m_requested_pmt_programs.empty())
    {
        m_pmt_done = true;
    }
}

void Demux_Channel_List::parse_sdt(SDT &&_sdt)
{
    m_onid = _sdt.original_network_id();
    m_tsid = _sdt.transport_stream_id();

    auto transport_streams = _sdt.move_transport_streams();

    for (auto &ts : transport_streams)
    {
        auto basic_type = to_basic_type(ts.service_type);

        if (basic_type != Basic_Service_Type::TV && basic_type != Basic_Service_Type::Radio)
        {
            continue;
        }

        // Check if we already have this service
        auto it = std::find_if(m_services.begin(), m_services.end(),
                               [&](const auto &svc)
        {
            return svc.service_id() == ts.service_id && svc.transponder_id() == m_current_tp_id;
        });

        if (it == m_services.end())
        {
            // Create new service
            m_services.emplace_back(m_current_tp_id, ts.service_id);
            it = m_services.end() - 1;
            it->set_satellite_id(m_current_tp_id.satellite_id());
        }

        auto pmt_it = m_pmt_pids.find(ts.service_id);
        if (pmt_it != m_pmt_pids.end())
        {
            it->set_pmt_pid(pmt_it->second);
        }

        if (it->name().empty())
        {
            it->set_name(ts.move_service_name());
        }

        if (it->service_type() == Service_Type::none)
        {
            it->set_service_type(ts.service_type);
        }

        auto deferred_it = m_deferred_pmts.find(ts.service_id);
        if (deferred_it != m_deferred_pmts.end())
        {
            DEBUG_MSG(DEMUX, DEBUG, "Channel List SDT: applying deferred PMT for service "
                      << dec << ts.service_id << "\n");
            parse_pmt(std::move(deferred_it->second));
            m_deferred_pmts.erase(deferred_it);
        }

        for(auto &ld : ts._linkage_descriptors)
        {
            if(ld.linkage_type() == Linkage_Descriptor::Linkage_Type::epg_service)
            {
                if(it->epg_pid() == 0)
                {
                    it->set_epg_pid(ld.service_id());
                }
            }
        }
    }
}

void Demux_Channel_List::parse_pmt(PMT &&_pmt)
{
    auto program_number = _pmt.program_number();
    auto it = std::find_if(m_services.begin(), m_services.end(),
                           [&](const auto &svc)
    {
        return svc.service_id() == program_number && svc.transponder_id() == m_current_tp_id;
    });

    // PMT can arrive before SDT; defer until service classification is known.
    if (it == m_services.end())
    {
        m_deferred_pmts[program_number] = std::move(_pmt);
        return;
    }

    auto streams = _pmt.move_streams();

    it->set_pcr_pid(_pmt.pcr_pid());
    it->set_ca_types(_pmt.move_ca_types());
    it->clear_audio_pids();

    for (auto &str : streams)
    {
        if (str.dvb_subtitle_present)
        {
            it->set_subtitle_pid(str.pid);
            it->set_dvb_subtitle_present(str.dvb_subtitle_present);
            it->set_dvb_subtitle(_pmt.move_dvb_subtitles());
        }

        if (str.regionalizacao > Regionalizacao::Undefined)
        {
            it->set_regionalizacao(str.regionalizacao);
        }

        if (str.viewer_channel)
        {
            it->set_viewer_channel(str.viewer_channel + m_base_viewer_channel);
        }

        auto add_audio{[&](uint8_t stream_type)
        {
            auto codec = audio_codec_from_stream_type(stream_type);

            if (codec == Audio_Codec::AC3)
            {
                return;
            }

            auto pidit = std::find_if(it->audio_pids().begin(), it->audio_pids().end(),
                                      [&str](const auto &a)
            {
                return a.pid == str.pid;
            });

            if (pidit == it->audio_pids().end())
            {
                it->add_audio_pid(str.pid, str.language_code, codec);
            }
        }};

        switch (str.stream_type)
        {
            case STREAM_TYPE_11172_AUDIO:
            case STREAM_TYPE_13818_AUDIO:
            case STREAM_TYPE_14496_3_AUDIO:
            case STREAM_TYPE_DTS_AUDIO:
            case STREAM_TYPE_13818_7_AUDIO:
            {
                add_audio(str.stream_type);
                break;
            }

            case STREAM_TYPE_PRIVATE:
            {
                switch (str.component_tag)
                {
                    case 0x01:
                    case 0x02:
                    case 0x05:
                    case 0x40:
                    case 0x41:
                    case 0x43:
                    case 0x44:
                        if (str.stream_sub_type != 0)
                        {
                            add_audio(str.stream_sub_type);
                        }
                        else
                        {
                            add_audio(STREAM_TYPE_14496_3_AUDIO);
                        }
                        break;
                }
                break;
            }

            case STREAM_TYPE_11172_VIDEO:
            case STREAM_TYPE_13818_VIDEO:
                it->set_video_pid(str.pid);
                it->set_video_codec(Video_Codec::MPEG2);
                break;

            case STREAM_TYPE_14496_2_VIDEO:
                it->set_video_pid(str.pid);
                it->set_video_codec(Video_Codec::MPEG4);
                break;

            case STREAM_TYPE_14496_10_VIDEO:
            case STREAM_TYPE_H264_VIDEO:
                it->set_video_pid(str.pid);
                it->set_video_codec(Video_Codec::H264);
                break;

            case STREAM_TYPE_HEVC_VIDEO:
                it->set_video_pid(str.pid);
                it->set_video_codec(Video_Codec::HEVC);
                break;
        }
    }

    it->sort_audio_pids();
}

} // namespace mb
