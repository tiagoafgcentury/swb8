#include "mb_dvb_cat.h"

namespace mb {

CAT::CAT(uint8_t **_data, size_t _size)
{
    const auto p_data = *_data;
    _version_number = (p_data[5] & 0b00111110) >> 1;
    _section_number = p_data[6];
    _last_section_number = p_data[7];
    _cat_section_data.assign(*_data, _size);
    *_data = nullptr;
}

#ifndef NDEBUG
void CAT::hash(std::size_t &seed) const
{
    hash_combine(seed, std::string_view(reinterpret_cast<const char *>(_cat_section_data.data()), _cat_section_data.size()));
}

#endif

} // namespace mb
