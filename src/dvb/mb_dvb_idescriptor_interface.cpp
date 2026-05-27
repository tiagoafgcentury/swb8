#include "mb_dvb_idescriptor_interface.h"
#include "common/mb_globals.h"
#include "mb_dvb_globals.h"
#include "common/mb_assert.h"

#include <string.h>

namespace mb {

Audio_Stream_Header_AAC_Descriptor::Audio_Stream_Header_AAC_Descriptor(const uint8_t * /*_data*/, size_t /*_size*/)
{
}

CA_Descriptor::CA_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 4);
    _ca_system_id = (_data[0] << 8) | _data[1];
    _pid = (_data[2] << 8) | _data[3];
}

Content_Descriptor::Content_Descriptor(const uint8_t *_data, size_t _size)
{
    while(_size > 0)
    {
        auto content_nibble { _data[0] };
        //auto user_byte = _data[1];
        _size -= 2;
        _data += 2;

        if((content_nibble & 0b11110000) == 0)
        {
            _content = Content_Nibble::Undefined_Content;
        }
        else if((content_nibble & 0b11110000) == 0b00010000)
        {
            switch(content_nibble & 0b00001111)
            {
                case 0x0:
                    _content = Content_Nibble::Movie_Drama_General;
                    break;

                case 0x1:
                    _content = Content_Nibble::Detective_Thriller;
                    break;

                case 0x2:
                    _content = Content_Nibble::Adventure_Western_War;
                    break;

                case 0x3:
                    _content = Content_Nibble::Science_Fiction_Fantasy_Horror;
                    break;

                case 0x4:
                    _content = Content_Nibble::Comedy;
                    break;

                case 0x5:
                    _content = Content_Nibble::Soap_Melodrama_Folkloric;
                    break;

                case 0x6:
                    _content = Content_Nibble::Romance;
                    break;

                case 0x7:
                    _content = Content_Nibble::Serious_Classical_Religious_Historical_movie_Drama;
                    break;

                case 0x8:
                    _content = Content_Nibble::Adult_Movie_Drama;
                    break;

                default:
                    _content = Content_Nibble::Undefined_Content;
                    break;
            };
        }
        else if((content_nibble & 0b11110000) == 0b00100000)
        {
            switch(content_nibble & 0b00001111)
            {
                case 0x0:
                    _content = Content_Nibble::News_Current_Affairs;
                    break;

                case 0x1:
                    _content = Content_Nibble::News_Weather_Report;
                    break;

                case 0x2:
                    _content = Content_Nibble::News_Magazine;
                    break;

                case 0x3:
                    _content = Content_Nibble::Documentary;
                    break;

                default:
                    _content = Content_Nibble::Undefined_Content;
                    break;
            };
        }
        else
        {
            _content = Content_Nibble::Undefined_Content;
        }
    }
}

Extended_Event_Descriptor::Extended_Event_Descriptor(const uint8_t *_data, size_t /*_size*/)
{
    //auto descriptor_number { (_data[0] & 0b11110000) >> 4 };
    //auto last_descriptor_number { _data[0] & 0b00001111 };
    auto p { _data + 1 };
    strncpy(ISO_639_language_code, reinterpret_cast<const char *>(p), 3);
    p += 3;
    uint8_t length_of_items { *p };
    p += 1;

    while(length_of_items > 0)
    {
        _items.emplace_back();
        auto &i { _items.back() };
        uint8_t item_description_length { *p };
        p += 1;
        length_of_items -= 1;
        i.item_description = convert_iso8859_1(p, item_description_length);
        length_of_items -= item_description_length;
        p += item_description_length;
        uint8_t item_length { *p };
        p += 1;
        length_of_items -= 1;
        i.item = convert_iso8859_1(p, item_length);
        p += item_length;
        length_of_items -= item_length;
    }

    uint8_t text_length { *p };
    p += 1;
    _text = convert_iso8859_1(p, text_length);
}

Language_Descriptor::Language_Descriptor(const uint8_t *_data, size_t _size)
{
    if (_size == 4)
    {
        memcpy(_language_code, &_data[0], sizeof(_language_code));
        _audio_type = _data[3];
    }
}

