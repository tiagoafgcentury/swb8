#include "mb_demux_lineup.h"
#include "common/mb_globals.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_demux.h"
#include "mb_zone_id.h"
#include "common/mb_config.h"
#include "hal/mb_system.h"
#include "tasks/mb_task_tuner.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <map>
#include <limits>
#include <unordered_set>

using namespace std::chrono_literals;

namespace mb {

Transponder &operator<<(Transponder &tp, const Satellite_Delivery_System_Descriptor &sat)
{
    tp.transponder_id.set_frequency(sat.frequency(), from_dvb_standard_polarity(sat.polarization()), tp.satellite_id);
    tp.symbol_rate = sat.symbol_rate();
    tp.dvb_mode = sat.modulation_system() == 0 ? DVB_Mode::DVBS : DVB_Mode::DVBS2;
    return tp;
}

Transponder &operator<<(Transponder &tp, const S2_Satellite_Delivery_System_Descriptor &sat)
{
    tp.transponder_id.set_frequency(sat.frequency(), from_dvb_standard_polarity(sat.polarization()), tp.satellite_id);
    tp.symbol_rate = sat.symbol_rate();
    tp.dvb_mode = DVB_Mode::DVBS2;
    return tp;
}

Demux_Lineup::Demux_Lineup(Task_Demux *_task_demux):
    lineup_started(decltype(lineup_started)::clock::now()),
    tp_started(lineup_started),
    m_task_demux(_task_demux)
{
    auto demux = get_demux();
    using namespace std::placeholders;
    demux->clear_bat_callback();
    demux->clear_nit_callback();
    demux->clear_sdt_callback();
    demux->set_bat_callback(std::bind(&Demux_Lineup::bat_callback, this, _1, _2));
    demux->set_nit_callback(std::bind(&Demux_Lineup::nit_callback, this, _1, _2));
    demux->set_sdt_callback(std::bind(&Demux_Lineup::sdt_callback, this, _1, _2));
}

Demux_Lineup::~Demux_Lineup()
{
    auto demux = get_demux();
    demux->clear_bat_callback();
    demux->clear_nit_callback();
    demux->clear_sdt_callback();
}

Demux *Demux_Lineup::get_demux()
{
    return m_task_demux->get_demux();
}

void Demux_Lineup::transponder_lock(const Event_Tuner_Lock &_event)
{
    if(_event.success)
    {
        auto config = Config::get_config();
        const auto &sat_config = config->selected_satellite_config();
        const auto tp = &(start_list[start_list_idx]);

        if (lineup->get_transponder(tp->transponder_id) == nullptr)
        {
            lineup->transponders.push_back(*tp);
        }

        m_task_demux->set_state(Task_Demux::ST_LINEUP_RUNNING);
        tp_started = decltype(tp_started)::clock::now();
        DEBUG_MSG(DEMUX, DEBUG, "Current TSID: " << dec << tp->transport_stream_id <<
                  "\nCurrent NID: " << tp->network_id <<
                  "\nCurrent ONID: " << tp->original_network_id <<
                  endl
                 );
                locked_frequencies.emplace(tp->transponder_id);

        if(tp->is_home_channel)
        {
            done_tables = 0;
            if(sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Sky))
            {
                const auto my_zone_id = Zone_ID::get_zone_id(Satellite_Operator::Sky);
                auto bouquet_id_customer = BOUQUET_ID + my_zone_id;

                if(bouquet_id_customer > BOUQUET_ID)
                {
                    auto bat_table_id_customer = std::make_shared<Demux::TS_Filter_BAT_Bouquet_Id>(BAT_TABLE_ID, bouquet_id_customer);
                    m_task_demux->table_require(TABLE_REQUIRE_NAME(BAT) BAT_TSPID, bat_table_id_customer);
                }
                else
                {
                    auto bat_table_id = std::make_shared<Demux::TS_Filter_BAT_Bouquet_Id>(BAT_TABLE_ID, BOUQUET_ID);
                    m_task_demux->table_require(TABLE_REQUIRE_NAME(BAT) BAT_TSPID, bat_table_id);
                }

                auto nit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(NIT_TABLE_ID_ACTUAL);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(NIT) NIT_TSPID, nit_table_id);


                auto sdt_table_id_actual = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_actual);
                auto sdt_table_id_other = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_OTHER);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_other);
            }
            else if(sat_config.network_policies == static_cast<uint32_t>(Network_Policies::TVRO))
            {
                auto nit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(NIT_TABLE_ID_ACTUAL);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(NIT) NIT_TSPID, nit_table_id);
                auto sdt_table_id_actual = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_actual);
                auto sdt_table_id_other = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_OTHER);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_other);
                auto bat_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(BAT_TABLE_ID);
                m_task_demux->table_require(TABLE_REQUIRE_NAME(BAT) BAT_TSPID, bat_table_id);
            }
            else
            {
                DEBUG_MSG(DEMUX, ERROR, "MISSING SAT POLICIES!\n");
            }
        }
        else if(sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Generic))
        {
            // Generic uses only PAT + PMT + SDT. NIT and BAT are ignored.
            done_tables = 0;
            DEBUG_MSG(DEMUX, INFO, "[Generic] TP locked freq="
                      << dec << tp->transponder_id.frequency() / 1000
                      << " kHz — requesting PAT + SDT-actual\n");
            // Request PAT to discover program PIDs and then PMTs.
            m_task_demux->pat_table_require();
            // Request SDT-actual to get service names and types.
            auto sdt_table_id_actual = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
            m_task_demux->table_require(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_actual);
        }        
        else
        {
            done_tables = ptNIT | ptBAT;
        }
    }
    else
    {
        const auto tp = &(start_list[start_list_idx]);
        failed_frequencies.emplace(tp->transponder_id);
        auto &transponders = lineup->transponders;
        auto it = std::find(transponders.begin(), transponders.end(), _event.tp);

        if(it != transponders.end())
        {
            transponders.erase(it);
        }

        finish_tp();
    }
}

