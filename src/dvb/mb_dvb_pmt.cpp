#include "mb_dvb_pmt.h"
#include "common/mb_globals.h"

namespace mb {

PMT::PMT():
    _program_number{ 0 },
    _version_number{ 0 },
    _current_next_indicator{ 0 },
    _section_number{ 0 },
    _last_section_number{ 0 },
    _pcr_pid{ 0 },
    _component_tag{ 0 }
{
}

PMT::PMT(PMT &&_other):
    _program_number(_other._program_number),
    _version_number(_other._version_number),
    _current_next_indicator(_other._current_next_indicator),
    _section_number(_other._section_number),
    _last_section_number(_other._last_section_number),
    _pcr_pid(_other._pcr_pid),
    _ca_types(std::move(_other._ca_types)),
    _component_tag(_other._component_tag),
    _pmt_section_data(std::move(_other._pmt_section_data)),
    _streams(std::move(_other._streams))
{
}

PMT &PMT::operator=(PMT &&_other)
{
    _program_number = _other._program_number;
    _version_number = _other._version_number;
    _current_next_indicator = _other._current_next_indicator;
    _section_number = _other._section_number;
    _last_section_number = _other._last_section_number;
    _pcr_pid = _other._pcr_pid;
    _ca_types = std::move(_other._ca_types);
    _component_tag = _other._component_tag;
    _pmt_section_data = std::move(_other._pmt_section_data);
    _streams = std::move(_other._streams);
    return *this;
}

PMT::PMT(uint8_t **_data, size_t _size)
{
    auto p_data = *_data;
    auto p_size = _size;
    mb_assert(_size >= 12);
    mb_assert(table_id() == p_data[0]);

    if(_size < 12)
    {
        return;
    }

    if(table_id() != p_data[0])
    {
        return;
    }

    // section_syntax_indicator: The section_syntax_indicator is a 1-bit field which shall be set to "1".
    auto section_syntax_indicator = p_data[1] & 0b10000000;

    if(!section_syntax_indicator)
    {
        return;
    }

    auto section_length = ((p_data[1] & 0b00001111) << 8) | p_data[2];
    auto end = p_data + section_length + 3;
    if(end > p_data + p_size)
    {
        return;
    }

    _program_number = (p_data[3] << 8) | p_data[4];
    _version_number = (p_data[5] & 0b00111110) >> 1;
    _current_next_indicator = (p_data[5] & 0b00000001);
    _section_number = p_data[6];
    _last_section_number = p_data[7];
    _pcr_pid = ((p_data[8] & 0b00011111) << 8) | p_data[9];
    uint16_t program_info_size = ((p_data[10] & 0b00000011) << 8) | p_data[11];
    p_data += 12;
    p_size -= 12;
    if (p_size < 4 || program_info_size > p_size - 4)
    {
        return;
    }

    if(program_info_size)
    {
        parse_descriptors(p_data, program_info_size);
        p_data += program_info_size;
        p_size -= program_info_size;

    }

    while(p_size >= 5 + 4 && p_data + 5 < end)

    {
        size_t es_info_length = p_size - 5 - 4;
        _streams.emplace_back(this, p_data, es_info_length);
        p_data += es_info_length + 5;
        p_size -= es_info_length + 5;
    }

    _pmt_section_data.assign(*_data, _size);
    *_data = nullptr;
}

void PMT::set_ca_descriptor(CA_Descriptor _descriptor)
{
    CA_Info info;
    info.id = _descriptor.ca_system_id();
    info.pid = _descriptor.pid();

    switch(_descriptor.ca_system_id())
    {
        case 0x1712:
            info.type = CA_Type::Verimatrix;
            break;

        case 0x1802:
        case 0x182B:
        case 0x186A:
        case 0x1870:
        case 0x1871:
            info.type = CA_Type::Nagra;
            break;

        default:
            info.type = CA_Type::Unknown;
            break;
    }

    _ca_types.push_back(std::move(info));
}

void PMT::set_registration_descriptor(Registration_Descriptor _descriptor)
{
    auto format = _descriptor.format();

    switch(format[0])
    {
    }

    DEBUG_MSG(DVB, ERROR, "Stream type not handled: " << format << endl);
}

void PMT::set_subtitling_descriptor(Subtitling_Descriptor _descriptor)
{
    _dvb_subtitle = _descriptor.move_subtitle();
}

PMT::Stream::Stream(PMT *_parent, const uint8_t *_data, size_t &_size):
    m_parent(_parent)
{
    stream_type = _data[0];
    pid = ((_data[1] & 0b00011111) << 8) | _data[2];
    uint16_t this_size = ((_data[3] & 0b00000011) << 8) | _data[4];
    if(this_size > _size)
    {
        _size = 0;
        return;
    }
    _size = this_size;
    parse_descriptors(_data + 5, _size);
}

void PMT::Stream::set_multilingual_service_name(Multilingual_Service_Name _descriptor)
{
    DEBUG_MSG(DVB, ERROR, "Unused Multilingual Service Names: ");
    auto names = _descriptor.move_names();

    for(const auto &n : names)
    {
        DEBUG_MSG(DVB, ERROR, "\n" << n.language << " " << n.service_name << " " << n.service_provider_name);
    }

    DEBUG_MSG(DVB, ERROR, "\n");
}

void PMT::Stream::set_registration_descriptor(Registration_Descriptor _descriptor)
{
    auto format = _descriptor.format();

    switch(format[0])
    {
        case 'A':
            if(strncmp(format, "AC-3", 4) == 0)
            {
                if(stream_type != 0)
                {
                    stream_sub_type =  STREAM_TYPE_AC3_AUDIO;
                }
                else
                {
                    stream_type = STREAM_TYPE_AC3_AUDIO;
                }

                return;
            }
    }

    DEBUG_MSG(DVB, ERROR, "Stream type not handled: " << format << endl);
}

void PMT::Stream::set_subtitling_descriptor(Subtitling_Descriptor _descriptor)
{
    m_parent->set_subtitling_descriptor(_descriptor);
    dvb_subtitle_present = true;
}

void PMT::Stream::set_ca_descriptor(CA_Descriptor _descriptor)
{
    m_parent->set_ca_descriptor(_descriptor);
}

#ifndef NDEBUG
void PMT::Stream::hash(std::size_t &seed) const
{
    hash_combine(seed, stream_type, pid, component_tag, audio_type, language_code, regionalizacao, viewer_channel, dvb_subtitle_present);
}

void PMT::hash(std::size_t &seed) const
{
    hash_combine(seed, _program_number, _version_number, _current_next_indicator,
                 _section_number, _last_section_number, _pcr_pid, _ca_types,
                 _component_tag);

    for(const auto &s : _streams)
    {
        s.hash(seed);
    }
}

#endif

} // namespace mb
