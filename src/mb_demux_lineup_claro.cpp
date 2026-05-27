#include "mb_demux_lineup_claro.h"
#include "mb_demux_lineup.h"
#include "mb_demux_table_manager.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "hal/mb_demux.h"
#include "hal/mb_tuner.h"
#include "src/tasks/mb_task_tuner.h"
#include "mb_events.h"

#include <algorithm>
#include <chrono>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

using namespace std::chrono_literals;

namespace mb {

Demux_Lineup_Claro::Demux_Lineup_Claro(Task_Demux *_task_demux):
    Demux_Lineup(_task_demux)
{
}

Demux_Lineup_Claro::~Demux_Lineup_Claro()
{
}

void Demux_Lineup_Claro::parse_bat(BAT &&_bat, Lineup *_lineup)
{
    // Claro only sends viewer_channel in the first section of the BAT
    Viewer_Channel_t viewer_channel;

    if(_bat.viewer_channel())
    {
        viewer_channel = _bat.viewer_channel();
        m_bouquet_cannels[_bat.bouquet_id()] = viewer_channel;
    }
    else
    {
        viewer_channel = m_bouquet_cannels[_bat.bouquet_id()];
    }

    auto bouquet_name { _bat.move_bouquet_name() };
    auto transport_streams { _bat.move_transport_streams() };

    for(auto &ts : transport_streams)
    {
        auto tp = std::find_if(_lineup->transponders.begin(), _lineup->transponders.end(),
                               [&](const auto & s)
        {
            return s.transport_stream_id == ts.transport_stream_id() && s.satellite_id == m_current_satellite_id;
        });

        if(tp == _lineup->transponders.end())
        {
            DEBUG_MSG(DEMUX, WARN, "BAT transport_stream_id not found: " << dec << ts.transport_stream_id() << "\n");
            continue;
        }

        auto service_list_descriptors { ts.move_service_list_descriptors().move_services() };
        auto linkage_descriptors { ts.move_linkage_descriptors() };

        for(auto sit = _lineup->services.begin(); sit != _lineup->services.end(); ++sit)
        {
            if(sit->transponder_id() == tp->transponder_id)
            {
                for(const auto &s : service_list_descriptors)
                {
                    if(sit->service_id() == s.service_id)
                    {
                        sit->set_viewer_channel(viewer_channel + m_base_viewer_channel);
                        sit->set_bouquet_id(_bat.bouquet_id());
                        sit->set_bouquet_name(bouquet_name);
                        if (sit->regionalizacao() != Regionalizacao::RegionalizadoNacional)
                        {
                            sit->set_regionalizacao(Regionalizacao::Regionalizado);
                        }
                        break;
                    }
                }

                for(auto &ld : linkage_descriptors)
                {
                    if(sit->service_id() == ld.service_id())
                    {
                        auto zones { ld.move_zone_ids() };

                        if(sit->zones().empty())
                        {
                            sit->set_zones(std::move(zones));
                        }
                        else
                        {
                            for(auto z : zones)
                            {
                                sit->insert_zone(z);
                            }
                        }

                        break;
                    }
                }
            }
        }
    } // for (auto &ts : transport_streams)

    auto linkage_descriptors { _bat.move_linkage_descriptors() };

    for(auto &ld : linkage_descriptors)
    {
        auto tp = std::find_if(_lineup->transponders.begin(), _lineup->transponders.end(),
                               [&](const auto & s)
        {
            return s.transport_stream_id == ld.transport_stream_id() && s.satellite_id == m_current_satellite_id;
        });

        if(tp == _lineup->transponders.end())
        {
            DEBUG_MSG(DEMUX, WARN, "BAT transport_stream_id not found: " << dec << ld.transport_stream_id() << "\n");
            continue;
        }

        auto srv = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                                [&](const auto & s)
        {
            return s.service_id() == ld.service_id() && s.transponder_id() == tp->transponder_id;
        });

        if(srv == _lineup->services.end())
        {
            DEBUG_MSG(DEMUX, WARN, "BAT service_id not found: " << dec << ld.service_id() << "\n");
            continue;
        }

        srv->set_viewer_channel(viewer_channel + m_base_viewer_channel);
        srv->set_bouquet_id(_bat.bouquet_id());
        srv->set_bouquet_name(bouquet_name);
        srv->set_regionalizacao(Regionalizacao::RegionalizadoNacional);
    }
}