void Demux_Lineup::start_tp()
{
   if (start_list_idx == 0)
    {
        if (auto evt = event_callback.lock())
        {
            m_scan_sat_count = evt->scan_sat_count;
            m_scan_sat_index = evt->scan_sat_index;

            bool is_sky = (m_current_satellite_id == 2);
            bool is_claro = (m_current_satellite_id == 1);

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
            DEBUG_MSG(DEMUX, INFO, "Scan Session: Count=" << m_scan_sat_count << " Index=" << m_scan_sat_index << " BaseChannel=" << m_base_viewer_channel << "\n");
        }
    }

    m_task_demux->clear_table_queue();
    m_process_passes_with_no_tables = 0;
    tp_started = decltype(tp_started)::clock::now();

NEXT_TP:
    if(start_list_idx >= start_list.size())
    {
        m_task_demux->set_state(Task_Demux::ST_LINEUP_FINISHED);
        return;
    }

    const auto tp = &(start_list[start_list_idx]);
    if (tp) {
        if(locked_frequencies.count({tp->transponder_id}) or
           failed_frequencies.count({tp->transponder_id}))
        {
            start_list_idx++;
            goto NEXT_TP;
        }

        DEBUG_MSG(DEMUX, INFO, "Locking: " << tp->transponder_id.satellite_id() << "/" << tp->transponder_id.frequency() << "/" << tp->symbol_rate << "\n");
        Task::post_event_transponder_lock(POST_CALLER tp);
        if (auto event = event_callback.lock(); event && event->partial_callback)
        {
            auto progress = start_list_idx * 100 / start_list.size();
            DEBUG_MSG(DEMUX, ERROR, "Progress: " << progress << "%\n");
            event->partial_callback(progress, lineup->services);
        }
        m_task_demux->set_state(Task_Demux::ST_LOCKING);
    }
    else {
        DEBUG_MSG(DEMUX, ERROR, "No more transponders to lock\n");
        m_task_demux->set_state(Task_Demux::ST_LINEUP_FINISHED);
    }
}

