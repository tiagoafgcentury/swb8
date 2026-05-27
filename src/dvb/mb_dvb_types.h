#pragma once

#include <string_view>

namespace mb {

constexpr uint8_t INVALID_DVB_TABLE_SECTION_NUMBER = 0xFF;

enum class CA_Type
{
    None = 0x00,
    Verimatrix = 0x01,
    Nagra = 0x02,
    Unknown = 0xFF,
};
std::string_view to_str(CA_Type _type);

enum class Regionalizacao
{
    Undefined = 0x00,
    Regionalizado = 0x01,
    RegionalizadoNacional = 0x03,
    NaoRegionalizado = 0x02,
};
std::string_view to_str(Regionalizacao _type);

enum class Service_Type
{
    none = 0x00,
    digital_television_service = 0x01,
    digital_radio_sound_service = 0x02,
    teletext_service = 0x03,
    nvod_reference_service = 0x04,
    nvod_time_shifted_service = 0x05,
    mosaic_service = 0x06,
    fm_radio_service = 0x07,
    dvb_srm_service = 0x08,
    reserved_for_future_use_00 = 0x09,
    advanced_codec_digital_radio_sound_service = 0x0a,
    advanced_codec_mosaic_service = 0x0b,
    data_broadcast_service = 0x0c,
    reserved_for_common_interface_usage = 0x0d,
    rcs_map = 0x0e,
    rcs_fls = 0x0f,
    dvb_mhp_service = 0x10,
    mpeg2_hd_digital_television_service = 0x11,
    advanced_codec_sd_digital_television_service = 0x16,
    advanced_codec_sd_nvod_time_shifted_service = 0x17,
    advanced_codec_sd_nvod_reference_service = 0x18,
    advanced_codec_hd_digital_television_service = 0x19,
    advanced_codec_hd_nvod_time_shifted_service = 0x1a,
    advanced_codec_hd_nvod_reference_service = 0x1b,
    advanced_codec_frame_compatible_plano_stereoscopic_hd_digital_television_service = 0x1c,
    advanced_codec_frame_compatible_plano_stereoscopic_hd_nvod_time_shifted_service = 0x1d,
    advanced_codec_frame_compatible_plano_stereoscopic_hd_nvod_reference_service = 0x1e,
    hevc_digital_television_service = 0x1f
};
std::string_view to_str(Service_Type _type);

enum class Basic_Service_Type
{
    TV,
    Radio,
    Other
};

Basic_Service_Type to_basic_type(Service_Type _type);

} // namespace mb
