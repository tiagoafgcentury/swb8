#pragma once

#include <cstddef>

namespace mb {

class OTP
{
public:
    OTP();
    ~OTP();

    bool read(void *_data, size_t _start, size_t _length);
};

} // namespace mb