void Demux_Lineup::finish_tp()
{
    const auto tp = start_list_idx < start_list.size() ? &(start_list[start_list_idx]) : nullptr;
    DEBUG_MSG(DEMUX, INFO, "[Lineup] finish_tp: freq="
              << dec << (tp ? tp->transponder_id.frequency() / 1000 : 0)
              << " kHz  services_in_lineup=" << (lineup ? lineup->services.size() : 0) << "\n");
     if (m_task_demux->m_table_queue.size() != 0)
    {
        DEBUG_MSG(DEMUX, WARN, "Clearing table queue in finish_tp (" << m_task_demux->m_table_queue.size() << " items)\n");
        m_task_demux->clear_table_queue();
    }
    parse_tables(lineup);

    if(m_task_demux->state() == Task_Demux::ST_PAT_UPDATING && done_tables & ptPMT)
    {
        m_task_demux->set_state(Task_Demux::ST_IDLE);
    }

    bats.clear();
    pmts.clear();
    sdts.clear();

    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    if(sat_config.read_all_tps)
    {
        for(const auto &tp : lineup->transponders)
        {
            if(locked_frequencies.count({tp.transponder_id}) == 0 and
               failed_frequencies.count({tp.transponder_id}) == 0)
            {
                auto it = std::find(start_list.begin(), start_list.end(), tp);
                if (it == start_list.end())
                {
                    Transponder copy;
                    copy.transponder_id = tp.transponder_id;
                    copy.symbol_rate = tp.symbol_rate;
                    copy.dvb_mode = tp.dvb_mode;
                    copy.transport_stream_id = tp.transport_stream_id;
                    copy.original_network_id = tp.original_network_id;
                    copy.network_id = tp.network_id;
                    copy.is_home_channel = tp.is_home_channel;
                    copy.satellite_id = tp.satellite_id;
                    start_list.push_back(std::move(copy));
                }
            }
        }
    }

    start_list_idx++;
    start_tp();
}

void Demux_Lineup::process()
{
    parse_tables(lineup);
    maybe_emit_partial_update();

    if(m_task_demux->state() == Task_Demux::ST_PAT_UPDATING && done_tables & ptPMT)
    {
        m_task_demux->set_state(Task_Demux::ST_IDLE);
    }

    auto now = decltype(lineup_started)::clock::now();

    if(now - lineup_started > 15min)
    {
        m_task_demux->set_state(Task_Demux::ST_LINEUP_FINISHED);
    }


    if(m_task_demux->state() == Task_Demux::ST_LINEUP_RUNNING && (now - tp_started) > 15s)
    {
        const auto tp = start_list_idx < start_list.size() ? &(start_list[start_list_idx]) : nullptr;
        DEBUG_MSG(DEMUX, WARN, "[Lineup] TP timeout: freq="
                  << dec << (tp ? tp->transponder_id.frequency() / 1000 : 0)
                  << " kHz  done_tables=0x" << hex << done_tables
                  << "  pmts_pending=" << dec << pmts.size() << "\n");
        m_task_demux->clear_table_queue();
        finish_tp();
        return;
    }

    if(m_task_demux->m_table_queue.size() == 0)
    {
        // Since tables might have been post'ed and
        // the handle has not been processed yet, we
        // have to make sure that all events have
        // been processed before we can finish
        if(m_process_passes_with_no_tables++ > 3)
        {
            finish_tp();
        }
    }
    else
    {
        m_process_passes_with_no_tables = 0;
    }

}

void Demux_Lineup::maybe_emit_partial_update()
{
    if(!lineup)
    {
        return;
    }

    const auto current_size = lineup->services.size();
    if(current_size == m_last_partial_services_size)
    {
        return;
    }

    m_last_partial_services_size = current_size;

    if(auto event = event_callback.lock(); event && event->partial_callback)
    {
        const size_t transponder_seq = start_list.empty() ? 0u : (start_list_idx + 1u);
        event->partial_callback(transponder_seq, lineup->services);
    }
}

void Demux_Lineup::finish()
{ 
    auto total_time = decltype(lineup_started)::clock::now() - lineup_started;
    DEBUG_MSG(DEMUX, DEBUG, "Lineup done in " << round(std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count() / 100) / 10 << "s\n");
    m_task_demux->set_state(Task_Demux::ST_IDLE);

    bool should_emit = true;
    if(auto event = event_callback.lock(); event)
    {
        should_emit = event->emit_lineup_ready;
    }

    if(should_emit)
    {
        DEBUG_MSG(DEMUX, INFO, "m_restart_after_lock: " << m_task_demux->m_restart_after_lock << "\n");
        Task::post_event_lineup_changed();
        Task::post_event_lineup_ready(
        {
            .origin = Lineup_Origin::LO_SATELLITE,
            .duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_time),
            .restart = m_task_demux->m_restart_after_lock
        });
    }

    if(auto event = event_callback.lock(); event)
    {
        if(event->callback)
        {
            DEBUG_MSG(DEMUX, DEBUG, "Calling lineup ready callback\n");
            event->callback(true);
        }
    }

    m_eit_services_sections.clear();
    m_service_filter.clear();
}