Linkage_Descriptor::Linkage_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 7);

    if(_size < 7)
    {
        return;
    }

    _transport_stream_id = (_data[0] << 8) | _data[1];
    _original_network_id = (_data[2] << 8) | _data[3];
    _service_id = (_data[4] << 8) | _data[5];
    _linkage_type = _data[6];

    switch(_linkage_type)
    {
        case SATELLITE_DELIVERY_DESCRIPTOR:
        {
            break;
        }

        case CA_REPLACEMENT_SERVICE:
        {
            _ca_rs_descriptor.emplace(_data+7, _size);
            break;
        }

        case LINKAGE_TYPE_SW_UPDATE:
        {
            if(not _ota_swdl_descriptor.has_value())
            {
                _ota_swdl_descriptor.emplace();
            }

            _ota_swdl_descriptor.value().emplace_back(_data, _size);
            break;
        }

        case UD_TVRO_LINKAGE_ZONE_ID:
        {
            _data += 7;
            const uint8_t tvro_header[] {0x09, 0x42, 0x52, 0x41}; // 0x09 "BRA"

            if(memcmp(_data, tvro_header, 4) == 0)
            {
                _data += 4;
                _size -= 11;

                while(_size > 0)
                {
                    uint8_t zone_id_size = static_cast<uint8_t>(_data[0]) + 1u; // +1 == zone_id_size itself

                    if(zone_id_size != 2 or zone_id_size > _size)
                    {
                        DEBUG_MSG(DVB, ERROR, "zone_id_size mismatch: " << dec << (int)zone_id_size << " vs. " << (int)_size << "\n");
                        break;
                    }

                    Zone_ID_t zone_id { _data[1] };
                    _zone_ids.insert(zone_id);
                    _data += zone_id_size;
                    _size -= zone_id_size;
                }
            }
            else
            {
                DEBUG_MSG(DVB, ERROR, "TVRO Header MISMATCH\n");
            }

            break;
        }
    }
}

Local_Time_Offset_Descriptor::Local_Time_Offset_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size % 13 == 0);
    _time_offsets.reserve(_size / 13);

    for(; _size >= 13; _size -= 13, _data += 13)
    {
        _time_offsets.emplace_back(_data);
    }
}

Local_Time_Offset_Descriptor::Time_Offset::Time_Offset(const uint8_t *_data)
{
    memcpy(country_code, _data, 3);
    country_code[3] = 0;
    country_region_id = _data[3] >> 2;
    local_time_offset_polarity = _data[3] & 0b00000001;
    auto hours = ((_data[4] & 0b11110000) * 10) + (_data[4] & 0b00001111);
    auto minutes = ((_data[5] & 0b11110000) * 10) + (_data[5] & 0b00001111);
    local_time_offset = std::chrono::hours(hours) + std::chrono::minutes(minutes);
}

Multilingual_Service_Name::Multilingual_Service_Name(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 4);
    auto end { _data + _size };

    while(_data < end)
    {
        Name name;
        name.language.assign(reinterpret_cast<const char *>(_data), 3);
        auto service_provider_name_length = _data[3];
        _data += 4;
        _size -= 4;

        [[likely]] if(service_provider_name_length > 0 and service_provider_name_length < _size)
        {
            name.service_provider_name.assign(reinterpret_cast<const char *>(_data), service_provider_name_length);
        }

        _data += service_provider_name_length;
        _size -= 1;
        auto service_name_length = _data[0];

        [[likely]] if(service_name_length > 0 and service_name_length <= _size)
        {
            name.service_name.assign(reinterpret_cast<const char *>(_data), service_name_length);
        }

        _data += service_name_length;
        _names.emplace_back(std::move(name));
    }
}

