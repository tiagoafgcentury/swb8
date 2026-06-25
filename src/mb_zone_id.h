#pragma once

#include "common/mb_types.h"
#include <optional>

namespace mb {

class Zone_ID
{
private:
    Zone_ID();
    virtual ~Zone_ID();

    Zone_ID_t m_zoneid_claro { 0 };
    Segment_ID_t m_zoneid_sky { 0 };

    static Zone_ID s_instance;

    void write_zone_id(Satellite_Operator _oper);

public:
    static void set_zone_id(const Zone_ID_t _new_zone_id, const Segment_ID_t _new_segment_id);
    static uint16_t get_zone_id(std::optional<Satellite_Operator> _oper = std::nullopt);
};


} // namespace mb