void Demux_Lineup::bat_callback(PID_t _pid, BAT &&_bat)
{
    auto bouquet_id = _bat.bouquet_id();
    DEBUG_MSG(DEMUX, DEBUG, "Got BAT " << dec << bouquet_id << " " << _bat.peek_bouquet_name() << " " <<
                (int)_bat.section_number() << "/" << (int)_bat.last_section_number() << "\n");

    const auto &it = bats.find(_bat.bouquet_id());
    if(it == bats.end())
    {
        goto PARSE_BAT;
    }

    if(it->second.empty())
    {
        goto PARSE_BAT;
    }

    if(not it->second[_bat.section_number()].has_value())
    {
        goto PARSE_BAT;
    }

    // We have altrady parsed this BAT/section
    goto DONE_PARSED_BAT;

PARSE_BAT:
    bats.generic_table_callback(std::move(_bat));

DONE_PARSED_BAT:
    auto bats_is_done = check_bats_is_done(bouquet_id);
    if(bats_is_done)
    {
        done_tables |= ptBAT;
        auto bat_id = std::make_shared<Demux::TS_Filter_Table_Id>(BAT_TABLE_ID);
        m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(BAT) _pid, bat_id, Demux::Data_Type::SECTION);
    }
}

void Demux_Lineup::pat_program_callback(const PAT::Program &_program)
{
    DEBUG_MSG(DEMUX, DEBUG, "PAT Program: " << dec << _program.program << " PID: " << _program.pid << "\n");
    if(lineup)
    {
        auto [tp_id, is_locked] = Task_Tuner::get_current_transponder();
        for(auto &s : lineup->services)
        {
            if(s.service_id() == _program.program && s.transponder_id() == tp_id)
            //if(s.service_id() == _program.program)
            {
                s.set_pmt_pid(_program.pid);
                break;
            }
        }
    }
}

void Demux_Lineup::nit_callback(PID_t pid, NIT &&_nit)
{
    auto table_id = _nit.table_id();
    auto section_number = _nit.section_number();
    auto last_section_number = _nit.last_section_number();

    if(lineup)
    {
        parse_nit(std::move(_nit), lineup);
    }

    if(nit_sections.empty())
    {
        nit_sections.resize(last_section_number + 1);
    }

    nit_sections[section_number] = true;

    for(auto b : nit_sections)
    {
        if(!b)
        {
            return;
        }
    }

    done_tables |= ptNIT;
    auto nit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(table_id);
    m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(NIT) pid, nit_table_id, Demux::Data_Type::SECTION);
}

void Demux_Lineup::parse_pmt(PMT &&_pmt, Lineup *_lineup)
{
    auto [current_tp, _] = Task_Tuner::get_current_transponder();
    basic_parse_pmt(std::move(_pmt), _lineup, current_tp, m_base_viewer_channel);
}

void Demux_Lineup::sdt_callback(PID_t /*pid*/, SDT &&_sdt)
{
    sdts.generic_table_callback(std::move(_sdt));
    if(check_sdts_is_done())
    {
        done_tables |= ptSDT;
        auto sdt_table_id_actual = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
        m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_actual, Demux::Data_Type::SECTION);
        auto sdt_table_id_other = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_OTHER);
        m_task_demux->remove_need_table(TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_table_id_other, Demux::Data_Type::SECTION);
    }
}

void Demux_Lineup::check_sdt_ts(TS_ID_t _ts_id)
{
    if (not sdts.contains(_ts_id))
    {
        DEBUG_MSG(DEMUX, DEBUG, "SDT Require TS: " << dec << _ts_id << "\n");
        sdts[_ts_id] = {};
    }
}

