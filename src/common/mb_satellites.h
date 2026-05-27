#pragma once

#include "mb_lineup.h"

namespace mb {

class MB_Satellites
{
public:
    static MB_Satellites s_instance;
    
    static Transponder_List get_transponder_list_full();
    static Transponder_List get_transponder_list_for_nit(Satellite_Operator _satellite);
    static Transponder_List get_transponder_list_for_snr(Satellite_Operator _satellite);

    static OTA_TS_PID_List get_transponder_list_for_ota(Satellite_Operator _satellite);
    static void set_transponder_list_for_ota(Satellite_Operator _satellite, OTA_TS_PID_List &&_list);
    static bool has_transponder_list_for_ota(Satellite_Operator _satellite);
};

} // namespace mb