OTA_SWDL_Descriptor::OTA_SWDL_Descriptor(const uint8_t *_data, size_t _size)
{
    auto p { _data };
    [[maybe_unused]] auto end { _data + _size };
    _tsid = (p[0] << 8) | p[1];
    p += 2;
    _onid = (p[0] << 8) | p[1];
    p += 2;
    _svc_id = (p[0] << 8) | p[1];
    p += 2;
    [[maybe_unused]] auto linkage_type = *p++;
    [[maybe_unused]] auto OUI_data_length = *p++;
    _OUI = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += 4;
    //[[maybe_unused]] auto selector_length = *p++;
    _manufacturer_code = *p++;
    _hardware_code = *p++;
    _model_code = *p++;
    _download_mode = *p++;
    _software_version = (p[0] << 8) | p[1];
    p += 2;
    _pid = (p[0] << 8) | p[1];
    p += 2;
    _factory_reset_flag = *p++;
}

CA_RS_Descriptor::CA_RS_Descriptor(const uint8_t *_data, size_t _size)
{
    auto p { _data };
    mb_assert(_size >= 8);
    [[maybe_unused]] auto end { _data + _size };

    _bseid_tag = p[0];
    _bseid_length = p[1];
    _bseid = (p[2] << 8) | p[3];
    _r_osd_dur_tag = p[4];
    _r_osd_dur_length = p[5];
    _r_osd_duration = (p[6] << 8) | p[7];
}

Parental_Rating_Descriptor::Parental_Rating_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 4 && _size % 4 == 0);
    auto end { _data + _size };

    while(_data < end)
    {
        _ratings.emplace_back();
        auto &r { _ratings.back() };
        r.country_code[0] = _data[0];
        r.country_code[1] = _data[1];
        r.country_code[2] = _data[2];
        r.rating = _data[3];
        _data += 4;
    }
}

Registration_Descriptor::Registration_Descriptor(const uint8_t *_data, size_t _size)
{
    MB_ZERO(_format);
    strncpy(_format, reinterpret_cast<const char *>(_data), std::min(_size, sizeof(_format) - 1));
}

S2_Satellite_Delivery_System_Descriptor::S2_Satellite_Delivery_System_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 14);
    _descriptor_tag_extension = _data[0];
    _receiver_profiles = _data[1] >> 3;
    _s2x_mode = (_data[2] & 0b11000000) >> 6;
    _scrambling_sequence_selector = (_data[2] & 0b00100000) >> 5;
    _ts_gs_s2x_mode = _data[2] & 0b00000011;
    _frequency = ((bcd2dec(_data[3]) * 1000000) + (bcd2dec(_data[4]) * 10000) + (bcd2dec(_data[5]) * 100) + bcd2dec(_data[6])) * 10;
    _orbital_position = bcd2dec((_data[7] << 8) * 100) + bcd2dec(_data[8]);
    _west_east_flag = (_data[9] & 0b10000000) >> 7;
    _polarization = (_data[9] & 0b01100000) >> 5;
    _multiple_input_stream_flag = (_data[9] & 0b00010000) >> 4;
    _roll_off = (_data[9] & 0b00001110) >> 1;
    _symbol_rate = ((bcd2dec(_data[10]) * 1000000) + (bcd2dec(_data[11]) * 10000) + (bcd2dec(_data[12]) * 100) + bcd2dec((_data[13] & 0b11110000) >> 4)) / 10;
}

Satellite_Delivery_System_Descriptor::Satellite_Delivery_System_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 11);
    _frequency = ((bcd2dec(_data[0]) * 1000000) + (bcd2dec(_data[1]) * 10000) + (bcd2dec(_data[2]) * 100) + bcd2dec(_data[3])) * 10;
    _orbital_position = bcd2dec((_data[4] << 8) * 100) + bcd2dec(_data[5]);
    _west_east_flag = (_data[6] & 0b10000000) >> 7;
    _polarization = (_data[6] & 0b01100000) >> 5;
    _roll_off = (_data[6] & 0b00011000) >> 3;
    _modulation_system = (_data[6] & 0b00000100) >> 2;
    _modulation_type = (_data[6] & 0b00000011);
    _symbol_rate = ((bcd2dec(_data[7]) * 1000000) + (bcd2dec(_data[8]) * 10000) + (bcd2dec(_data[9]) * 100) + bcd2dec((_data[10] & 0b11110000) >> 4)) / 100;
    _FEC_inner = _data[10] & 0b00001111;
}

