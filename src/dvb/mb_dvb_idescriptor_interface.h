#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <chrono>
#include <set>

#include "mb_dvb_types.h"
#include "common/mb_types.h"

namespace mb {

class Audio_Stream_Header_AAC_Descriptor
{
private:

public:
    explicit Audio_Stream_Header_AAC_Descriptor(const uint8_t *_data, size_t _size);
};

class Audio_Stream_Header_Mpeg4_Descriptor
{
private:

public:
    explicit Audio_Stream_Header_Mpeg4_Descriptor(const uint8_t *_data, size_t _size);
};

class CA_Descriptor
{
private:
    uint16_t _ca_system_id { 0 };
    PID_t _pid: 13;
public:
    explicit CA_Descriptor(const uint8_t *_data, size_t _size);

    auto ca_system_id() const
    {
        return _ca_system_id;
    }
    auto pid() const
    {
        return _pid;
    }
};

class Caption_Service_Descriptor
{
public:
    explicit Caption_Service_Descriptor(const uint8_t *_data, size_t _size);
};

class Content_Descriptor
{
public:
    enum class Content_Nibble
    {
        Undefined_Content,
        Movie_Drama_General,
        Detective_Thriller,
        Adventure_Western_War,
        Science_Fiction_Fantasy_Horror,
        Comedy,
        Soap_Melodrama_Folkloric,
        Romance,
        Serious_Classical_Religious_Historical_movie_Drama,
        Adult_Movie_Drama,
        News_Current_Affairs,
        News_Weather_Report,
        News_Magazine,
        Documentary
    };

private:
    Content_Nibble _content;
public:
    explicit Content_Descriptor(const uint8_t *_data, size_t _size);

    auto content_nibble() const
    {
        return _content;
    }
};

class Extended_Event_Descriptor
{
public:
    struct Item
    {
        std::string item_description;
        std::string item;
    };

private:
    char ISO_639_language_code[4] { 0 };

    std::string _text;

    std::vector<Item> _items;
public:
    explicit Extended_Event_Descriptor(const uint8_t *_data, size_t _size);

    const char *language_code() const
    {
        return ISO_639_language_code;
    }
    auto move_items()
    {
        return std::move(_items);
    }
    auto move_text()
    {
        return std::move(_text);
    }

    void append(Extended_Event_Descriptor &&_other)
    {
        _text.append(std::move(_other._text));
        _items.insert(_items.end(), _other._items.begin(), _other._items.end());
    }
};

class Language_Descriptor
{
private:
    uint8_t _audio_type { 0 };
    char _language_code[3 + 1] { 0 };
public:
    explicit Language_Descriptor(const uint8_t *_data, size_t _size);

    auto audio_type() const
    {
        return _audio_type;
    }
    const char *language_code() const
    {
        return _language_code;
    }
};

class OTA_SWDL_Descriptor
{
private:
    uint16_t _tsid = 0;
    uint16_t _onid = 0;
    uint16_t _svc_id = 0;
    uint32_t _OUI: 24;
    uint8_t _manufacturer_code = 0;
    uint8_t _hardware_code = 0;
    uint8_t _model_code = 0;
    uint8_t _download_mode = 0;
    uint16_t _software_version = 0;
    uint16_t _pid: 13;
    uint8_t _factory_reset_flag = 0;

public:
    OTA_SWDL_Descriptor(const uint8_t *_data, size_t _size);

    auto tsid() const
    {
        return _tsid;
    }
    auto onid() const
    {
        return _onid;
    }
    auto svc_id() const
    {
        return _svc_id;
    }
    auto OUI() const
    {
        return _OUI;
    }
    auto manufacturer_code() const
    {
        return _manufacturer_code;
    }
    auto hardware_code() const
    {
        return _hardware_code;
    }
    auto model_code() const
    {
        return _model_code;
    }
    auto download_mode() const
    {
        return _download_mode;
    }
    auto software_version() const
    {
        return _software_version;
    }
    auto pid() const
    {
        return _pid;
    }
    auto factory_reset_flag() const
    {
        return _factory_reset_flag;
    }
};

class CA_RS_Descriptor
{
private:
    uint8_t _bseid_tag = 0;
    uint8_t _bseid_length = 0;
    uint16_t _bseid = 0;
    uint8_t _r_osd_dur_tag = 0;
    uint8_t _r_osd_dur_length = 0;
    uint16_t _r_osd_duration = 0;

public:
    CA_RS_Descriptor(const uint8_t *_data, size_t _size );

