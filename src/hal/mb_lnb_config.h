#pragma once

#include "common/mb_types.h"

namespace {

// C Band LNB L.O. frequency
constexpr auto LO_5150 = 5150;
constexpr auto LO_5750 = 5750;

// Ku Band LNB L.O. frequency
constexpr auto LO_9750 = 9750;
constexpr auto LO_10600 = 10600;

// Multi L.O. frequency
constexpr auto LO_11400 = 11400;
constexpr auto LO_10250 = 10250;

constexpr auto KU_11700 = 11700;

}

namespace mb {

enum LBNF_Voltage
{
    v13,
    v18,
};

struct LNBF_Config
{
    int l_freq { 0 };
    LBNF_Voltage voltage { v13 };
    bool tone_22khz { false };
};

LNBF_Config get_lnbf_config(Band _band, LNBF_Type _lnbf, bool _inverted, int _freq, Polarity _pol);

} // namespace mb
