#include "mb_dvb_nit.h"
#include "mb_dvb_globals.h"
#include "mb_dvb_nit.h"

#include "common/mb_assert.h"

namespace mb {

NIT::NIT(const uint8_t *_data, size_t _size):
    ITransport_Stream_Interface(this),
    _network_id{0},
    _version_number{0},
    _current_next_indicator{0}
{
    mb_assert(NIT_TABLE_ID_ACTUAL == _data[0] || NIT_TABLE_ID_OTHER == _data[0]);

    if(not(NIT_TABLE_ID_ACTUAL == _data[0] || NIT_TABLE_ID_OTHER == _data[0]))
    {
        return;
    }

    _table_id = _data[0];
    // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    auto section_syntax_indicator = _data[1] & 0b10000000;

    if(section_syntax_indicator == 0)
    {
        return;
    }

    auto section_length = ((_data[1] & 0b00001111) << 8) | _data[2];
    auto end = _data + section_length + 3;
    mb_assert(end == _data + _size);
    _network_id = (_data[3] << 8) | _data[4];
    _version_number = (_data[5] & 0b00111110) >> 1;
    _current_next_indicator = _data[5] & 0b00000001;
    _section_number = _data[6];
    _last_section_number = _data[7];
    auto network_descriptors_length = ((_data[8] & 0b00001111) << 8) | _data[9];
    _data += 10;
    mb_assert(_data < end);
    mb_assert(_data + network_descriptors_length < end);
    parse_descriptors(_data, network_descriptors_length);
    _data += network_descriptors_length;
    mb_assert(_data < end);
    auto transport_stream_loop_length = ((_data[0] & 0b00001111) << 8) | _data[1];
    mb_assert(_data + transport_stream_loop_length < end);
    parse_transport_streams(_data + 2, transport_stream_loop_length);
}

#ifndef NDEBUG
void NIT::hash(std::size_t &seed) const
{
    hash_combine(seed, _table_id, _network_id, _version_number, _current_next_indicator,
                 _section_number, _last_section_number, _network_name, _linkage_descriptors);
}

#endif

} // namespace mb
