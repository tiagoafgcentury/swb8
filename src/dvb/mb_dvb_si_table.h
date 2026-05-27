#pragma once

#include <cstdint>
#include "common/mb_types.h"

namespace mb {

class SI_Table
{
public:
    virtual Table_ID_t table_id() const = 0;
#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const = 0;

    virtual std::size_t hash() const
    {
        std::size_t result { 0 };
        hash(result);
        return result;
    }
#endif
};

};
