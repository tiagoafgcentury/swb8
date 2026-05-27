#include "mb_globals.h"
#include "mb_types.h"
#include "../dvb/mb_dvb_globals.h"
#include "mb_hash.h"

#include <stdio.h>

namespace mb {

std::string_view to_str(Polarity _type)
{
    switch(_type)
    {
        case Polarity::Horizontal:
            return "Horizontal";

        case Polarity::Vertical:
            return "Vertical";

        case Polarity::Left:
            return "Left";

        case Polarity::Right:
            return "Right";

        case Polarity::UNDEFINED:
        default:
            return "Undefined";
    }
}

std::string_view to_str(DVB_Mode _type)
{
    switch(_type)
    {
        case DVB_Mode::DVBS:
            return "DVBS";

        case DVB_Mode::DVBS2:
            return "DVBS2";

        case DVB_Mode::DVBS2X:
            return "DVBS2X";

        default:
            return "UNDEFINED";
    }
}

std::string_view to_str(Modulation_Type _type)
{
    switch(_type)
    {
        case Modulation_Type::Default:
            return "Default";

        case Modulation_Type::QAM_4:
            return "QAM_4";

        case Modulation_Type::QAM_4_NR:
            return "QAM_4_NR";

        case Modulation_Type::QAM_16:
            return "QAM_16";

        case Modulation_Type::QAM_32:
            return "QAM_32";

        case Modulation_Type::QAM_64:
            return "QAM_64";

        case Modulation_Type::QAM_128:
            return "QAM_128";

        case Modulation_Type::QAM_256:
            return "QAM_256";

        case Modulation_Type::QAM_512:
            return "QAM_512";

        case Modulation_Type::BPSK:
            return "BPSK";

        case Modulation_Type::QPSK:
            return "QPSK";

        case Modulation_Type::DQPSK:
            return "DQPSK";

        case Modulation_Type::PSK_8:
            return "PSK_8";

        case Modulation_Type::APSK_16:
            return "APSK_16";

        case Modulation_Type::APSK_32:
            return "APSK_32";

        case Modulation_Type::APSK_64:
            return "APSK_64";

        case Modulation_Type::APSK_128:
            return "APSK_128";

        case Modulation_Type::APSK_256:
            return "APSK_256";

        case Modulation_Type::APSK_L_8:
            return "APSK_L_8";

        case Modulation_Type::APSK_L_16:
            return "APSK_L_16";

        case Modulation_Type::APSK_L_32:
            return "APSK_L_32";

        case Modulation_Type::APSK_L_64:
            return "APSK_L_64";

        case Modulation_Type::APSK_L_128:
            return "APSK_L_128";

        case Modulation_Type::APSK_L_256:
            return "APSK_L_256";

        case Modulation_Type::VSB_8:
            return "VSB_8";

        case Modulation_Type::VSB_16:
            return "VSB_16";

        case Modulation_Type::AUTO:
            return "AUTO";

        case Modulation_Type::UNDEFINED:
        default:
            return "Undefined";
    }
}

std::string_view to_str(Band _type)
{
    switch(_type)
    {
        case Band::C:
            return "C";

        case Band::Ku:
            return "KU";

        case Band::UNDEFINED:
        default:
            return "Undefined";
    }
}

std::string_view to_str(LNBF_Type _type)
{
    switch(_type)
    {
        case LNBF_Type::Mono:
            return "Monoponto";

        case LNBF_Type::Multi:
            return "Multiponto";

        case LNBF_Type::Universal:
            return "Universal";

        default:
            return "Undefined";
    };
}

std::string_view to_str(LNBF_Position _type)
{
    switch(_type)
    {
        case LNBF_Position::Normal:
            return "Normal";

        case LNBF_Position::Inverted:
            return "Invertido";

        default:
            return "Undefined";
    }
}

std::string_view to_str(DiseqC_Type _type)
{
    switch(_type)
    {
        case DiseqC_Type::None:
            return "None";

        case DiseqC_Type::DiseqC_1_0:
            return "DiseqC 1.0";

        case DiseqC_Type::DiseqC_1_1:
            return "DiseqC 1.1";

        case DiseqC_Type::LNBF_Switch:
            return "LNBF Switch";

        default:
            return "Undefined";
    }
}

std::string_view to_str(FEC_Rate _fec_rate)
{
    switch(_fec_rate)
    {
        case FEC_Rate::AUTO:
            return "AUTO";

        case FEC_Rate::FEC_1_2:
            return "FEC_1_2";

        case FEC_Rate::FEC_2_3:
            return "FEC_2_3";

        case FEC_Rate::FEC_3_4:
            return "FEC_3_4";

        case FEC_Rate::FEC_4_5:
            return "FEC_4_5";

        case FEC_Rate::FEC_5_6:
            return "FEC_5_6";

        case FEC_Rate::FEC_6_7:
            return "FEC_6_7";

        case FEC_Rate::FEC_7_8:
            return "FEC_7_8";

        case FEC_Rate::FEC_8_9:
            return "FEC_8_9";

        case FEC_Rate::FEC_9_10:
            return "FEC_9_10";

        case FEC_Rate::FEC_1_4:
            return "FEC_1_4";

        case FEC_Rate::FEC_1_3:
            return "FEC_1_3";

        case FEC_Rate::FEC_2_5:
            return "FEC_2_5";

        case FEC_Rate::FEC_3_5:
            return "FEC_3_5";

        case FEC_Rate::FEC_5_9:
            return "FEC_5_9";

        case FEC_Rate::FEC_7_9:
            return "FEC_7_9";

        case FEC_Rate::FEC_4_15:
            return "FEC_4_15";

        case FEC_Rate::FEC_7_15:
            return "FEC_7_15";

        case FEC_Rate::FEC_8_15:
            return "FEC_8_15";

        case FEC_Rate::FEC_11_15:
            return "FEC_11_15";

        case FEC_Rate::FEC_13_18:
            return "FEC_13_18";

        case FEC_Rate::FEC_9_20:
            return "FEC_9_20";

        case FEC_Rate::FEC_11_20:
            return "FEC_11_20";

        case FEC_Rate::FEC_23_36:
            return "FEC_23_36";

        case FEC_Rate::FEC_25_36:
            return "FEC_25_36";

        case FEC_Rate::FEC_11_45:
            return "FEC_11_45";

        case FEC_Rate::FEC_13_45:
            return "FEC_13_45";

        case FEC_Rate::FEC_14_45:
            return "FEC_14_45";

        case FEC_Rate::FEC_26_45:
            return "FEC_26_45";

        case FEC_Rate::FEC_28_45:
            return "FEC_28_45";

        case FEC_Rate::FEC_29_45:
            return "FEC_29_45";

        case FEC_Rate::FEC_31_45:
            return "FEC_31_45";

        case FEC_Rate::FEC_32_45:
            return "FEC_32_45";

        case FEC_Rate::FEC_77_90:
            return "FEC_77_90";

        default:
            return "Unknown";
    }
}

std::string_view to_str(Video_Codec _codec)
{
    switch(_codec)
    {
        case Video_Codec::MPEG2:
            return "MPEG-2";

        case Video_Codec::MPEG4:
            return "MPEG-4";

        case Video_Codec::H264:
            return "H264";

        case Video_Codec::HEVC:
            return "HEVC";

        case Video_Codec::None:
            return "Nenhum";

        case Video_Codec::UNDEFINED:
            return "Unknown";
    }

    return "UNDF";
}

std::string_view to_str(Audio_Codec _codec)
{
    switch(_codec)
    {
        case Audio_Codec::AAC:
            return "AAC";

        case Audio_Codec::MP1:
            return "MPEG-1";

        case Audio_Codec::MP2:
            return "MPEG-2";

        case Audio_Codec::AC3:
            return "AC3";

        case Audio_Codec::None:
            return "Nenhum";

        case Audio_Codec::UNDEFINED:
            return "Unknown";
    }

    return "UNDF";
}


std::string_view to_str(Satellite_Operator _operator)
{
    switch(_operator)
    {
        case Satellite_Operator::Claro:
            return "Star One D2";

        case Satellite_Operator::Sky:
            return "Sky B1";

        case Satellite_Operator::Generic:
            return "Generic";
    }
    return "UNDF";
};

std::string_view to_str(Lineup_Origin _lineup_origin)
{
    switch(_lineup_origin)
    {
        case Lineup_Origin::LO_DATABASE:
            return "Databse";

        case Lineup_Origin::LO_SATELLITE:
            return "Satellite";
    }

    return "UNDF";
}

std::string_view to_str(OTA_Type _ota_type)
{
    switch(_ota_type)
    {
        case OTA_Type::EiTV:
            return "EiTV";

        case OTA_Type::Skyworth:
            return "Skyworth";

        case OTA_Type::Century_DSI:
            return "Century";
    }

    return "UNDF";
}

std::string_view to_str(Clock_Type _clock_status)
{
    switch(_clock_status)
    {
        case Clock_Type::Auto:
            return "Auto";

        case Clock_Type::Manual:
            return "Manual";
            break;

        case Clock_Type::Timezone:
;
            break;
    }

    return "Auto";
}

std::string_view to_str(Resolution_Standard _resolution)
{
    switch(_resolution)
    {
        case Resolution_Standard::_480i_60Hz:
            return "480i";

        case Resolution_Standard::_480p_60Hz:
            return "480p";

        case Resolution_Standard::_720p_60Hz:
            return "720p";

        case Resolution_Standard::_1080p_60Hz:
            return "1080p (60Hz)";

        case Resolution_Standard::_1080i_30Hz:
            return "1080i";
    }

    return "Undefined";
}

std::string_view to_str(Color_Standard _color_standart)
{
    switch(_color_standart)
    {
        case Color_Standard::None:
            return "None";

        case Color_Standard::NTSC_60:
            return "NTSC 60";

        case Color_Standard::PAL_M_50:
            return "PAL-M 50";

        case Color_Standard::PAL_M_60:
            return "PAL-M 60";

        case Color_Standard::PAL_N_50:
            return "PAL-N 50";

        case Color_Standard::UNDEFINED:
            break;
    }

    return "Undefined";
}

std::string_view to_str(Aspect_Mode _aspect_mode)
{
    switch(_aspect_mode)
    {
        case Aspect_Mode::AUTO:
            return "Auto";

        case Aspect_Mode::PILLBOX_16x9:
            return "Pillbox 16x9";

        case Aspect_Mode::PANSCAN_16x9:
            return "Panscan 16x9";

        case Aspect_Mode::LETTERBOX_16x9:
            return "Letterbox 16x9";

        case Aspect_Mode::FULLSCREEN_16x9:
            return "FullScreen 16x9";

        case Aspect_Mode::PANSCAN_4x3:
            return "Panscan 4x3";

        case Aspect_Mode::LETTERBOX_4x3:
            return "Letterbox 4x3";

        case Aspect_Mode::FULLSCREEN_4x3:
            return "FullScreen 4x3";
    }

    return "Undefined";
}

std::string_view to_str(Message_Categories _category)
{
    switch(_category)
    {
        case Message_Categories::Program_Access:
            return "Program Access";

        case Message_Categories::Program_Access_Denied:
            return "Program Access Denied";

        case Message_Categories::Event_Popup:
            return "Event Popup";

        case Message_Categories::Event_CAK_Reset:
            return "Event CAK Reset";

        case Message_Categories::COUNT:
            return "COUNT";
    }

    return "Undefined";
}


std::string_view to_str(CC_Type _cc_type)
{
    switch(_cc_type)
    {
        case CC_Type::Disabled:
            return "Disabled";

        case CC_Type::Subtitle:
            return "Subtitle";

        case CC_Type::Closed_Caption:
            return "Closed Caption";

        case CC_Type::COUNT:
            return "COUNT";
    }

    return "Undefined";
}


std::string_view to_str(Channel_List_Type _channel_list_type)
{
    switch(_channel_list_type)
    {
        case Channel_List_Type::MY_TV_CHANNELS:
            return "My TV Channels";

        case Channel_List_Type::MY_RADIO_CHANNELS:
            return "My Radios Channels";

        case Channel_List_Type::ALL_TV_CHANNELS:
            return "All TV Channels";

        case Channel_List_Type::ALL_RADIO_CHANNELS:
            return "All Radio Channels";

        case Channel_List_Type::COUNT:
            return "Count";
    }

    return "Undefined";
}

Basic_Service_Type to_basic_type(Service_Type _type)
{
    switch(_type)
    {
        case Service_Type::digital_radio_sound_service:
        case Service_Type::fm_radio_service:
            return Basic_Service_Type::Radio;

        case Service_Type::digital_television_service:
        case Service_Type::mpeg2_hd_digital_television_service:
        case Service_Type::advanced_codec_sd_digital_television_service:
        case Service_Type::advanced_codec_hd_digital_television_service:
        case Service_Type::hevc_digital_television_service:
            return Basic_Service_Type::TV;

        case Service_Type::none:
        case Service_Type::teletext_service:
        case Service_Type::nvod_reference_service:
        case Service_Type::nvod_time_shifted_service:
        case Service_Type::mosaic_service:
        case Service_Type::dvb_srm_service:
        case Service_Type::reserved_for_future_use_00:
        case Service_Type::advanced_codec_digital_radio_sound_service:
        case Service_Type::advanced_codec_mosaic_service:
        case Service_Type::data_broadcast_service:
        case Service_Type::reserved_for_common_interface_usage:
        case Service_Type::rcs_map:
        case Service_Type::rcs_fls:
        case Service_Type::dvb_mhp_service:
        case Service_Type::advanced_codec_sd_nvod_time_shifted_service:
        case Service_Type::advanced_codec_sd_nvod_reference_service:
        case Service_Type::advanced_codec_hd_nvod_time_shifted_service:
        case Service_Type::advanced_codec_hd_nvod_reference_service:
        case Service_Type::advanced_codec_frame_compatible_plano_stereoscopic_hd_digital_television_service:
        case Service_Type::advanced_codec_frame_compatible_plano_stereoscopic_hd_nvod_time_shifted_service:
        case Service_Type::advanced_codec_frame_compatible_plano_stereoscopic_hd_nvod_reference_service:
        default:
            return Basic_Service_Type::Other;
    }
}

struct DVB_Table_Section::Ptr
{
    Ptr(Size_Type _size, uint8_t *_data):
        m_size(_size),
        m_data(_data)
    {
    }