void Demux_Lineup::basic_parse_pmt(PMT &&_pmt, Lineup *_lineup, Transponder_Id _tp_id, Viewer_Channel_t _base_viewer_channel)
{
    auto streams = _pmt.move_streams();

    for(auto &str : streams)
    {
        switch(str.stream_type)
        {
            case STREAM_TYPE_11172_AUDIO: // MPEG 1
            case STREAM_TYPE_13818_AUDIO: // MPEG 20
            case STREAM_TYPE_14496_3_AUDIO: // AAC
            case STREAM_TYPE_DTS_AUDIO:
            case STREAM_TYPE_13818_7_AUDIO:
            {
                goto FOUND_USEFULL_STREAM;
            }

            case STREAM_TYPE_PRIVATE_DATA:
                break;

            case STREAM_TYPE_PRIVATE:
                switch(str.component_tag)
                {
                    case 0x00:
                    case 0x01:
                    case 0x02:
                    case 0x05:
                    case 0x40:
                    case 0x41:
                    case 0x43:
                    case 0x44:
                        goto FOUND_USEFULL_STREAM;

                    case 0x03:
                    default:
                        DEBUG_MSG(DEMUX, DEBUG, "Audio type: " << dec << (int)str.component_tag << "\n");
                }

                break;

            case SYSTEM_CLOCK_DESCRIPTOR:
                break;

            case STREAM_TYPE_11172_VIDEO:
            case STREAM_TYPE_13818_VIDEO:
            case STREAM_TYPE_14496_2_VIDEO:
            case STREAM_TYPE_14496_10_VIDEO:
            case STREAM_TYPE_H264_VIDEO:
            case STREAM_TYPE_HEVC_VIDEO:
                goto FOUND_USEFULL_STREAM;

            case STREAM_TYPE_AC3_AUDIO:
            default:
                break;
        }
    }

    DEBUG_MSG(DEMUX, DEBUG, "PMT Ignored Program: 0x" << dec << setfill('0') << setw(4) << (int)_pmt.program_number() << "\n");
    return; // no usefull stream found
FOUND_USEFULL_STREAM:
    auto it = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                           [&](const auto & s)
    {
                return s.service_id() == _pmt.program_number() && s.transponder_id() == _tp_id;
    });

    if(it == _lineup->services.end())
    {
        DEBUG_MSG(DEMUX, WARN, "[Lineup][PMT] program=0x" << hex << setfill('0') << setw(4)
                  << (int)_pmt.program_number()
                  << " NOT FOUND in lineup for tp=" << dec << _tp_id.frequency() / 1000
                  << " kHz (ONID match needed? check if SDT pre-added it)\n");
        return;
    }

    it->set_pcr_pid(_pmt.pcr_pid());
    it->set_ca_types(_pmt.move_ca_types());
    it->clear_audio_pids();

    for(auto &str : streams)
    {
        if(str.dvb_subtitle_present)
        {
            it->set_subtitle_pid(str.pid);
            it->set_dvb_subtitle_present(str.dvb_subtitle_present);
            it->set_dvb_subtitle(_pmt.move_dvb_subtitles());
        }

        if(str.regionalizacao > Regionalizacao::Undefined)
        {
            it->set_regionalizacao(str.regionalizacao);
        }

        if(str.viewer_channel)
        {
            it->set_viewer_channel(str.viewer_channel + _base_viewer_channel);
        }

        auto add_audio {[&](uint8_t stream_type)
        {
            auto codec = audio_codec_from_stream_type(stream_type);

            if(codec == Audio_Codec::AC3)
            {
                return;
            }

            auto pidit = std::find_if(it->audio_pids().begin(), it->audio_pids().end(),
                                      [&str](const auto & a)
            {
                return a.pid == str.pid;
            });

            if(pidit == it->audio_pids().end())
            {
                it->add_audio_pid(str.pid, str.language_code, codec);
            }
            else
            {
                if(pidit->codec != codec)
                {
                    DEBUG_MSG(DEMUX, WARN, "Duplicate audio pid: " << str.pid << "\n");
                }
            }
        }};

        switch(str.stream_type)
        {
            case STREAM_TYPE_11172_AUDIO: // MPEG 1
            case STREAM_TYPE_13818_AUDIO: // MPEG 20
            case STREAM_TYPE_14496_3_AUDIO: // AAC
            case STREAM_TYPE_DTS_AUDIO:
            case STREAM_TYPE_13818_7_AUDIO:
            {
                add_audio(str.stream_type);
                break;
            }

            case STREAM_TYPE_PRIVATE_DATA:
                break;

            case STREAM_TYPE_PRIVATE:
            {
                switch(str.component_tag)
                {
                    case 0x01:
                    case 0x02:
                    case 0x05:
                    case 0x40:
                    case 0x41:
                    case 0x43:
                    case 0x44:
                        if(str.stream_sub_type != 0)
                        {
                            add_audio(str.stream_sub_type);
                        }
                        else
                        {
                            add_audio(STREAM_TYPE_14496_3_AUDIO);
                        }

                        break;

                    case 0x03:
                    default:
                        DEBUG_MSG(DEMUX, DEBUG, "Audio type: " << dec << (int)str.component_tag << "\n");
                }

                break;
            }

            case SYSTEM_CLOCK_DESCRIPTOR:
                break;

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

            case STREAM_TYPE_AC3_AUDIO:
            default:
                DEBUG_MSG(DEMUX, WARN, "Unahandled Stream Type: 0x" << hex << setfill('0') << setw(2) << (int)str.stream_type << endl);
                break;
        }
    }

    it->sort_audio_pids();
}

}
