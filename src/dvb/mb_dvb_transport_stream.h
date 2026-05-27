#pragma once

#include <cstdint>

#include "mb_dvb_si_table.h"
#include "mb_dvb_idescriptor_interface.h"
#include "common/mb_types.h"
#include "common/mb_assert.h"

namespace mb {

class Transport_Stream : public IDescriptor_Interface
{
private:
    TS_ID_t _transport_stream_id { 0 };
    NID_t _original_network_id { 0 };

    SI_Table *_parent { nullptr };

    std::vector<Linkage_Descriptor> _linkage_descriptors;
    Service_List_Descriptor _service_list_descriptors;
    Ud_Sky_Logical_Channel_Descriptor _ud_sky_logical_channel_descriptors;
    Ud_Sky_Service_Order_List_Descriptor _ud_sky_service_order_list_descriptors;

    Satellite_Delivery_System_Descriptor _satellite_delivery_system_descriptor;
    S2_Satellite_Delivery_System_Descriptor _s2_satellite_delivery_system_descriptor;

protected:
    virtual void set_linkage_descriptor(Linkage_Descriptor _descriptor) override
    {
        _linkage_descriptors.emplace_back(std::move(_descriptor));
    }

    void set_service_list_descriptor(Service_List_Descriptor _descriptor) override
    {
        _service_list_descriptors = std::move(_descriptor);
    }

    void set_satellite_delivery_system_descriptor(Satellite_Delivery_System_Descriptor _descriptor) override
    {
        mb_assert(_satellite_delivery_system_descriptor.frequency() == 0);
        mb_assert(_s2_satellite_delivery_system_descriptor.frequency() == 0);
        _satellite_delivery_system_descriptor = std::move(_descriptor);
    }

    void set_s2_satellite_delivery_system_descriptor(S2_Satellite_Delivery_System_Descriptor _descriptor) override
    {
        mb_assert(_satellite_delivery_system_descriptor.frequency() == 0);
        mb_assert(_s2_satellite_delivery_system_descriptor.frequency() == 0);
        _s2_satellite_delivery_system_descriptor = std::move(_descriptor);
    }

    void set_ud_sky_logical_channel_descriptor(Ud_Sky_Logical_Channel_Descriptor _descriptor) override
    {
        _ud_sky_logical_channel_descriptors = std::move(_descriptor);
    }

    void set_ud_sky_service_order_list_descriptor(Ud_Sky_Service_Order_List_Descriptor _descriptor) override
    {
        _ud_sky_service_order_list_descriptors = std::move(_descriptor);
    }

    void set_sinalizacao_dos_canais_nao_regionalizados(Sinalizacao_dos_Canais_Nao_Regionalizados _descriptor) override;

public:
    explicit Transport_Stream(SI_Table *parent, const uint8_t *_data, size_t _size);
    virtual ~Transport_Stream() {}

    virtual Table_ID_t table_id() const override
    {
        return _parent->table_id();
    }

    auto transport_stream_id() const
    {
        return _transport_stream_id;
    };
    auto original_network_id() const
    {
        return _original_network_id;
    };

    auto move_linkage_descriptors()
    {
        return std::move(_linkage_descriptors);
    }
    auto move_service_list_descriptors()
    {
        return std::move(_service_list_descriptors);
    }
    auto move_satellite_delivery_system_descriptor() const
    {
        return std::move(_satellite_delivery_system_descriptor);
    }
    auto move_s2_satellite_delivery_system_descriptor() const
    {
        return std::move(_s2_satellite_delivery_system_descriptor);
    }
    auto move_ud_sky_logical_channel_descriptors()
    {
        return std::move(_ud_sky_logical_channel_descriptors);
    }
    auto move_ud_sky_service_order_list_descriptors()
    {
        return std::move(_ud_sky_service_order_list_descriptors);
    }

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const override;
#endif
};

};
