#include "mb_dvb_transport_stream.h"

namespace mb {

Transport_Stream::Transport_Stream(SI_Table *parent, const uint8_t *_data, size_t _size):
    _parent{parent}
{
    mb_assert(_size >= 6);
    _transport_stream_id = (_data[0] << 8) | _data[1];
    _original_network_id = (_data[2] << 8) | _data[3];
    auto transport_descriptors_length { ((_data[4] & 0b00001111) << 8) | _data[5] };
    parse_descriptors(_data + 6, transport_descriptors_length);
}

void Transport_Stream::set_sinalizacao_dos_canais_nao_regionalizados(Sinalizacao_dos_Canais_Nao_Regionalizados _descriptor)
{
    auto services { _descriptor.move_services() };

    for(auto &s : _service_list_descriptors)
    {
        for(const auto &it : services)
        {
            if(s.service_id == it.service_id)
            {
                s.viewer_channel = it.viewer_channel;
                s.regionalizacao = Regionalizacao::NaoRegionalizado;
            }
        }
    }
}

#ifndef NDEBUG
void Transport_Stream::hash(std::size_t &seed) const
{
    hash_combine(seed, _transport_stream_id, _original_network_id, _linkage_descriptors, _service_list_descriptors,
                 _satellite_delivery_system_descriptor, _s2_satellite_delivery_system_descriptor);
}

#endif
} // namespace mb
