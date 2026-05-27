#include "mb_config.h"
#include "mb_globals.h"
#include "mb_state_file.h"
#include "common/mb_hash.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sqlite/sqlite3.h>
#include <array>

namespace mb {

namespace {

// mantido para compatibilidade com o arquivo de configuracao
const std::array<Config::Satellite_Config, MAX_SATELLITE_PARAMS> sat_params =
{
    Config::Satellite_Config{"Star One D2", Network_Id_Claro, 11000000, 13000000, false, true, false, static_cast<uint32_t>(Network_Policies::TVRO),
        Transponder_List{Transponder{Transponder_Id{12120000, Polarity::Vertical, 1}, 29892, DVB_Mode::DVBS2, 101, Network_Id_Claro, Network_Id_Claro, true}}
    },
    Config::Satellite_Config{"Sky B1", Network_Id_Sky, 9750000, 12750000, false, true, true, static_cast<uint32_t>(Network_Policies::Sky),
        Transponder_List{Transponder{Transponder_Id{10722000, Polarity::Vertical, 2}, 30000, DVB_Mode::DVBS2, 24614, Network_Id_Sky, Network_Id_Sky, true}}
    },
    Config::Satellite_Config{"Generic", 0, 9750000, 12750000, false, true, false, static_cast<uint32_t>(Network_Policies::Generic), {}},
};

std::vector<Satellite> satellite_list;

}

Config *Config::s_config { nullptr };

Config::Config()
{
    mb_assert(s_config == nullptr);
    s_config = this;
    m_current_satellite = 0;
    m_selected_satellite = 0;
    m_selected_satellite_by_policies = 0;
    set_config();
    load_satellite_list();
}

Config::~Config()
{
    mb_assert(s_config == this);
    s_config = nullptr;
}

void Config::set_config()
{
    State_File::App_State_File file;
    set_satellite_config(file.network_id);
    set_band(file.band);
    set_lnbf_type(file.lnbf_type);
    m_current_satellite = file.current_satellite_id;
}

void Config::set_satellite_config_by_id(uint16_t _satellite_id, std::vector<Transponder> tp_list)
{
    if(satellite_list.empty())
    {
        DEBUG_MSG(COMMON, ERROR, "No satellites loaded\n");
        mb_assert(false);
    }

    uint16_t index = 0 ;
    for( ; index < satellite_list.size(); index++)
    {
        if(satellite_list[index].id == _satellite_id)
        {
            break;
        }
    }

    if (index == satellite_list.size())
    {
        DEBUG_MSG(COMMON, ERROR, "Invalid satellite id\n");
        return;
    }
    
    m_satellites.clear();
    m_satellites.resize(1);
    m_current_satellite = _satellite_id;

    uint16_t sat_idx = MAX_SATELLITE_PARAMS - 1;
    for(auto i = 0; i < MAX_SATELLITE_PARAMS; i++)
    {
        if(sat_params[i].network_policies == static_cast<uint32_t>(satellite_list[index].network_policies))
        {
            sat_idx = i;
            break;
        }
    }

    m_satellites[m_selected_satellite].tps.clear();
    m_satellites[m_selected_satellite].name = satellite_list[index].name;
    m_satellites[m_selected_satellite].network_id = sat_params[sat_idx].network_id;
    auto frequency_min = satellite_list[index].band == Band::Ku ? 9750000 : 3600000;
    m_satellites[m_selected_satellite].frequency_min = frequency_min;
    auto frequency_max = satellite_list[index].band == Band::Ku ? 12750000 : 4200000;
    m_satellites[m_selected_satellite].frequency_max = frequency_max;
    m_satellites[m_selected_satellite].nit_other = sat_params[sat_idx].nit_other;
    m_satellites[m_selected_satellite].sdt_other = sat_params[sat_idx].sdt_other;
    m_satellites[m_selected_satellite].read_all_tps = sat_params[sat_idx].read_all_tps;
    m_satellites[m_selected_satellite].network_policies = static_cast<uint32_t>(satellite_list[index].network_policies);
    m_lnbf_type = LNBF_Type::Universal;
    for(const auto &tp : tp_list)
    {
        m_satellites[m_selected_satellite].tps.emplace_back(tp);
    }    
    DEBUG_MSG(COMMON, INFO, "SAT_NAME: " << m_satellites[m_selected_satellite].name << "\n");
    DEBUG_MSG(COMMON, INFO, "Total de transponders configurados: " << m_satellites[m_selected_satellite].tps.size() << "\n");
    DEBUG_MSG(COMMON, INFO, "network_policies: " << m_satellites[m_selected_satellite].network_policies << "\n");
}

void Config::set_satellite_config(NID_t _network_id)
{
    uint16_t sat_idx = MAX_SATELLITE_PARAMS - 1;
    for(auto idx = 0; idx < MAX_SATELLITE_PARAMS; idx++)
    {
        if(sat_params[idx].network_id == _network_id)
        {
            sat_idx = idx;
            break;
        }
    }

    m_satellites.clear();
    m_satellites.resize(1);
    m_satellites[m_selected_satellite].name = sat_params[sat_idx].name;
    m_satellites[m_selected_satellite].network_id = sat_params[sat_idx].network_id;
    m_satellites[m_selected_satellite].frequency_min = sat_params[sat_idx].frequency_min;
    m_satellites[m_selected_satellite].frequency_max = sat_params[sat_idx].frequency_max;
    m_satellites[m_selected_satellite].nit_other = sat_params[sat_idx].nit_other;
    m_satellites[m_selected_satellite].sdt_other = sat_params[sat_idx].sdt_other;
    m_satellites[m_selected_satellite].read_all_tps = sat_params[sat_idx].read_all_tps;
    m_satellites[m_selected_satellite].network_policies = sat_params[sat_idx].network_policies;
    m_satellites[m_selected_satellite].tps.reserve(sat_params[sat_idx].tps.size());

    for(const auto &tp : sat_params[sat_idx].tps)
    {
        m_satellites[m_selected_satellite].tps.emplace_back(tp);
    }


    if (!satellite_list.empty())
    {
        for (const auto &sat : satellite_list)
        {
            if (static_cast<uint32_t>(sat.network_policies) == m_satellites[m_selected_satellite].network_policies)
            {
                m_current_satellite = sat.id;
                break;
            }
        }
    }

    DEBUG_MSG(COMMON, INFO, "m_selected_satellite: " << static_cast<int>(m_selected_satellite) << "\n");
    DEBUG_MSG(COMMON, INFO, "m_satellites.size(): " << m_satellites.size() << "\n");
    DEBUG_MSG(COMMON, INFO, "SAT_NAME: " << sat_params[sat_idx].name.c_str() << "\n");
}

void Config::load_satellite_list()
{
    DEBUG_MSG(COMMON, DEBUG, TERM_YELLOW_BOLD "Config::load_satellite_list()\n" TERM_RESET);
    Task::post_event_satellite_list_load(load_satellite_list_callback);
}

void Config::load_satellite_list_callback(const std::vector<Satellite> &sats)
{
    DEBUG_MSG(COMMON, DEBUG, TERM_YELLOW_BOLD "Config::load_satellite_list_callback()\n" TERM_RESET);
    DEBUG_MSG(COMMON, DEBUG, TERM_YELLOW_BOLD "Total de satélites carregados: " << sats.size() << "\n" TERM_RESET);
    if(sats.size() > 0)
    {
        satellite_list.clear();
        satellite_list = sats;

        if (s_config)
        {
            s_config->select_satellite_by_id(s_config->m_current_satellite);
        }
    }
}

std::vector<Satellite> Config::get_satellite_list()
{
    return satellite_list;
}

void Config::set_satellite_by_network_policies(Network_Policies _network_policies)
{
    if(satellite_list.empty())
    {
        DEBUG_MSG(COMMON, WARN, "No satellites loaded\n");
        mb_assert(false);
    }

    for(auto idx = 0; idx < satellite_list.size(); idx++)
    {
        if(satellite_list[idx].network_policies == _network_policies)
        {
            m_selected_satellite_by_policies = idx;
            break;
        }
    }
}

Satellite Config::get_satellite_by_network_policies()
{
    if(m_selected_satellite_by_policies < satellite_list.size())
    {
        Satellite sat = satellite_list[m_selected_satellite_by_policies];
        DEBUG_MSG(COMMON, DEBUG, TERM_GREEN_BOLD "Selected satellite: " << sat.name.c_str() << "\n" TERM_RESET);
        return sat;
    }
    else
    {
        DEBUG_MSG(COMMON, ERROR, "Invalid satellite index\n");
        mb_assert(false);
        return {};
    }
}

uint8_t Config::get_diseqc_port(uint16_t _satellite_id)
{
    if(satellite_list.empty())
    {
        DEBUG_MSG(COMMON, ERROR, "No satellites loaded\n");
        return 255;
    }

    for(auto idx = 0; idx < satellite_list.size(); idx++)
    {
        if(satellite_list[idx].id == _satellite_id)
        {
            if(satellite_list[idx].switch_type == DiseqC_Type::DiseqC_1_0)
            {
                return satellite_list[idx].switch_pos;
            }
            return 255;
        }
    }

    return 255;
}

Satellite Config::get_satellite_by_id(uint16_t _satellite_id)
{
    if(satellite_list.empty())
    {
        DEBUG_MSG(COMMON, ERROR, "No satellites loaded\n");
        return {};
    }

    for(auto idx = 0; idx < satellite_list.size(); idx++)
    {
        DEBUG_MSG(COMMON, DEBUG, "satellite_list[idx].id: " << satellite_list[idx].id << "\n");
        if(satellite_list[idx].id == _satellite_id)
        {
            return satellite_list[idx];
        }
    }

    DEBUG_MSG(COMMON, ERROR, "Invalid satellite id: " << _satellite_id << "\n");
    return {};
}

void Config::select_satellite_by_id(Satellite_ID_t _satellite_id)
{
    if(satellite_list.empty())
    {
        DEBUG_MSG(COMMON, ERROR, "No satellites loaded\n");
        return;
    }

    for(auto idx = 0; idx < satellite_list.size(); idx++)
    {
        if(satellite_list[idx].id == _satellite_id)
        {
            const auto &sat = satellite_list[idx];
            m_current_satellite = _satellite_id;
            m_band = sat.band;
            m_lnbf_type = sat.type;

            if (m_satellites.empty())
            {
                m_satellites.resize(1);
            }

            uint16_t sat_idx = MAX_SATELLITE_PARAMS - 1;
            for(auto i = 0; i < MAX_SATELLITE_PARAMS; i++)
            {
                if(sat_params[i].network_policies == static_cast<uint32_t>(sat.network_policies))
                {
                    sat_idx = i;
                    break;
                }
            }

            m_satellites[m_selected_satellite].name = sat.name;
            m_satellites[m_selected_satellite].network_id = sat_params[sat_idx].network_id;
            m_satellites[m_selected_satellite].network_policies = static_cast<uint32_t>(sat.network_policies);
            m_satellites[m_selected_satellite].frequency_min = sat.band == Band::Ku ? 9750000 : 3600000;
            m_satellites[m_selected_satellite].frequency_max = sat.band == Band::Ku ? 12750000 : 4200000;
            m_satellites[m_selected_satellite].nit_other = sat_params[sat_idx].nit_other;
            m_satellites[m_selected_satellite].sdt_other = sat_params[sat_idx].sdt_other;
            m_satellites[m_selected_satellite].read_all_tps = sat_params[sat_idx].read_all_tps;

            m_satellites[m_selected_satellite].tps.clear();
            for(const auto &tp : sat_params[sat_idx].tps)
            {
                m_satellites[m_selected_satellite].tps.emplace_back(tp);
            }

            return;
        }
    }

    DEBUG_MSG(COMMON, ERROR, "Invalid satellite id: " << _satellite_id << "\n");
}

DiseqC_Type Config::get_diseqc_type(Satellite_ID_t _satellite_id)
{
    if(satellite_list.empty())
    {
        DEBUG_MSG(COMMON, ERROR, "No satellites loaded\n");
        mb_assert(false);
    }

    for(auto idx = 0; idx < satellite_list.size(); idx++)
    {
        if(satellite_list[idx].id == _satellite_id)
        {
            return satellite_list[idx].switch_type;
        }
    }

    return DiseqC_Type::None;
}

const Config::Satellite_Config &Config::selected_satellite_config() const
{
    mb_assert(m_satellites.size() > 0);
    if(m_selected_satellite >= m_satellites.size())
    {
        return m_satellites.at(0);
    }
    else
    {
        return m_satellites.at(m_selected_satellite);
    }
}

} // namespace mb