void Demux_Lineup_Claro::parse_nit(NIT &&_nit, Lineup *_lineup)
{
    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    auto transport_streams = _nit.move_transport_streams();

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
        auto service_list_descriptors { ts.move_service_list_descriptors() };
        auto services { service_list_descriptors.move_services() };
        DEBUG_MSG(DEMUX, DEBUG, "NIT Services count: " << services.size() << " TP: " << it->transponder_id.frequency() << "\n");

        for(auto &s : services)
        {
            auto sit = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                                    [&s, &it](const auto & serv)
            {
                return serv.service_id() == s.service_id && serv.transponder_id() == it->transponder_id;
            });

            if(sit == _lineup->services.end())
            {
                _lineup->services.emplace_back(it->transponder_id, s.service_id);
                DEBUG_MSG(DEMUX, DEBUG, "SID: " << dec << s.service_id << " SatID: " << (int)Config::get_config()->get_current_satellite() << "\n");
                sit = _lineup->services.begin();
                std::advance(sit, _lineup->services.size() - 1);
                sit->set_satellite_id(Config::get_config()->get_current_satellite());
            }

            sit->set_transponder_id(it->transponder_id);
            sit->set_service_type(static_cast<Service_Type>(s.service_type));
            sit->set_viewer_channel(s.viewer_channel + m_base_viewer_channel);
            sit->set_regionalizacao(s.regionalizacao);
        }
        DEBUG_MSG(DEMUX, DEBUG, "Total Service size: " << _lineup->services.size() << "\n");
    }
}

void Demux_Lineup_Claro::parse_sdt(SDT &&_sdt, Lineup *_lineup)
{
    const Transponder *tp { nullptr };
    auto transport_streams { _sdt.move_transport_streams() };

    if(_sdt.table_id() == SDT_TABLE_ID_ACTUAL)
    {
        auto [current_transponder, _] = Task_Tuner::get_current_transponder();
        if(_sdt.transport_stream_id())
        {
            for(auto &t : _lineup->transponders)
            {
                if(t.original_network_id == _sdt.original_network_id()
                   && t.transport_stream_id == _sdt.transport_stream_id()
                   && t.satellite_id == m_current_satellite_id)
                {
                    tp = &t;
                    break;
                }
            }
        }

        if(not tp)
        {
            tp = _lineup->get_transponder(current_transponder);
        }

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

        DEBUG_MSG(DEMUX, ERROR, "SDT TS Not found: " << dec << _sdt.original_network_id() << "/" << _sdt.transport_stream_id() << "\n");

        for(auto &s : transport_streams)
        {
            auto standard_name = s.move_service_name();

            if(standard_name.empty())
            {
                standard_name = s.move_service_provider_name();
            }

            DEBUG_MSG(DEMUX, DEBUG, "\tService: SID:" << dec << (int)s.service_id << " " << standard_name << "\n");
        }

        return;
    }

FOUND_TP:
    bool onid_tsid_mismatch = (_sdt.original_network_id() && _sdt.original_network_id() != tp->original_network_id) ||
                              (_sdt.transport_stream_id() && _sdt.transport_stream_id() != tp->transport_stream_id);

    for(auto &s : transport_streams)
    {
        auto sit = std::find_if(_lineup->services.begin(), _lineup->services.end(),
                                [&](const auto & service)
        {
            return service.service_id() == s.service_id && service.transponder_id() == tp->transponder_id;
        });

        if(sit == _lineup->services.end())
        {
            DEBUG_MSG(DEMUX, WARN, "ONID/TSID Mismatch: " << dec << s.service_id << " " << s.service_name << "\n");
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
        DEBUG_MSG(DEMUX, DEBUG, "SDT: Service: '" << dec << sit->service_id() << " " << sit->name() << "' Channel: " << sit->viewer_channel() << " SatID: " << sit->satellite_id() << "\n");

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
                              "\n\tType: " << (int)ld.linkage_type() <<
                              "\n\tONID: " << (int)ld.original_network_id() <<
                              "\n\tTSID: " << (int)ld.transport_stream_id() <<
                              "\n\tSID: " << (int)ld.service_id() <<
                              endl
                             );
            }
        }
    }
}

bool Demux_Lineup_Claro::check_sdts_is_done()
{
    if((done_tables & ptNIT) == 0)
    {
        return false;
    }

    if (sdts.empty())
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

void Demux_Lineup_Claro::parse_tables(Lineup *_lineup)
{
    using namespace std::placeholders;

#define DO_PARSE(TABLE) \
    do_parse(TABLE ## s, std::bind(&Demux_Lineup_Claro::parse_ ## TABLE, this, _1, _2), _lineup)

    if(done_tables & ptNIT)
    {
        DO_PARSE(sdt);

        if(done_tables & ptSDT)
        {
            DO_PARSE(pmt);
            DO_PARSE(bat);
        }
    }

#undef DO_PARSE
}

bool Demux_Lineup_Claro::check_bats_is_done(Bouquet_ID_t _bat_id)
{
    m_bouquet_seen_count[_bat_id]++;

    for(const auto &b : m_bouquet_seen_count)
    {
        if(b.second < BAT_RETRY_COUNT)
        {
            return false;
        }
    }

    // Check that we have all the sections
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

} // namespace mb
