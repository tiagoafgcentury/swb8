#include "mb_satellites.h"
#include "dvb/mb_dvb_globals.h"
#include "mb_debug.h"

#include <algorithm>
#include <iostream>

namespace mb {

MB_Satellites MB_Satellites::s_instance;

namespace {

struct TP_List
{
    Satellite_Operator satellite;

    int8_t priority_ota;
    int8_t priority_nit;
    int8_t priority_snr;

    Transponder transponder;
};

constexpr auto MAX_TP_PARAMS = 6;
const std::array<TP_List, MAX_TP_PARAMS> g_tp_data_list =
{
    {
        // Claro Home Channel
        {
            .satellite = Satellite_Operator::Claro,
            .priority_ota = 0,
            .priority_nit = 1,
            .priority_snr = 1,
            .transponder{Transponder_Id(12'120'000, Polarity::Vertical, 1), 29'892, DVB_Mode::DVBS2, 101, 45626, 45626, true}
        },

        // Claro Secondry Home Channel
        {
            .satellite = Satellite_Operator::Claro,
            .priority_ota = 1,
            .priority_nit = 1,
            .priority_snr = 1,
            .transponder{Transponder_Id(11'780'000, Polarity::Horizontal, 1), 29'892, DVB_Mode::DVBS2, 112, 45626, 45626, true}
        },

        // Claro Secondry Home Channel
        {
            .satellite = Satellite_Operator::Claro,
            .priority_ota = 2,
            .priority_nit = 1,
            .priority_snr = 1,
            .transponder{Transponder_Id(11'740'000, Polarity::Horizontal, 1), 29'892, DVB_Mode::DVBS2, 111, 45626, 45626, true}
        },

        // Sky Home Channel
        {
            .satellite = Satellite_Operator::Sky,
            .priority_ota = 0,
            .priority_nit = 0,
            .priority_snr = 0,
            .transponder{Transponder_Id(10'722'000, Polarity::Vertical, 2), 30'000, DVB_Mode::DVBS2, 24682, 162, 162, true}
        },
        // Sky Secondry Home Channel
        {
            .satellite = Satellite_Operator::Sky,
            .priority_ota = 1,
            .priority_nit = 1,
            .priority_snr = 1,
            .transponder{Transponder_Id(10'882'000, Polarity::Vertical, 2), 30'000, DVB_Mode::DVBS2, 24687, 162, 162, true}
        },
        // Sky Test OTA Channel
        {
            .satellite = Satellite_Operator::Sky,
            .priority_ota = 1,
            .priority_nit = 1,
            .priority_snr = 1,
            .transponder{Transponder_Id(11'510'000, Polarity::Horizontal, 2), 30'000, DVB_Mode::DVBS2, 24687, 162, 162, true}
        },
    }
};

template<typename Filter, typename Compare>
Transponder_List convert_tp_list(Filter _filter_fn, Compare _sort_fn)
{
    std::vector<TP_List> temp_list;

    for(const auto &tp : g_tp_data_list)
    {
        if(_filter_fn(tp))
        {
            temp_list.push_back(tp);
        }
    }

    std::sort(temp_list.begin(), temp_list.end(), _sort_fn);
    Transponder_List result;
    result.reserve(temp_list.size());

    for(const auto &tp : temp_list)
    {
        result.emplace_back(tp.transponder);
    }

    return result;
}

} // namespace

Transponder_List MB_Satellites::get_transponder_list_full()
{
    Transponder_List result;

    for(const auto &tp : g_tp_data_list)
    {
        result.emplace_back(tp.transponder);
    }

    return result;
}

Transponder_List MB_Satellites::get_transponder_list_for_nit(Satellite_Operator _satellite)
{
    auto filter_operator_and_nit = [_satellite](const TP_List & tp)
    {
        return tp.satellite == _satellite and tp.priority_nit >= 0;
    };
    auto sort_by_nit = [](const TP_List & lhs, const TP_List & rhs)
    {
        return lhs.priority_nit < rhs.priority_nit;
    };
    return convert_tp_list(filter_operator_and_nit, sort_by_nit);
}

Transponder_List MB_Satellites::get_transponder_list_for_snr(Satellite_Operator _satellite)
{
    auto filter_operator_and_snr = [_satellite](const TP_List & tp)
    {
        return tp.satellite == _satellite and tp.priority_snr >= 0;
    };
    auto sort_by_snr = [](const TP_List & lhs, const TP_List & rhs)
    {
        return lhs.priority_snr < rhs.priority_snr;
    };
    return convert_tp_list(filter_operator_and_snr, sort_by_snr);
}

static OTA_TS_PID_List s_ota_list;

OTA_TS_PID_List MB_Satellites::get_transponder_list_for_ota(Satellite_Operator _satellite)
{
    switch(_satellite)
    {
        case Satellite_Operator::Claro:
        {
            OTA_TS_PID_List result;
            auto filter_operator_and_ota = [_satellite](const TP_List & tp)
            {
                return tp.satellite == _satellite and tp.priority_ota >= 0;
            };
            auto sort_by_ota = [](const TP_List & lhs, const TP_List & rhs)
            {
                return lhs.priority_ota < rhs.priority_ota;
            };
            auto static_list = convert_tp_list(filter_operator_and_ota, sort_by_ota);

            for(const auto &it : static_list)
            {
                result.emplace_back(OTA_TS_PID{it, CLARO_OTA_TSPID});
            }
            return result;
        }

        case Satellite_Operator::Sky:
        {
            return s_ota_list;
        }

        case Satellite_Operator::Generic:
        {
            break;
        }
    };

    return {};
}

void MB_Satellites::set_transponder_list_for_ota(Satellite_Operator _satellite, OTA_TS_PID_List &&_list)
{
    switch(_satellite)
    {
        case Satellite_Operator::Claro:
        {
            // Claro has a fixed list
            mb_assert(false);
            break;
        }

        case Satellite_Operator::Sky:
        {
            s_ota_list = std::move(_list);
            break;
        }

        case Satellite_Operator::Generic:
        {
            break;
        }
    }
}

bool MB_Satellites::has_transponder_list_for_ota(Satellite_Operator _satellite)
{
    switch(_satellite)
    {
        case Satellite_Operator::Claro:
        {
            // Claro has a fixed list
            return true;
        }

        case Satellite_Operator::Sky:
        {
            return not s_ota_list.empty();
        }

        case Satellite_Operator::Generic:
        {
            break;
        }
    }
    return false;
}

} // namespace mb
