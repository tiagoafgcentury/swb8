#include "mb_demux_lineup_sky.h"
#include "mb_demux_lineup.h"
#include "mb_demux_table_manager.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "hal/mb_demux.h"
#include "hal/mb_tuner.h"
#include "tasks/mb_task_demux.h"
#include "tasks/mb_task_tuner.h"
#include "mb_events.h"
#include "mb_zone_id.h"
#include "common/mb_satellites.h"

#include <algorithm>
#include <chrono>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

using namespace std::chrono_literals;

namespace mb {

Demux_Lineup_Sky::Demux_Lineup_Sky(Task_Demux *_task_demux):
    Demux_Lineup(_task_demux)
{
}

Demux_Lineup_Sky::~Demux_Lineup_Sky()
{
}

void Demux_Lineup_Sky::parse_bat(BAT &&_bat, Lineup *_lineup)
{
    auto bouquet_name { _bat.move_bouquet_name() };
    auto transport_streams { _bat.move_transport_streams() };

    if(sdts.size() < transport_streams.size())
    {
        sdts.reserve(transport_streams.size());
    }

    for(auto &ts : transport_streams)
    {
        auto tp = std::find_if(_lineup->transponders.begin(), _lineup->transponders.end(),
                               [&](const auto & s)
        {
            return s.transport_stream_id == ts.transport_stream_id() && s.satellite_id == m_current_satellite_id;
        });

        if(tp == _lineup->transponders.end())
        {
            DEBUG_MSG(DEMUX, WARN, "BAT: TSID " << dec << ts.transport_stream_id()
                      << " not in lineup transponders yet, skipping BAT services for this TS\n");
            continue;
        }

        auto service_list_descriptors { ts.move_service_list_descriptors().move_services() };
        auto ud_sky_logical_channel_descriptors { ts.move_ud_sky_logical_channel_descriptors().move_services() };
        auto tsid = ts.transport_stream_id();

        for(const auto &sld : service_list_descriptors)
        {
            check_sdt_ts(tsid);
            _lineup->services.emplace_back(tp->transponder_id, sld.service_id);
            auto sit = _lineup->services.begin();
            std::advance(sit, _lineup->services.size() - 1);
            sit->set_satellite_id(Config::get_config()->get_current_satellite());
            DEBUG_MSG(DEMUX, DEBUG, "tsid: " << tsid << " service_id: " << (int)sld.service_id << " SatID: " << (int)sit->satellite_id() << "\n");
        }

        for(auto &lcd : ud_sky_logical_channel_descriptors)
        {
            auto sit = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                                    [&](const auto & service)
            {
                return service.service_id() == lcd.service_id && service.transponder_id() == tp->transponder_id;
            });

            if(sit == _lineup->services.end())
            {
                DEBUG_MSG(DEMUX, WARN, "Service_id not found for descriptor ud_sky_logical_channel. \n");
                break;
            }
            else
            {
                sit->set_viewer_channel(lcd.viewer_channel + m_base_viewer_channel);
                sit->set_bouquet_id(_bat.bouquet_id());
                sit->set_bouquet_name(bouquet_name);
                sit->set_regionalizacao(Regionalizacao::RegionalizadoNacional);
                DEBUG_MSG(DEMUX, DEBUG, "service_id: " << (int)lcd.service_id << " viewer_channel: " << (int)lcd.viewer_channel << "\n");
                lineup->transponders.push_back(*tp);
            }
        }
    } // for (auto &ts : transport_streams)
}

void Demux_Lineup_Sky::parse_nit(NIT &&_nit, Lineup *_lineup)
{
    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    auto transport_streams = _nit.move_transport_streams();
    auto linkage_descriptors = _nit.move_linkage_descriptors();

    if(sat_config.network_id != _nit.network_id())
    {
        DEBUG_MSG(DEMUX, WARN, "Ignore network id: " << dec << _nit.network_id() << "\n");
        return;
    }

    if(sdts.size() < transport_streams.size())
    {
        sdts.reserve(transport_streams.size());
    }

    for(auto &ts : transport_streams)
    {
        auto satellite_delivery_system_descriptor = ts.move_satellite_delivery_system_descriptor();
        if(satellite_delivery_system_descriptor.frequency())
        {
            set_satellite_descriptor(_lineup, _nit, ts, std::move(satellite_delivery_system_descriptor), sat_config);
        }
        else
        {
            auto s2_satellite_delivery_system_descriptor = ts.move_s2_satellite_delivery_system_descriptor();

            if(s2_satellite_delivery_system_descriptor.frequency())
            {
                set_satellite_descriptor(_lineup, _nit, ts, std::move(s2_satellite_delivery_system_descriptor), sat_config);
            }
        }
    }
}