    auto bseid_tag() const
    {
        return _bseid_tag;
    }
    auto bseid_length() const
    {
        return _bseid_length;
    }
    auto bseid() const
    {
        return _bseid;
    }
    auto r_osd_dur_tag() const
    {
        return _r_osd_dur_tag;
    }
    auto r_osd_dur_length() const
    {
        return _r_osd_dur_length;
    }
    auto r_osd_duration() const
    {
        return _r_osd_duration;
    }
};

class Linkage_Descriptor
{
    friend struct std::hash<mb::Linkage_Descriptor>;
public:
    enum class Linkage_Type
    {
        reserved_for_future_use = 0x00,
        information_service = 0x01,
        epg_service = 0x02,
        ca_replacement_service = 0x03,
        ts_containing_complete_network_bouquet_si = 0x04,
        service_replacement_service = 0x05,
        data_broadcast_service = 0x06,
        rcs_map = 0x07,
        mobile_hand_over = 0x08,
        system_software_update_service = 0x09,
        ts_containing_ssu_bat_or_nit = 0x0a,
        ip_mac_notification_service = 0x0b,
        ts_containing_int_bat_or_nit = 0x0c,
        event_linkage = 0x0d,
        extended_event_linkage = 0x0e,
    };

private:
    TS_ID_t _transport_stream_id { 0 };
    NID_t _original_network_id { 0 };
    Service_ID_t _service_id { 0 };
    uint8_t _linkage_type { 0 };
    std::set<Zone_ID_t> _zone_ids;

    std::optional<std::vector<OTA_SWDL_Descriptor>> _ota_swdl_descriptor;
    std::optional<CA_RS_Descriptor> _ca_rs_descriptor;

public:
    explicit Linkage_Descriptor(const uint8_t *_data, size_t _size);

    auto transport_stream_id() const
    {
        return _transport_stream_id;
    };
    auto original_network_id() const
    {
        return _original_network_id;
    };
    auto service_id() const
    {
        return _service_id;
    };
    auto linkage_type() const
    {
        return static_cast<Linkage_Type>(_linkage_type);
    };
    auto move_zone_ids()
    {
        return std::move(_zone_ids);
    }
    auto ota_swdl_descriptor() const
    {
        return _ota_swdl_descriptor;
    }
    auto move_ca_rs_descriptor() const
    {
        return std::move(_ca_rs_descriptor);
    }
};

class Local_Time_Offset_Descriptor
{
public:
    struct Time_Offset
    {
        Time_Offset(const uint8_t *_data);

        char country_code[4] { 0 };
        uint8_t country_region_id { 0 };
        uint8_t local_time_offset_polarity: 1;
        std::chrono::minutes local_time_offset { 0 };
    };

    std::vector<Time_Offset> _time_offsets;

public:
    explicit Local_Time_Offset_Descriptor(const uint8_t *_data, size_t _size);
    auto move_time_offsets()
    {
        return std::move(_time_offsets);
    }
};

class Maximum_Bit_Rate_Descriptor
{
private:

public:
    explicit Maximum_Bit_Rate_Descriptor(const uint8_t *_data, size_t _size);
};

class Multilingual_Service_Name
{
    friend struct std::hash<mb::Multilingual_Service_Name>;
public:
    struct Name
    {
        std::string language;
        std::string service_provider_name;
        std::string service_name;
    };

private:
    std::vector<Name> _names;

public:
    explicit Multilingual_Service_Name(const uint8_t *_data, size_t _size);
    auto move_names()
    {
        return std::move(_names);
    }
};

class Parental_Rating_Descriptor
{
private:
    struct Rating
    {
        char country_code[3 + 1] { 0 };
        uint8_t rating { 0 };
    };
    std::vector<Rating> _ratings;

public:
    explicit Parental_Rating_Descriptor(const uint8_t *_data, size_t _size);
    auto move_ratings()
    {
        return std::move(_ratings);
    };
};

class Registration_Descriptor
{
private:
    char _format[5] { 0 };

public:
    explicit Registration_Descriptor(const uint8_t *_data, size_t _size);