    ~Ptr()
    {
        if(m_data)
        {
            free(m_data);
        }
    }

    Size_Type m_size = { 0 };
    uint8_t *m_data = { nullptr };
};

DVB_Table_Section::DVB_Table_Section(const uint8_t *_data, Size_Type _size)
{
    auto p = static_cast<uint8_t *>(malloc(_size));
    memcpy(p, _data, _size);
    m_p = std::make_shared<Ptr>(_size, p);
}

void DVB_Table_Section::assign(uint8_t *_data, Size_Type _size)
{
    m_p = std::make_shared<Ptr>(_size, _data);
}

DVB_Table_Section::DVB_Table_Section(const DVB_Table_Section &_other):
    m_p(_other.m_p)
{
}

DVB_Table_Section::DVB_Table_Section(DVB_Table_Section &&_other):
    m_p(std::move(_other.m_p))
{
}

void DVB_Table_Section::operator=(DVB_Table_Section &&_other)
{
    m_p = std::move(_other.m_p);
}

void DVB_Table_Section::operator=(const DVB_Table_Section &_other)
{
    m_p = _other.m_p;
}

const uint8_t *DVB_Table_Section::data() const
{
    return m_p ? m_p->m_data : nullptr;
}

DVB_Table_Section::Size_Type DVB_Table_Section::size() const
{
    return m_p ? m_p->m_size : 0;
}

bool DVB_Table_Section::empty() const
{
    return m_p ? m_p->m_size == 0 : true;
}

} // namespace mb
