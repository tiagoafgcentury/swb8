#include "hal/mb_otp.h"

#include "common/mb_globals.h"

#include <aui_otp.h>
#include "mb_ali_globals.h"

namespace mb {

OTP::OTP()
{
    //ALI_EXEC(aui_otp_init(nullptr, nullptr));
}

OTP::~OTP()
{
    //ALI_EXEC(aui_otp_de_init(nullptr, nullptr));
}

bool OTP::read(void *_data, size_t _start, size_t _length)
{
    if(AUI_RTN_SUCCESS != aui_otp_read(_start * 4, static_cast<unsigned char *>(_data), _length))
    {
        DEBUG_MSG(HAL, ERROR, "aui_otp_read failed!\n");
        return false;
    }

    return true;
}

}