    const char *format() const
    {
        return _format;
    }
};

class S2_Satellite_Delivery_System_Descriptor
{
    friend struct std::hash<S2_Satellite_Delivery_System_Descriptor>;
private:
    uint8_t _descriptor_tag_extension { 0 };
    uint8_t _receiver_profiles: 5;
    uint8_t _s2x_mode: 2;
    uint8_t _scrambling_sequence_selector: 1;
    uint8_t _ts_gs_s2x_mode: 2;
    Frequency_t _frequency { 0 };
    uint16_t _orbital_position { 0 };
    uint8_t _west_east_flag: 1;
    uint8_t _polarization: 2;
    uint8_t _multiple_input_stream_flag: 1;
    uint8_t _roll_off: 3;
    Symbol_Rate_t _symbol_rate: 28;

public:
    S2_Satellite_Delivery_System_Descriptor() {}
    explicit S2_Satellite_Delivery_System_Descriptor(const uint8_t *_data, size_t _size);

    auto descriptor_tag_extension() const
    {
        return _descriptor_tag_extension;
    }
    auto receiver_profiles() const
    {
        return _receiver_profiles;
    }
    auto s2x_mode() const
    {
        return _s2x_mode;
    }
    auto scrambling_sequence_selector() const
    {
        return _scrambling_sequence_selector;
    }
    auto ts_gs_s2x_mode() const
    {
        return _ts_gs_s2x_mode;
    }
    auto frequency() const
    {
        return _frequency;
    }
    auto orbital_position() const
    {
        return _orbital_position;
    }
    auto west_east_flag() const
    {
        return _west_east_flag;
    }
    auto polarization() const
    {
        return _polarization;
    }
    auto multiple_input_stream_flag() const
    {
        return _multiple_input_stream_flag;
    }
    auto roll_off() const
    {
        return _roll_off;
    }
    auto symbol_rate() const
    {
        return _symbol_rate;
    }
};

class Satellite_Delivery_System_Descriptor
{
    friend struct std::hash<mb::Satellite_Delivery_System_Descriptor>;
private:
    Frequency_t _frequency { 0 };
    uint16_t _orbital_position { 0 };
    uint8_t _west_east_flag: 1;
    uint8_t _polarization: 2;
    uint8_t _roll_off: 2;
    uint8_t _modulation_system: 1;
    uint8_t _modulation_type: 2;
    Symbol_Rate_t _symbol_rate: 28;
    uint8_t _FEC_inner: 4;

public:
    Satellite_Delivery_System_Descriptor() {}
    explicit Satellite_Delivery_System_Descriptor(const uint8_t *_data, size_t _size);

    auto frequency() const
    {
        return _frequency;
    }
    auto orbital_position() const
    {
        return _orbital_position;
    }
    auto west_east_flag() const
    {
        return _west_east_flag;
    }
    auto polarization() const
    {
        return _polarization;
    }
    auto roll_off() const
    {
        return _roll_off;
    }
    auto modulation_system() const
    {
        return _modulation_system;
    }
    auto modulation_type() const
    {
        return _modulation_type;
    }
    auto symbol_rate() const
    {
        return _symbol_rate;
    }
    auto FEC_inner() const
    {
        return _FEC_inner;
    }
};

class Service_Descriptor
{
private:
    uint8_t _service_type { 0 };
    std::string _service_provider_name;
    std::string _service_name;

public:
    explicit Service_Descriptor(const uint8_t *_data, size_t _size);

