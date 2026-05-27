#include "mb_dvb_tables.h"
#include "common/mb_globals.h"
#include "common/mb_assert.h"

#include <algorithm>

namespace mb {

std::unordered_set<EIT_Service_ID_t> EIT::s_service_filter;

bool EIT::check_service_filter(const uint8_t *_data)
{
    return s_service_filter.count((_data[3] << 8) | _data[4]) > 0;
}

EIT::EIT(const uint8_t *_data, size_t _size)
{
    _table_id = _data[0];
    mb_assert(_table_id == EIT_TABLE_ID_PF_ACTUAL || _table_id == EIT_TABLE_ID_PF_OTHER ||
              (_table_id >= EIT_TABLE_ID_SCHEDULE_ACTUAL_LOW && _table_id <= EIT_TABLE_ID_SCHEDULE_ACTUAL_HIGH) ||
              (_table_id >= EIT_TABLE_ID_SCHEDULE_OTHER_LOW && _table_id <= EIT_TABLE_ID_SCHEDULE_OTHER_HIGH));
    // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    auto section_syntax_indicator { _data[1] & 0b10000000 };

    [[unlikely]] if(section_syntax_indicator != 0b10000000)
    {
        return;
    }

    auto section_length { (((_data[1] & 0b00001111) << 8) | _data[2]) - 4 }; // 4 = CRC
    auto end { _data + 3 + section_length };
    mb_assert(end == _data + (_size - 4));  // 3 = pos section length, 4 = CRC
    _size = section_length;
    _service_id = (_data[3] << 8) | _data[4];
    _version_number = (_data[5] & 0b00111110) >> 1;
    _current_next_indicator = _data[5] & 0b00000001;
    _section_number = _data[6];
    _last_section_number = _data[7];
    _transport_stream_id = (_data[8] << 8) | _data[9];
    _original_network_id = (_data[10] << 8) | _data[11];
    _segment_last_section_number = _data[12];
    _last_table_id = _data[13];
    _data += 11 + 3;
    _size -= 11;
    mb_assert(_data + _size == end);
    load_event(_data, _size);
}

void EIT::load_event(const uint8_t *_data, size_t _size)
{
    while(_size > 0)
    {
        auto s { _size };
        _events.emplace_back(this, _data, s);
        _size -= s;
        _data += s;
    }
}

EIT::Event::Event(EIT *_parent, const uint8_t *_data, size_t &_size):
    parent{_parent},
    start_time{&_data[2]}
{
    event_id = (_data[0] << 8) | _data[1];
    std::chrono::hours duration_hours{bcd2dec(_data[7])};
    std::chrono::minutes duration_minutes{bcd2dec(_data[8])};
    std::chrono::seconds duration_seconds{bcd2dec(_data[9])};
    duration = duration_hours + duration_minutes + duration_seconds;
    running_status = _data[10] & 0b11100000;
    free_CA_mode = _data[10] & 0b00010000;
    auto descriptors_loop_length { ((_data[10] & 0b00001111) << 8) + _data[11] };
    _data += 12;
    _size = 12 + descriptors_loop_length;
    parse_descriptors(_data, descriptors_loop_length);
}

void EIT::Event::set_parental_rating_descriptor(Parental_Rating_Descriptor _descriptor)
{
    parental_rating_descriptor = std::move(_descriptor);
}

void EIT::Event::set_short_event_descriptor(Short_Event_Descriptor _descriptor)
{
    if(short_event_descriptor.has_value())
    {
        short_event_descriptor->append(std::move(_descriptor));
    }
    else
    {
        short_event_descriptor = std::move(_descriptor);
    }
}

void EIT::Event::set_content_descriptor(Content_Descriptor _descriptor)
{
    content_descriptor = std::move(_descriptor);
}

void EIT::Event::set_extended_event_descriptor(Extended_Event_Descriptor _descriptor)
{
    if(extended_event_descriptor.has_value())
    {
        extended_event_descriptor->append(std::move(_descriptor));
    }
    else
    {
        extended_event_descriptor = std::move(_descriptor);
    }
}

void EIT::Event::set_linkage_descriptor(Linkage_Descriptor _descriptor)
{
    ca_rs_descriptor = _descriptor.move_ca_rs_descriptor();
}

#ifndef NDEBUG
void EIT::Event::hash(std::size_t &seed) const
{
    hash_combine(seed, event_id, start_time, duration.count(), running_status, free_CA_mode, rating);
}

void EIT::hash(std::size_t &seed) const
{
    hash_combine(seed, _table_id, _service_id, _version_number, _current_next_indicator,
                 _section_number, _last_section_number, _transport_stream_id, _original_network_id,
                 _segment_last_section_number, _last_table_id);

    for(const auto &e : _events)
    {
        e.hash(seed);
    }
}

#endif

} // namespace mb