Service_Descriptor::Service_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 3);
    _service_type = _data[0];
    auto service_provider_name_length { _data[1] };

    if(service_provider_name_length > 0)
    {
        _service_provider_name = convert_iso8859_1(&_data[2], service_provider_name_length);
    }

    _data += service_provider_name_length + 2;
    auto service_name_length { _data[0] };

    if(service_name_length > 0)
    {
        _service_name = convert_iso8859_1(&_data[1], service_name_length);
    }
}

Service_List_Descriptor::Service_List_Descriptor(const uint8_t *_data, size_t _size)
{
    const auto descriptor_size { (16u + 8u) / 8u };
    _services.reserve(_size / descriptor_size);

    while(_size >= descriptor_size)
    {
        _services.emplace_back((_data[0] << 8) | _data[1], _data[2]);
        _data += descriptor_size;
        _size -= descriptor_size;
    }
}

Short_Event_Descriptor::Short_Event_Descriptor(const uint8_t *_data, size_t _size)
{
    if (_size < 4)
    {
        return;
    }

    auto p { _data };
    auto end { _data + _size };
    strncpy(ISO_639_language_code, reinterpret_cast<const char *>(p), 3);
    auto event_name_length { _data[3] };
    p += 4;

    if (p + event_name_length > end)
    {
        return;
    }

    event_name = convert_iso8859_1(p, event_name_length);
    p += event_name_length;

    if (p >= end)
    {
       return;
    }

    auto text_length { *p };
    p += 1;

    if (text_length > 0 and p + text_length <= end)
    {
        text = convert_iso8859_1(p, text_length);
    }
}

Sinalizacao_dos_Canais_Regionalizados::Sinalizacao_dos_Canais_Regionalizados(const uint8_t *_data, size_t _size)
{
    mb_assert(_size == 2);
    _viewer_channel = (_data[0] << 8) | _data[1];
}

Sinalizacao_dos_Canais_Nao_Regionalizados::Sinalizacao_dos_Canais_Nao_Regionalizados(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 4 && _size % 4 == 0);

    while(_size >= 4)
    {
        auto service_id { (_data[0] << 8) | _data[1] };
        auto viewer_channel { (_data[2] << 8) | _data[3] };
        _services.emplace_back(service_id, viewer_channel);
        _data += 4;
        _size -= 4;
    }
}

Ud_Sky_Logical_Channel_Descriptor::Ud_Sky_Logical_Channel_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 4 && _size % 4 == 0);

    while(_size >= 4)
    {
        auto service_id { (_data[0] << 8) | _data[1] };
        auto viewer_channel { (_data[2] << 8) | _data[3] };
        _services.emplace_back(service_id, viewer_channel);
        _data += 4;
        _size -= 4;
    }
}

Ud_Sky_Service_Order_List_Descriptor::Ud_Sky_Service_Order_List_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 5 && _size % 5 == 0);

    while(_size >= 5)
    {
        auto service_id { (_data[0] << 8) | _data[1] };
        auto general_order { (_data[2] << 8) | (_data[3] & 0b11110000) };
        auto order_by_type { ((_data[3] & 0b00001111) << 8) | (_data[4]) };
        _services.emplace_back(service_id, general_order, order_by_type);
        _data += 5;
        _size -= 5;
    }
}

Stream_Identifier_Descriptor::Stream_Identifier_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size == 1);
    _component_tag = _data[0];
}

Subtitling_Descriptor::Subtitling_Descriptor(const uint8_t *_data, size_t _size)
{
    mb_assert(_size >= 8 && _size % 8 == 0);
    auto end { _data + _size };

    while(_data < end)
    {
        _subtitles.emplace_back();
        auto &s { _subtitles.back() };
        s.iso639_language_code[0] = _data[0];
        s.iso639_language_code[1] = _data[1];
        s.iso639_language_code[2] = _data[2];
        s.subtitling_type  = _data[3];
        s.composition_page_id = (_data[4] << 8) | _data[5];
        s.ancillary_page_id =  (_data[6] << 8) | _data[7];
        _data += 8;
    }
}

