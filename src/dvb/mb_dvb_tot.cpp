#include "mb_dvb_tot.h"

namespace mb {

TOT::TOT(const uint8_t *_data, size_t _size):
    TDT{_data, _size}
{
    // Same as TDT
    // mb_assert(_size >= 3);
    // mb_assert(table_id() == _data[0]);
    // if(table_id() != _data[0])
    // {
    //     return;
    // }
    //
    // // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    // auto section_syntax_indicator = _data[1] & 0b10000000;
    // if(section_syntax_indicator == 1)
    // {
    //     return;
    // }
    //
    // auto section_length = ((_data[1] & 0b00001111) << 8) | _data[2];
    auto descriptors_loop_length = ((_data[8] & 0b00001111) << 8) | _data[9];
    parse_descriptors(&_data[10], descriptors_loop_length);
}

void TOT::set_local_time_offset_descriptor(Local_Time_Offset_Descriptor _descriptor)
{
    _time_offsets = _descriptor.move_time_offsets();
}

} // namespace mb
