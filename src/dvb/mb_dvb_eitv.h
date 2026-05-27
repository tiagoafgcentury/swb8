#pragma once

#include <cstdint>

#include "mb_dvb_ota.h"

namespace mb {

class EiTV final : public OTA
{
public:
    explicit EiTV(const uint8_t *_data, size_t _size);
    ~EiTV() {}

    virtual Table_ID_t table_id() const override
    {
        return DSI_TABLE_ID;
    }

};

};