    auto service_type() const
    {
        return static_cast<Service_Type>(_service_type);
    }
    auto move_service_provider_name()
    {
        return std::move(_service_provider_name);
    }
    auto move_service_name()
    {
        return std::move(_service_name);
    }
};

class Service_List_Descriptor
{
public:
    struct Service
    {
        Service(Service_ID_t _id, uint8_t _type):
            service_id(_id), service_type(_type)
        {}

        Service_ID_t service_id { 0 };
        uint8_t service_type { 0 };
        Viewer_Channel_t viewer_channel { 0 };
        Regionalizacao regionalizacao { Regionalizacao::Undefined };
    };

private:
    std::vector<Service> _services;

public:
    Service_List_Descriptor() {}
    explicit Service_List_Descriptor(const uint8_t *_data, size_t _size);

    auto move_services()
    {
        return std::move(_services);
    }

    auto begin()
    {
        return _services.begin();
    }
    auto end()
    {
        return _services.end();
    }
};

class Short_Event_Descriptor
{
private:
    char ISO_639_language_code[4] { 0 };
    std::string event_name;
    std::string text;

public:
    explicit Short_Event_Descriptor(const uint8_t *_data, size_t _size);

    const char *language_code() const
    {
        return ISO_639_language_code;
    }

    std::string move_event_name()
    {
        return std::move(event_name);
    }

#ifndef NDEBUG
    const char *peek_event_name()
    {
        return event_name.c_str();
    }
#endif

    std::string move_text()
    {
        return std::move(text);
    }

    void append(Short_Event_Descriptor &&_other)
    {
        if(event_name.empty()) {
            event_name = std::move(_other.event_name);
        } else if(!_other.event_name.empty() && event_name != _other.event_name) {
            event_name.append(" ");
            event_name.append(_other.event_name);
        }
        text.append(std::move(_other.text));
    }

};

class Ud_Sky_Logical_Channel_Descriptor
{
public:
    struct Service
    {
        Service(Service_ID_t _service_id, Viewer_Channel_t _viewer_channel):
            service_id(_service_id), viewer_channel(_viewer_channel)
        {}

        Service_ID_t service_id { 0 };
        Viewer_Channel_t viewer_channel { 0 };
    };
private:
    std::vector<Service> _services;

public:
    Ud_Sky_Logical_Channel_Descriptor() {}
    explicit Ud_Sky_Logical_Channel_Descriptor(const uint8_t *_data, size_t _size);

    auto move_services()
    {
        return std::move(_services);
    }
};

class Ud_Sky_Service_Order_List_Descriptor
{
public:
    struct Service
    {
        Service(Service_ID_t _service_id, General_Order_t _general_order, Order_By_Type_t _order_by_type):
            service_id(_service_id), general_order(_general_order), order_by_type(_order_by_type)
        {}

        Service_ID_t service_id { 0 };
        Viewer_Channel_t viewer_channel { 0 };
        General_Order_t general_order { 0 };
        Order_By_Type_t order_by_type { 0 };
    };

private:
    std::vector<Service> _services;

public:
    Ud_Sky_Service_Order_List_Descriptor() {}
    explicit Ud_Sky_Service_Order_List_Descriptor(const uint8_t *_data, size_t _size);

    auto move_services()
    {
        return std::move(_services);
    }
};


class Sinalizacao_dos_Canais_Nao_Regionalizados
{
private:
    struct Service
    {
        Service(Service_ID_t _service_id, Viewer_Channel_t _viewer_channel):
            service_id(_service_id), viewer_channel(_viewer_channel)
        {}

        Service_ID_t service_id { 0 };
        Viewer_Channel_t viewer_channel { 0 };
    };
    std::vector<Service> _services;

public:
    explicit Sinalizacao_dos_Canais_Nao_Regionalizados(const uint8_t *_data, size_t _size);

