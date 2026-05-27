#pragma once

#include <cstdint>
#include <cstddef>

#include "mb_dvb_globals.h"
#include "mb_dvb_tdt.h"
#include "mb_dvb_idescriptor_interface.h"


namespace mb {

class TOT final : public TDT, public IDescriptor_Interface
{
protected:
    decltype(Local_Time_Offset_Descriptor::_time_offsets) _time_offsets;

    virtual void set_local_time_offset_descriptor(Local_Time_Offset_Descriptor _descriptor) override;

public:
    explicit TOT(const uint8_t *_data, size_t _size);

    virtual Table_ID_t table_id() const override
    {
        return TOT_TABLE_ID;
    }
    auto move_time_offsets()
    {
        return std::move(_time_offsets);
    }

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const override
    {
        TDT::hash(seed);
    }

    virtual std::size_t hash() const override
    {
        return SI_Table::hash();
    }
#endif
};

};
