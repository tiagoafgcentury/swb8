#pragma once

#include "mb_globals.h"
#include "mb_types.h"
#include "mb_lineup.h"
#include "mb_assert.h"
#include "tasks/mb_task_database.h"

#include <vector>

namespace mb {

constexpr auto MAX_SATELLITE_PARAMS = 3;

class Config
{
public:

    struct Satellite_Config
    {
        std::string name;
        NID_t network_id {};
        Frequency_t frequency_min = {};
        Frequency_t frequency_max = {};

        bool nit_other = false;
        bool sdt_other = false;
        bool read_all_tps = true;

        uint32_t network_policies = 0;
        Transponder_List tps;
    };

private:
    Band m_band = Band::UNDEFINED;
    LNBF_Type m_lnbf_type = LNBF_Type::UNDEFINED;

    uint32_t m_current_satellite = {};
    uint8_t m_selected_satellite = {};
    uint8_t m_selected_satellite_by_policies = {};
    std::vector<Satellite_Config> m_satellites;
    static Config *s_config;

public:
    Config();
    ~Config();

    static auto get_config()
    {
        mb_assert(s_config != nullptr);
        return s_config;
    }

    void set_config();

    Band band() const
    {
        return m_band;
    }

    void set_band(Band _band)
    {
        m_band = _band;
    }

    LNBF_Type lnbf_type() const
    {
        return m_lnbf_type;
    }

    void set_lnbf_type(LNBF_Type _type)
    {
        m_lnbf_type = _type;
    }

    int selected_satellite() const
    {
        return m_selected_satellite;
    }

    const Satellite_Config &selected_satellite_config() const;

    void set_satellite_resize(uint16_t size)
    {
        m_satellites.resize(size);
    }

    void set_satellite_config(NID_t _network_id);
    void set_satellite_config_by_id(uint16_t _satellite_id, std::vector<Transponder> tp_list);
    void set_satellite_by_network_policies(Network_Policies _network_policies);
    Satellite get_satellite_by_network_policies();
    void load_satellite_list();
    static void load_satellite_list_callback(const std::vector<Satellite> &sat);
    std::vector<Satellite> get_satellite_list();

    void set_current_satellite(uint32_t _current_satellite)
    {
        m_current_satellite = _current_satellite;
    }
    uint32_t get_current_satellite()
    {
        return m_current_satellite;
    }

    uint8_t get_diseqc_port(uint16_t _satellite_id);
    void select_satellite_by_id(uint16_t _satellite_id);
    Satellite get_satellite_by_id(uint16_t _satellite_id);
    DiseqC_Type get_diseqc_type(uint16_t _satellite_id);

};

}