    auto move_services()
    {
        return std::move(_services);
    }
};

class Sinalizacao_dos_Canais_Regionalizados
{
private:
    Viewer_Channel_t _viewer_channel { 0 };

public:
    explicit Sinalizacao_dos_Canais_Regionalizados(const uint8_t *_data, size_t _size);

    auto viewer_channel()
    {
        return _viewer_channel;
    }
};

class Smoothing_Buffer_Descriptor
{
private:

public:
    explicit Smoothing_Buffer_Descriptor(const uint8_t *_data, size_t _size);
};

class Stream_Identifier_Descriptor
{
private:
    uint8_t _component_tag { 0 };

public:
    explicit Stream_Identifier_Descriptor(const uint8_t *_data, size_t _size);
    auto component_tag() const
    {
        return _component_tag;
    }
};

class Subtitling_Descriptor
{
private:
    std::vector<DVB_Subtitle_Info> _subtitles;

public:
    explicit Subtitling_Descriptor(const uint8_t *_data, size_t _size);

    auto move_subtitle() const
    {
        return std::move(_subtitles);
    }

};


class IDescriptor_Interface
{
protected:
    virtual Table_ID_t table_id() const = 0;

    virtual void set_audio_stream_header_aac_descriptor(Audio_Stream_Header_AAC_Descriptor _descriptor);
    virtual void set_audio_stream_header_mpeg4_descriptor(Audio_Stream_Header_Mpeg4_Descriptor _descriptor);
    virtual void set_bouquet_name(const char *_name, size_t _length);
    virtual void set_ca_descriptor(CA_Descriptor _descriptor);
    virtual void set_caption_service_descriptor(Caption_Service_Descriptor _descriptor);
    virtual void set_content_descriptor(Content_Descriptor _descriptor);
    virtual void set_extended_event_descriptor(Extended_Event_Descriptor _descriptor);
    virtual void set_language_descriptor(Language_Descriptor _descriptor);
    virtual void set_linkage_descriptor(Linkage_Descriptor _descriptor);
    virtual void set_local_time_offset_descriptor(Local_Time_Offset_Descriptor _descriptor);
    virtual void set_maximum_bit_rate_descriptor(Maximum_Bit_Rate_Descriptor _descriptor);
    virtual void set_multilingual_service_name(Multilingual_Service_Name _descriptor);
    virtual void set_network_name(const char *_name, size_t _length);
    virtual void set_parental_rating_descriptor(Parental_Rating_Descriptor _descriptor);
    virtual void set_registration_descriptor(Registration_Descriptor _descriptor);
    virtual void set_s2_satellite_delivery_system_descriptor(S2_Satellite_Delivery_System_Descriptor _descriptor);
    virtual void set_satellite_delivery_system_descriptor(Satellite_Delivery_System_Descriptor _descriptor);
    virtual void set_service_descriptor(Service_Descriptor _descriptor);
    virtual void set_service_list_descriptor(Service_List_Descriptor _descriptor);
    virtual void set_short_event_descriptor(Short_Event_Descriptor _descriptor);
    virtual void set_sinalizacao_dos_canais_nao_regionalizados(Sinalizacao_dos_Canais_Nao_Regionalizados _descriptor);
    virtual void set_sinalizacao_dos_canais_regionalizados(Sinalizacao_dos_Canais_Regionalizados _descriptor);
    virtual void set_ud_sky_logical_channel_descriptor(Ud_Sky_Logical_Channel_Descriptor);
    virtual void set_ud_sky_service_order_list_descriptor(Ud_Sky_Service_Order_List_Descriptor);
    virtual void set_smoothing_buffer_descriptor(Smoothing_Buffer_Descriptor _descriptor);
    virtual void set_stream_identifier_descriptor(Stream_Identifier_Descriptor _descriptor);
    virtual void set_subtitling_descriptor(Subtitling_Descriptor _descriptor);

    void parse_descriptors(const uint8_t *_data, size_t _size);

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const = 0;
#endif
};

};

