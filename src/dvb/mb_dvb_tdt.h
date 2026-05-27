#pragma once

#include <utility>
#include <cstdint>
#include <cstddef>

#include "mb_dvb_globals.h"
#include "mb_dvb_si_table.h"
#include "mb_dvb_utc_mjd.h"

namespace mb {

class TDT : public SI_Table
{
private:
    UTC_MJD _UTC_time;

public:
    explicit TDT(const uint8_t *_data, size_t _size);

    virtual Table_ID_t table_id() const override
    {
        return TDT_TABLE_ID;
    }
    auto move_utc_time()
    {
        return std::move(_UTC_time);
    }

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const override;

    virtual std::size_t hash() const override
    {
        return SI_Table::hash();
    }
#endif
};

};
