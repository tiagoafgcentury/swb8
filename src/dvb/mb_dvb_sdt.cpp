#include "mb_dvb_sdt.h"
#include "common/mb_globals.h"
#include "mb_dvb_globals.h"

namespace mb {

SDT::SDT(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 10);
    _table_id = _data[0];
    mb_assert(table_id() == SDT_TABLE_ID_ACTUAL || table_id() == SDT_TABLE_ID_OTHER);

    if(!(table_id() == SDT_TABLE_ID_ACTUAL || table_id() == SDT_TABLE_ID_OTHER))
    {
        return;
    }

    // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    auto section_syntax_indicator = _data[1] & 0b10000000;

    if(!section_syntax_indicator)
    {
        return;
    }

    auto section_length = ((_data[1] & 0b00001111) << 8) | _data[2];
    auto end = _data + section_length + 3;
    mb_assert(end == _data + _size);
    _transport_stream_id = (_data[3] << 8) | _data[4];
    _version_number = (_data[5] & 0b00111110) >> 1;
    _current_next_indicator = (_data[5] & 0b00000001);
    _section_number = _data[6];
    _last_section_number = _data[7];
    _original_network_id = (_data[8] << 8) | _data[9];
    // reserved field uses _data[10]
    _size -= 11;
    _data += 11;
    mb_assert(_data < end);

    while(_size > 4)
    {
        size_t section_size = _size;
        _transport_streams.emplace_back(this, _data, section_size);
        _data += section_size;
        _size -= section_size;
    }

    mb_assert(_size == 4); // CRC
}

SDT::Transport_Stream::Transport_Stream(SDT *_parent, const uint8_t *_data, size_t &_size):
    parent{_parent}
{
    mb_assert(_size >= 5);
    service_id = (_data[0] << 8) | _data[1];
    EIT_schedule_flag = (_data[2] & 0b00000010) >> 1;
    EIT_present_follow_flag = _data[2] & 0b00000001;
    running_status = (_data[3] & 0b11100000) >> 5;
    free_CA_mode = (_data[3] & 0b00010000) >> 4;
    size_t descriptors_loop_length = ((_data[3] & 0b00001111) << 8) | _data[4];
    _data += 5;
    mb_assert(descriptors_loop_length <= _size);
    _size = 5 + descriptors_loop_length;
    parse_descriptors(_data, descriptors_loop_length);
}

#ifndef NDEBUG
void SDT::Transport_Stream::hash(std::size_t &seed) const
{
    hash_combine(seed, service_id, EIT_schedule_flag, EIT_present_follow_flag, running_status,
                 free_CA_mode, service_type, service_provider_name, service_name, multilingual_service_names,
                 conent_nibble, _linkage_descriptors);
}

void SDT::hash(std::size_t &seed) const
{
    hash_combine(seed, _table_id, _transport_stream_id, _version_number, _current_next_indicator,
                 _section_number, _last_section_number, _original_network_id);

    for(const auto &s : _transport_streams)
    {
        s.hash(seed);
    }
}

#endif

} // namespace mb
