#include "mb_lnb_config.h"
#include "common/mb_globals.h"

namespace mb {

LNBF_Config get_lnbf_config(Band _band, LNBF_Type _lnbf, bool _inverted, int _freq, Polarity _pol)
{
    LNBF_Config result;

    if(_inverted)
    {
        switch(_pol)
        {
            case Polarity::Horizontal:
                _pol = Polarity::Vertical;
                break;

            case Polarity::Vertical:
                _pol = Polarity::Horizontal;
                break;

            case Polarity::Left:
                _pol = Polarity::Right;
                break;

            case Polarity::Right:
                _pol = Polarity::Left;
                break;

            case Polarity::UNDEFINED:
                DEBUG_MSG(HAL, ERROR, "Polarity: UNDEFINED\n");
                break;
        }
    }

    switch(_band)
    {
        case Band::C:
        {
            switch(_lnbf)
            {
                case LNBF_Type::Mono:
                    result.l_freq = 5150 - _freq;
                    result.voltage = _pol == Polarity::Horizontal ? v18 : v13;
                    result.tone_22khz = false;
                    DEBUG("\nMONOPONTO\n");
                    break;

                case LNBF_Type::Multi:
                    result.l_freq = _pol == Polarity::Horizontal ? LO_5150 - _freq : LO_5750 - _freq;
                    result.voltage = v18;
                    result.tone_22khz = false;
                    DEBUG("\nMULTIPONTO\n");
                    break;

                case LNBF_Type::Universal: // Não existe
                case LNBF_Type::UNDEFINED:
                    mb_assert(false);
                    break;
            }

            break;
        }

        case Band::Ku:
        {
            switch(_lnbf)
            {
                case LNBF_Type::Mono: // Não existe
                case LNBF_Type::UNDEFINED:
                    mb_assert(false);
                    break;

                case LNBF_Type::Multi:
                    result.l_freq = _pol == Polarity::Horizontal ? _freq - LO_11400 : _freq - LO_10250;
                    result.voltage = v18;
                    result.tone_22khz = false;
                    DEBUG("\nMULTIPONTO\n");
                    break;

                case LNBF_Type::Universal:
                    result.l_freq = _freq > KU_11700 ? _freq - LO_10600 : _freq - LO_9750;
                    result.voltage = _pol == Polarity::Horizontal ? v18 : v13;
                    result.tone_22khz = _freq > KU_11700;
                    DEBUG("\nUNIVERSAL\n");
                    break;
            }
            break;
        }

        case Band::UNDEFINED:
            mb_assert(false);
            break;
    }

    return result;
}

} // namespace mb