void IDescriptor_Interface::parse_descriptors(const uint8_t *_data, size_t _size)
{
    for(uint8_t descriptor_length = 0; _size > 2; _data += descriptor_length, _size -= descriptor_length)
    {
        auto descriptor_tag { _data[0] };
        descriptor_length = _data[1];
        //mb_assertdescriptor_tag >= 0x40 || (descriptor_tag >= 0x80 && descriptor_tag < 0xFF));
        _size -= 2;

        if(descriptor_length > _size)
        {
            DEBUG_MSG(DVB, ERROR, "Descriptor Length greater then available data = " << dec << (int)descriptor_length << " > " << (int)(_size) << endl);
            return;
        }

        _data += 2;

        switch(descriptor_tag)
        {
            case REGISTRATION_DESCRIPTOR:
                set_registration_descriptor(Registration_Descriptor(_data, descriptor_length));
                break;

            // case VIDEO_STREAM_HEADER_H264:
            // case SYSTEM_CLOCK_EXTERNAL_REFERENCE:
            // case MAXIMUM_BIT_RATE:
            // case SMOOTHING_BUFFER:
            // case AUDIO_STREAM_HEADER_MPEG4:
            // case AUDIO_STREAM_HEADER_AAC:
            // case CAPTION_SERVICE_DESCRIPTOR:
            //     break;

            case CA_DESCRIPTOR:
                set_ca_descriptor(CA_Descriptor(_data, descriptor_length));
                break;

            case LANGUAGE_DESCRIPTOR:
                set_language_descriptor(Language_Descriptor(_data, descriptor_length));
                break;

            case NETWORK_NAME_DESCRIPTOR:
                set_network_name(reinterpret_cast<const char *>(_data), descriptor_length);
                break;

            case SERVICE_LIST_DESCRIPTOR:
                set_service_list_descriptor(Service_List_Descriptor(_data, descriptor_length));
                break;

            // case STUFFING_DESCRIPTOR:
            //     DEBUG_MSG("\t\tSTUFFING_DESCRIPTOR\n");
            //     break;

            case SATELLITE_DELIVERY_DESCRIPTOR:
                set_satellite_delivery_system_descriptor(Satellite_Delivery_System_Descriptor(_data, descriptor_length));
                break;

            // case CABLE_DELIVERY_DESCRIPTOR:
            //     DEBUG_MSG("\t\tCABLE_DELIVERY_DESCRIPTOR\n");
            //     break;

            case BOUQUET_NAME_DESCRIPTOR:
                set_bouquet_name(reinterpret_cast<const char *>(_data), descriptor_length);
                break;

            case SERVICE_DESCRIPTOR:
                set_service_descriptor(Service_Descriptor(_data, descriptor_length));
                break;

            // case COUNTRY_AVAILABILITY_DESCRIPTOR:
            //     DEBUG_MSG("\t\tCOUNTRY_AVAILABILITY_DESCRIPTOR\n");
            //     break;

            case LINKAGE_DESCRIPTOR:
                set_linkage_descriptor(Linkage_Descriptor(_data, descriptor_length));
                break;

            // case NVOD_REFERENCE_DESCRIPTOR:
            //     DEBUG_MSG("\t\tNVOD_REFERENCE_DESCRIPTOR\n");
            //     break;

            // case TIME_SHIFTED_SERVICE_DESCRIPTOR:
            //     DEBUG_MSG("\t\tTIME_SHIFTED_SERVICE_DESCRIPTOR\n");
            //     break;

            case SHORT_EVENT_DESCRIPTOR:
                set_short_event_descriptor(Short_Event_Descriptor(_data, descriptor_length));
                break;

            case EXTENDED_EVENT_DESCRIPTOR:
                set_extended_event_descriptor(Extended_Event_Descriptor(_data, descriptor_length));
                break;

            // case TIME_SHIFTED_EVENT_DESCRIPTOR:
            //     DEBUG_MSG("\t\tTIME_SHIFTED_EVENT_DESCRIPTOR\n");
            //     break;

            // case COMPONENT_DESCRIPTOR:
            //     DEBUG_MSG("\t\tCOMPONENT_DESCRIPTOR\n");
            //     break;

            // case MOSAIC_DESCRIPTOR:
            //     DEBUG_MSG("\t\tMOSAIC_DESCRIPTOR\n");
            //     break;

            case STREAM_IDENTIFIER_DESCRIPTOR:
                set_stream_identifier_descriptor(Stream_Identifier_Descriptor(_data, descriptor_length));
                break;

            // case CA_IDENTIFIER_DESCRIPTOR:
            //     DEBUG_MSG("\t\tCA_IDENTIFIER_DESCRIPTOR\n");
            //     break;

            case CONTENT_DESCRIPTOR:
                set_content_descriptor(Content_Descriptor(_data, descriptor_length));
                break;

            case PARENTAL_RATING_DESCRIPTOR:
                set_parental_rating_descriptor(Parental_Rating_Descriptor(_data, descriptor_length));
                break;

            // case TELETEXT_DESCRIPTOR:
            //     DEBUG_MSG_NL(DVB, INFO, "\t\tTELETEXT_DESCRIPTOR\n");
            //     break;

            // case TELEPHONE_DESCRIPTOR:
            //     DEBUG_MSG("\t\tTELEPHONE_DESCRIPTOR\n");
            //     break;

            case LOCAL_TIME_OFFSET_DESCRIPTOR:
                set_local_time_offset_descriptor(Local_Time_Offset_Descriptor(_data, descriptor_length));
                break;

            case SUBTITLING_DESCRIPTOR:
                set_subtitling_descriptor(Subtitling_Descriptor(_data, descriptor_length));
                break;

            // case TERRESTRIAL_DELIVERY_DESCRIPTOR:
            //     DEBUG_MSG("\t\tTERRESTRIAL_DELIVERY_DESCRIPTOR\n");
            //     break;

            // case MULTILINGUAL_NETWORK_NAME_DESCRIPTOR:
            //     DEBUG_MSG("\t\tMULTILINGUAL_NETWORK_NAME_DESCRIPTOR\n");
            //     break;

            // case MULTILINGUAL_BOUQUET_NAME_DESCRIPTOR:
            //     DEBUG_MSG("\t\tMULTILINGUAL_BOUQUET_NAME_DESCRIPTOR\n");
            //     break;

            case MULTILINGUAL_SERVICE_NAME_DESCRIPTOR:
                set_multilingual_service_name(Multilingual_Service_Name(_data, descriptor_length));
                break;

            // case MULTILINGUAL_COMPONENT_DESCRIPTOR:
            //     DEBUG_MSG("\t\tMULTILINGUAL_COMPONENT_DESCRIPTOR\n");
            //     break;

            // case PRIVATE_DATA_SPECIFIER_DESCRIPTOR:
            //     break;

            // case SERVICE_MOVE_DESCRIPTOR:
            //     DEBUG_MSG("\t\tSERVICE_MOVE_DESCRIPTOR\n");
            //     break;

            // case SHORT_SMOOTHING_BUFFER_DESCRIPTOR:
            //     DEBUG_MSG("\t\tSHORT_SMOOTHING_BUFFER_DESCRIPTOR\n");
            //     break;

            // case FREQUENCY_LIST_DESCRIPTOR:
            //     DEBUG_MSG("\t\tFREQUENCY_LIST_DESCRIPTOR\n");
            //     break;

            // case PARTIAL_TRANSPORT_STREAM_DESCRIPTOR:
            //     DEBUG_MSG("\t\tPARTIAL_TRANSPORT_STREAM_DESCRIPTOR\n");
            //     break;

            // case DATA_BROADCAST_DESCRIPTOR:
            //     DEBUG_MSG("\t\tDATA_BROADCAST_DESCRIPTOR\n");
            //     break;

            // case CA_SYSTEM_DESCRIPTOR:
            //     DEBUG_MSG("\t\tCA_SYSTEM_DESCRIPTOR\n");
            //     break;

            // case DATA_BROADCAST_ID_DESCRIPTOR:
            //     DEBUG_MSG("\t\tDATA_BROADCAST_ID_DESCRIPTOR\n");
            //     break;

            case S2_SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR:
            case DRM_DESCRIPTOR:
                set_s2_satellite_delivery_system_descriptor(S2_Satellite_Delivery_System_Descriptor(_data, descriptor_length));
                break;

            case SINALIZACAO_DOS_CANAIS_NAO_REGIONALIZADOS:
                set_sinalizacao_dos_canais_nao_regionalizados(Sinalizacao_dos_Canais_Nao_Regionalizados(_data, descriptor_length));
                break;

            case SINALIZACAO_DOS_CANAIS_REGIONALIZADOS:
                set_sinalizacao_dos_canais_regionalizados(Sinalizacao_dos_Canais_Regionalizados(_data, descriptor_length));
                break;

            case UD_SKY_LOGICAL_CHANNEL_DESCRIPTOR:
                set_ud_sky_logical_channel_descriptor(Ud_Sky_Logical_Channel_Descriptor(_data, descriptor_length));
                break;

            case UD_SKY_SERVICE_ORDER_LIST_DESCRIPTOR:
                set_ud_sky_service_order_list_descriptor(Ud_Sky_Service_Order_List_Descriptor(_data, descriptor_length));
                break;

            default:
                break;
        }
    }
}

