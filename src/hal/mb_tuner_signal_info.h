#pragma once

#include <inttypes.h>
#include <common/mb_types.h>

namespace mb {

struct SignalInfo
{
    uint32_t signal_noise_ratio { 0 };
    uint32_t strength { 0 };
    uint32_t quality { 0 };
    uint32_t frequency { 0 };
    uint32_t symbol_rate { 0 };
    uint32_t bit_error_rate { 0 };
    Modulation_Type mode_type { Modulation_Type::Default };
    FEC_Rate fec_rate { FEC_Rate::AUTO };
    DVB_Mode signal_type { DVB_Mode::DVBS };
};

}
