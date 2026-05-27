#include "mb_dvb_ota.h"
#include "common/mb_globals.h"

#include <algorithm>

namespace mb {


#ifndef NDEBUG
void OTA::hash(std::size_t &seed) const
{
    for(const auto &of : _ota_files)
    {
        hash_combine(seed, of);
    }
}

#endif

} // namespace mb
