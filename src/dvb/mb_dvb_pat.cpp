#include "mb_dvb_pat.h"

#ifndef NDEBUG
#include "common/mb_globals.h"
#endif
#include "common/mb_assert.h"

namespace mb {

PAT::PAT(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 12);
    mb_assert(table_id() == _data[0]);
    auto end = _data + _size;

    if(table_id() != _data[0])
    {
        return;
    }

    // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    auto section_syntax_indicator = _data[1] & 0b10000000;

    if(!section_syntax_indicator)
    {
        return;
    }

    auto section_length = ((_data[1] & 0b00000011) << 8) | _data[2];
    //auto table_id_extension = (_data[3] << 8) | _data[4];
    _version_number = (_data[5] & 0b00111110) >> 1;
    _current_next = (_data[5] & 0b00000001);
    _section_number = _data[6];
    _last_section_number = _data[7];
    _data += 8;
    auto section_end = _data + section_length - 5 - 4; // 5 = Section Lenght, 4 = CRC
    mb_assert(section_end <= end);

    if(section_end > end)
    {
        return;
    }

    _programs.reserve(section_length / 4);

    for(; _data < section_end; _data += 4)
    {
        auto program = (_data[0] << 8) | _data[1];
        auto pid = ((_data[2] & 0b00011111) << 8) | _data[3];
        _programs.emplace_back(program, pid);
    }
}

#ifndef NDEBUG
void PAT::Program::hash(std::size_t &seed) const
{
    hash_combine(seed, program, pid);
}

void PAT::hash(std::size_t &seed) const
{
    for(const auto &p : _programs)
    {
        p.hash(seed);
    }
}

#endif

} // namespace mb
