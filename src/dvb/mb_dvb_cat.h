#pragma once

#include <cstdint>
#include <cstddef>

#include "mb_dvb_globals.h"
#include "mb_dvb_si_table.h"
#include "common/mb_types.h"

namespace mb {

// Conditional Access Table - Totally Encrypted
class CAT final : public SI_Table
{
private:
    uint8_t _version_number: 5;
    Section_Number_t _section_number { 0 };
    Section_Number_t _last_section_number { 0 };

    DVB_Table_Section _cat_section_data;

public:
    explicit CAT(uint8_t **_data, size_t _size);
    virtual ~CAT() {}

    virtual Table_ID_t table_id() const override
    {
        return CAT_TABLE_ID;
    }

    auto version_number() const
    {
        return _version_number;
    }

    auto section_number() const
    {
        return _section_number;
    }

    auto last_section_number() const
    {
        return _last_section_number;
    }

    DVB_Table_Section move_cat_section_data()
    {
        return std::move(_cat_section_data);
    }

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const override;

    virtual std::size_t hash() const override
    {
        std::size_t result { 0 };
        hash(result);
        return result;
    }
#endif
};

};
