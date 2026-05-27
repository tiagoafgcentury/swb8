#include "mb_dvb_types.h"
#include <stdio.h>

namespace mb {

std::string_view to_str(CA_Type _type)
{
    switch(_type)
    {
        case CA_Type::None:
            return "None";

        case CA_Type::Nagra:
            return "Nagra";

        case CA_Type::Verimatrix:
            return "Verimatrix";

        case CA_Type::Unknown:
            return "Unknown";
    }

    return "UNDF";
}


std::string_view to_str(Regionalizacao _type)
{
    switch(static_cast<Regionalizacao>(_type))
    {
        case Regionalizacao::Undefined:
            return "Não definido";

        case Regionalizacao::Regionalizado:
            return "Regionalizado";

        case Regionalizacao::NaoRegionalizado:
            return "Não Regionalizado";

        case Regionalizacao::RegionalizadoNacional:
            return "Regionalizado Nacional";
    }

    return "<NA>";
}

std::string_view to_str(Service_Type _type)
{
    switch(_type)
    {
        case Service_Type::digital_television_service:
            return "Digital Television Service";

        case Service_Type::digital_radio_sound_service:
            return "Digital Radio Sound Service";

        case Service_Type::advanced_codec_digital_radio_sound_service:
            return "Advanced Codec Digital Radio Sound Service";

        case Service_Type::mpeg2_hd_digital_television_service:
            return "MPEG-2 HD Digital Television Service";

        case Service_Type::advanced_codec_sd_digital_television_service:
            return "Advanced Codec SD Digital Television Service";

        case Service_Type::advanced_codec_hd_digital_television_service:
            return "Advanced Codec HD Digital Television Service";

        case Service_Type::hevc_digital_television_service:
            return "HEVC Digital Television Service";

        default:
        {
            auto v = static_cast<unsigned int>(_type);
            constexpr auto max_digits = 6;
            static char buffer[max_digits + 1];
            snprintf(buffer, max_digits + 1, "0x%x", v);
            return buffer;
        }
    }
}

} // namespace mb