namespace {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

::mb::Polarity from_dvb_standard_polarity(uint8_t _polarity)
{
    switch(_polarity)
    {
        case 0b00:
            return ::mb::Polarity::Horizontal;

        case 0b01:
            return ::mb::Polarity::Vertical;

        case 0b10:
            return ::mb::Polarity::Left;

        case 0b11:
            return ::mb::Polarity::Right;
    }

    return ::mb::Polarity::UNDEFINED;
}

#pragma GCC diagnostic pop

}

#ifndef NDEBUG
#include "common/mb_globals.h"

namespace std {
template<>
struct hash<set<uint8_t>>
{
    size_t operator()(const set<uint8_t> &s) const noexcept
    {
        size_t result { 0 };

        for(auto e : s)
        {
            mb::hash_combine(result, e);
        }

        return result;
    }
};

template<>
struct hash<mb::Linkage_Descriptor>
{
    size_t operator()(const mb::Linkage_Descriptor &ld) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, ld._transport_stream_id, ld._original_network_id, ld._service_id, ld._linkage_type, ld._zone_ids);
        return result;
    }
};

template<>
struct hash<vector<mb::Linkage_Descriptor>>
{
    size_t operator()(const vector<mb::Linkage_Descriptor> &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &e : v)
        {
            mb::hash_combine(result, e);
        }

        return result;
    }
};

template<>
struct hash<mb::Multilingual_Service_Name::Name>
{
    size_t operator()(const mb::Multilingual_Service_Name::Name &v) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, v.language, v.service_provider_name, v.service_name);
        return result;
    }
};

template<>
struct hash<vector<mb::Multilingual_Service_Name::Name>>
{
    size_t operator()(const vector<mb::Multilingual_Service_Name::Name> &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &e : v)
        {
            mb::hash_combine(result, e);
        }

        return result;
    }
};

template<>
struct hash<mb::Multilingual_Service_Name>
{
    size_t operator()(const mb::Multilingual_Service_Name &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &e : v._names)
        {
            mb::hash_combine(result, e);
        }

        return result;
    }
};

template<>
struct hash<vector<mb::Multilingual_Service_Name>>
{
    size_t operator()(const vector<mb::Multilingual_Service_Name> &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &e : v)
        {
            mb::hash_combine(result, e);
        }

        return result;
    }
};

template<>
struct hash<mb::Service_List_Descriptor::Service>
{
    size_t operator()(const mb::Service_List_Descriptor::Service &v) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, v.service_id, v.service_type, v.viewer_channel, v.regionalizacao);
        return result;
    }
};

template<>
struct hash<vector<mb::Service_List_Descriptor::Service>>
{
    size_t operator()(const vector<mb::Service_List_Descriptor::Service> &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &s : v)
        {
            mb::hash_combine(result, s);
        }

        return result;
    }
};

template<>
struct hash<mb::Service_List_Descriptor>
{
    size_t operator()(const mb::Service_List_Descriptor &v) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, v);
        return result;
    }
};

template<>
struct hash<mb::Satellite_Delivery_System_Descriptor>
{
    size_t operator()(const mb::Satellite_Delivery_System_Descriptor &v) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, v._frequency, v._orbital_position, v._west_east_flag,
                         v._polarization, v._roll_off, v._modulation_system, v._modulation_type,
                         v._symbol_rate, v._FEC_inner);
        return result;
    }
};

template<>
struct hash<mb::S2_Satellite_Delivery_System_Descriptor>
{
    size_t operator()(const mb::S2_Satellite_Delivery_System_Descriptor &v) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, v._descriptor_tag_extension, v._receiver_profiles, v._s2x_mode,
                         v._scrambling_sequence_selector, v._ts_gs_s2x_mode, v._frequency,
                         v._orbital_position, v._west_east_flag, v._polarization, v._multiple_input_stream_flag,
                         v._roll_off, v._symbol_rate);
        return result;
    }
};

}
#endif