void Demux_Lineup_Sky::parse_sdt(SDT &&_sdt, Lineup *_lineup)
{
    const Transponder *tp { nullptr };
    const bool is_current { _sdt.table_id() == SDT_TABLE_ID_ACTUAL };
    auto transport_streams { _sdt.move_transport_streams() };
    auto [current_transponder, _] = Task_Tuner::get_current_transponder();

    if(is_current)
    {
        tp = _lineup->get_transponder(current_transponder);
        if(not tp)
        {
            DEBUG_MSG(DEMUX, WARN, "'Current' Transponder not found in lineup: " << current_transponder.frequency() / 1000 << "\n");
            return;
        }
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

        DEBUG_MSG(DEMUX, WARN, "SDT TS Not found: " << dec << _sdt.original_network_id() << "/" << _sdt.transport_stream_id() << "\n");

        for(auto &s : transport_streams)
        {
            auto standard_name = s.move_service_name();

            if(standard_name.empty())
            {
                standard_name = s.move_service_provider_name();
            }

            DEBUG_MSG(DEMUX, DEBUG, "Service: SID:" << dec << (int)s.service_id << " " << standard_name << "\n");
        }

        return;
    }

FOUND_TP:
    //bool onid_tsid_mismatch = (_sdt.original_network_id() && _sdt.original_network_id() != tp->original_network_id) ||
    //                          (_sdt.transport_stream_id() && _sdt.transport_stream_id() != tp->transport_stream_id);

    for(auto &s : transport_streams)
    {
        auto sit = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                                [&](const auto & service)
        {
            return service.service_id() == s.service_id && service.transponder_id() == tp->transponder_id;
        });

        if(sit == _lineup->services.end())
        {
            DEBUG_MSG(DEMUX, DEBUG, "Ignore: " << dec << s.service_id << " " << s.move_service_name() << "\n");
            continue;
        }

        if(sit->name().empty())
        {
            sit->set_name(s.move_service_name());
        }

        if(sit->service_type() == Service_Type::none)
        {
            sit->set_service_type(s.service_type);
        }

#ifdef MBGUI_SAT_MONITOR

        if(sit->service_provider_name.empty())
        {
            sit->service_provider_name = s.move_service_provider_name();
        }

        PRINT_DEBUG_MSG("SDT: Service: '" << dec << sit->service_id << " " << sit->name << "' Provider: '" << sit->service_provider_name << "' Type: 0x" << hex << (int)sit->service_type);
#endif
        DEBUG_MSG(DEMUX, DEBUG, "SDT: Service: '" << dec << sit->service_id() << " " << sit->name() << "' Type: 0x" << hex << (int)sit->service_type() << " SatID: " << dec << sit->satellite_id() << "\n");

        for(auto &ld : s._linkage_descriptors)
        {
            switch(ld.linkage_type())
            {
                case Linkage_Descriptor::Linkage_Type::epg_service:
                    if(sit->epg_pid() == 0)
                    {
                        sit->set_epg_pid(ld.service_id());
                    }

                    break;

                default:
                    DEBUG_MSG(DEMUX, WARN, "Linkage not handled: " << dec << tp << " Service ID: " << sit->service_id() <<
                              "\n\tType: "  << (int)ld.linkage_type() <<
                              "\n\tONID: "  << (int)ld.original_network_id() <<
                              "\n\tTSID: "  << (int)ld.transport_stream_id() <<
                              "\n\tSID: "  << (int)ld.service_id() <<
                              endl
                             );
            }
        }
    }
}