void IDescriptor_Interface::set_bouquet_name(const char * /*_name*/, size_t /*_length*/)
{
    DEBUG_MSG(DVB, WARN, "Bouquet Name not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
};

void IDescriptor_Interface::set_network_name(const char * /*_name*/, size_t /*_length*/)
{
    DEBUG_MSG(DVB, WARN, "Network Name not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
};

void IDescriptor_Interface::set_language_descriptor(Language_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Language Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_linkage_descriptor(Linkage_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Linkage not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
};

void IDescriptor_Interface::set_local_time_offset_descriptor(Local_Time_Offset_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Local Time Offset not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_parental_rating_descriptor(Parental_Rating_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Parental Rating not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_multilingual_service_name(Multilingual_Service_Name /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Multilingual Service Name not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_satellite_delivery_system_descriptor(Satellite_Delivery_System_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Satellite Delivery System not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_s2_satellite_delivery_system_descriptor(S2_Satellite_Delivery_System_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "S2 Satellite Delivery System not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_service_list_descriptor(Service_List_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Service List not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_service_descriptor(Service_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Service Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_sinalizacao_dos_canais_regionalizados(Sinalizacao_dos_Canais_Regionalizados /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Sinalizacao dos Canais Regionalizados not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_sinalizacao_dos_canais_nao_regionalizados(Sinalizacao_dos_Canais_Nao_Regionalizados /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Sinalizacao dos Canais Não Regionalizados not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_ud_sky_logical_channel_descriptor(Ud_Sky_Logical_Channel_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "User Define Sky Logical Channel Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_ud_sky_service_order_list_descriptor(Ud_Sky_Service_Order_List_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "User Define Sky Service Order List Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_stream_identifier_descriptor(Stream_Identifier_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Stream Identifier Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_audio_stream_header_aac_descriptor(Audio_Stream_Header_AAC_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Audio Stream Header AAC Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_audio_stream_header_mpeg4_descriptor(Audio_Stream_Header_Mpeg4_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Audio Stream Header MPEG4 Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_smoothing_buffer_descriptor(Smoothing_Buffer_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Smoothing Buffer Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_ca_descriptor(CA_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "CA Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_caption_service_descriptor(Caption_Service_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Caption Service Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_maximum_bit_rate_descriptor(Maximum_Bit_Rate_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Maximum Bit Rate Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_content_descriptor(Content_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Content Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_extended_event_descriptor(Extended_Event_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Extended Event Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_short_event_descriptor(Short_Event_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Short Event Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_registration_descriptor(Registration_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Short Registration Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

void IDescriptor_Interface::set_subtitling_descriptor(Subtitling_Descriptor /*_descriptor*/)
{
    DEBUG_MSG(DVB, WARN, "Subtitling Descriptor not handled for table: 0x" << hex << setw(2) << setfill('0') << (int)table_id() << endl);
    //mb_assertfalse);
}

} // namespace mb
