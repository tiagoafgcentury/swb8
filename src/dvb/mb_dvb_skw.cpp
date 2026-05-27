#include "mb_dvb_skw.h"
#include "common/mb_globals.h"

#include <algorithm>

namespace mb {

SKW::SKW(const uint8_t *_data, size_t _size)
{
    mb_assert(table_id() == _data[0]);

    if(table_id() != _data[0] || _size < 0x19)
    {
        return;
    }

    //private_data: 0x975A9F868B5DA729800000005B85EB9F035B6A360909020001000000000000000000000000000000000000000000000000000000000000613AE0C5
    _ota_files.reserve(1);
    OTA_File ota;
    ota.type = OTA_Type::Skyworth;
    ota.table_id = _data[0];
    ota.product = 0;  // TBD
    ota.version = _data[0x17] << 8 | _data[0x18];
    _ota_files.push_back(std::move(ota));
}

} // namespace mb