bool Demux_Lineup_Sky::check_sdts_is_done()
{
    if ((done_tables & ptBAT) == 0)
    {
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

void Demux_Lineup_Sky::parse_tables(Lineup *_lineup)
{
    using namespace std::placeholders;
#define DO_PARSE(TABLE) \
    do_parse(TABLE ## s, std::bind(&Demux_Lineup_Sky::parse_ ## TABLE, this, _1, _2), _lineup)

    if (done_tables & ptNIT and done_tables & ptBAT)
    {
        DO_PARSE(bat);

        if(done_tables & ptSDT)
        {
            DO_PARSE(sdt);

            if(done_tables & ptPMT)
            {
                DO_PARSE(pmt);
            }
        }
    }

#undef DO_PARSE
}

bool Demux_Lineup_Sky::check_bats_is_done(Bouquet_ID_t)
{
    if(bats.empty())
    {
        return false;
    }

    for(const auto &b : bats)
    {
        if(b.second.empty())
        {
            return false;
        }

        for(const auto &t : b.second)
        {
            if (not t.has_value())
            {
                return false;
            }
        }
    }

    return true;
}

void Demux_Lineup_Sky::transponder_lock(const Event_Tuner_Lock &_event)
{
    Demux_Lineup::transponder_lock(_event);

    if(_event.success)
    {
        m_task_demux->pat_table_require();
    }
}

void Demux_Lineup_Sky::start_tp()
{
    Demux_Lineup::start_tp();
}

void Demux_Lineup_Sky::finish_tp()
{
    auto find_service = [this](Service_ID_t _service_id)
    {
        return std::find_if(lineup->services.begin(), lineup->services.end(), [_service_id](const auto &sit)
        {
            return sit.service_id() == _service_id;
        });
    };

    DEBUG_MSG(DEMUX, DEBUG, "=== finish_tp: TV/Radio classification BEGIN (pmts=" << dec << pmts.size() << ") ===\n");

    for(auto &p : pmts)
    {
        for(auto &s : p.second)
        {
            if(s.has_value())
            {
                const auto prog_num = s.value().table.program_number();
                bool has_video = false;
                auto streams = s.value().table.move_streams();

                // Log SDT-derived service type before PMT analysis
                auto it_pre = find_service(prog_num);
                Service_Type sdt_type = (it_pre != lineup->services.end()) ? it_pre->service_type() : Service_Type::none;
                DEBUG_MSG(DEMUX, DEBUG, "  PMT prog=" << dec << prog_num
                    << " name='" << (it_pre != lineup->services.end() ? it_pre->name() : "<not found>") << "'"
                    << " SDT service_type=0x" << hex << (int)sdt_type
                    << " num_streams=" << dec << streams.size() << "\n");

                for(auto &str : streams)
                {
                    DEBUG_MSG(DEMUX, DEBUG, "    stream_type=0x" << hex << setfill('0') << setw(2) << (int)str.stream_type << "\n");

                    switch(str.stream_type)
                    {
                        case STREAM_TYPE_11172_AUDIO: // MPEG 1
                        case STREAM_TYPE_13818_AUDIO: // MPEG 20
                        case STREAM_TYPE_14496_3_AUDIO: // AAC
                        case STREAM_TYPE_DTS_AUDIO:
                        case STREAM_TYPE_13818_7_AUDIO:
                        case STREAM_TYPE_PRIVATE_DATA:
                        case STREAM_TYPE_PRIVATE:
                        case SYSTEM_CLOCK_DESCRIPTOR:
                            break;

                        case STREAM_TYPE_11172_VIDEO:
                        case STREAM_TYPE_13818_VIDEO:
                        case STREAM_TYPE_14496_2_VIDEO:
                        case STREAM_TYPE_14496_10_VIDEO:
                        case STREAM_TYPE_H264_VIDEO:
                        case STREAM_TYPE_HEVC_VIDEO:
                        case STREAM_TYPE_VC1_VIDEO:
                        case STREAM_TYPE_AVS_VIDEO:
                            has_video = true;
                            DEBUG_MSG(DEMUX, DEBUG, "    -> VIDEO stream found (0x" << hex << setfill('0') << setw(2) << (int)str.stream_type << ") => TV\n");
                            {
                                auto it = find_service(prog_num);
                                if (it != lineup->services.end() && !lineup->is_tv_service(it->service_type()))
                                {
                                    DEBUG_MSG(DEMUX, DEBUG, "  -> prog=" << dec << prog_num
                                        << " name='" << it->name() << "'"
                                        << ": SDT type=0x" << hex << (int)it->service_type()
                                        << " was wrong => correcting to TV\n");
                                    it->set_service_type(Service_Type::digital_television_service);
                                }
                            }
                            goto NEXT_PMT;

                        case STREAM_TYPE_AC3_AUDIO:
                            break;
                        default:
                            DEBUG_MSG(DEMUX, WARN, "    -> Unhandled stream type 0x" << hex << setfill('0') << setw(2) << (int)str.stream_type << " (treated as unknown)\n");
                            break;
                    }
                }

                if (!has_video && streams.size() > 0)
                {
                    auto it = find_service(prog_num);
                    if(it != lineup->services.end())
                    {
                        if(lineup->is_tv_service(it->service_type()))
                        {
                            DEBUG_MSG(DEMUX, DEBUG, "  -> prog=" << dec << prog_num
                                << " name='" << it->name() << "'"
                                << ": no video streams, SDT wrongly said TV (0x" << hex << (int)it->service_type()
                                << ") => correcting to RADIO\n");
                        }
                        else
                        {
                            DEBUG_MSG(DEMUX, DEBUG, "  -> prog=" << dec << prog_num
                                << " name='" << it->name() << "'"
                                << ": no video streams, SDT type=0x" << hex << (int)it->service_type()
                                << " => set as RADIO\n");
                        }
                        it->set_service_type(Service_Type::digital_radio_sound_service);
                    }
                    else
                    {
                        DEBUG_MSG(DEMUX, DEBUG, "  -> prog=" << dec << prog_num << ": not found in lineup services\n");
                    }
                }
            }
        }
        NEXT_PMT: {}
    }

    DEBUG_MSG(DEMUX, DEBUG, "=== finish_tp: TV/Radio classification END ===\n");

    Demux_Lineup::finish_tp();
}

} // namespace mb
