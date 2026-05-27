#pragma once

#include "common/mb_types.h"

namespace mb {

class Zone_ID
{
private:
    Zone_ID();
    virtual ~Zone_ID();

    Zone_ID_t m_zoneid_claro { 0 };
    Zone_ID_t m_zoneid_sky { 0 };

    static Zone_ID s_instance;

    void write_zone_id(Satellite_Operator _operator, Zone_ID_t _zone_id);

public:
    static void set_zone_id(Satellite_Operator _operator, const Zone_ID_t _new_zone_id);
    static Zone_ID_t get_zone_id(Satellite_Operator _operator);
};


} // namespace mb
