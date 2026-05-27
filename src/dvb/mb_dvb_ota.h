#pragma once

#include <cstdint>

#include "mb_dvb_si_table.h"
#include "mb_dvb_globals.h"
#include "common/mb_types.h"

namespace mb {

class OTA : public SI_Table
{
protected:
    std::vector<OTA_File> _ota_files;

public:
    virtual ~OTA() {}


    auto move_ota_files()
    {
        return std::move(_ota_files);
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
