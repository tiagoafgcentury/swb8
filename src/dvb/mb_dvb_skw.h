#pragma once

#include <cstdint>

#include "mb_dvb_ota.h"

namespace mb {

class SKW final : public OTA
{
public:
    explicit SKW(const uint8_t *_data, size_t _size);
    ~SKW() {}

    virtual Table_ID_t table_id() const override
    {
        return SKYWORTH_OTA_HEADER_TABLE_ID;
    }
};

};
