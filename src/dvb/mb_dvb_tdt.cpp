#include "mb_dvb_tdt.h"
#include "common/mb_types.h"
#include "common/mb_assert.h"

namespace mb {

TDT::TDT(const uint8_t *_data, size_t _size):
    _UTC_time{_data + 3}
{
    auto this_table_id { _data[0] };

    if(TDT_TABLE_ID == this_table_id)
    {
        mb_assert(_size == 8);
        mb_assert(table_id() == this_table_id);

        if(table_id() != this_table_id)
        {
            return;
        }
    }
    else
    {
        mb_assert(TOT_TABLE_ID == this_table_id);

        if(TOT_TABLE_ID != this_table_id)
        {
            return;
        }
    }

    // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    auto section_syntax_indicator { _data[1] & 0b10000000 };

    if(section_syntax_indicator == 1)
    {
        return;
    }

    auto section_length { ((_data[1] & 0b00001111) << 8) | _data[2] };
    auto end { _data + section_length + 3 };
    mb_assert(end == _data + _size);
}

#ifndef NDEBUG
void TDT::hash(std::size_t &seed) const
{
    hash_combine(seed, _UTC_time);
}

#endif

} // namespace mb
