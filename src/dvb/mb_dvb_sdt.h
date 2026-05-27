#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "mb_dvb_si_table.h"
#include "mb_dvb_idescriptor_interface.h"
#include "common/mb_types.h"
#include "common/mb_assert.h"

namespace mb {

class SDT final : public SI_Table
{
private:
    Table_ID_t _table_id { 0 };
    TS_ID_t _transport_stream_id { 0 };
    uint8_t _version_number: 5 { 0 };
    uint8_t _current_next_indicator: 1 { 0 };
    Section_Number_t _section_number { 0 };
    Section_Number_t _last_section_number { 0 };
    NID_t _original_network_id { 0 };

    struct Transport_Stream : public IDescriptor_Interface
    {
        Transport_Stream(SDT *_parent, const uint8_t *_data, size_t &_size);
        virtual ~Transport_Stream() {}

        SDT *parent;

        Service_ID_t service_id { 0 };
        uint8_t EIT_schedule_flag: 1 { 0 };
        uint8_t EIT_present_follow_flag: 1 { 0 };
        uint8_t running_status: 3 { 0 };
        uint8_t free_CA_mode: 1 { 0 };
        Service_Type service_type { Service_Type::none };

        std::string service_provider_name;
        std::string service_name;
        std::vector<Multilingual_Service_Name::Name> multilingual_service_names;

        Content_Descriptor::Content_Nibble conent_nibble { Content_Descriptor::Content_Nibble::Undefined_Content };

        std::vector<Linkage_Descriptor> _linkage_descriptors;

        virtual Table_ID_t table_id() const override
        {
            return parent->table_id();
        }

        virtual void set_linkage_descriptor(Linkage_Descriptor _descriptor) override
        {
            _linkage_descriptors.emplace_back(std::move(_descriptor));
        }

        virtual void set_service_descriptor(Service_Descriptor _descriptor) override
        {
            mb_assert(service_provider_name.empty());
            mb_assert(service_name.empty());
            service_provider_name = _descriptor.move_service_provider_name();
            service_name = _descriptor.move_service_name();
            service_type = _descriptor.service_type();
        }

        void set_content_descriptor(Content_Descriptor _descriptor) override
        {
            conent_nibble = _descriptor.content_nibble();
        }

        void set_multilingual_service_name(Multilingual_Service_Name _descriptor) override
        {
            multilingual_service_names = _descriptor.move_names();
        }

        auto move_service_name()
        {
            return std::move(service_name);
        }

        auto move_service_provider_name()
        {
            return std::move(service_provider_name);
        }

#ifndef NDEBUG
        virtual void hash(std::size_t &seed) const override;
#endif
    };

    std::vector<Transport_Stream> _transport_streams;

public:
    explicit SDT()
    {
    }

    explicit SDT(const uint8_t *_data, size_t _size);

    virtual uint8_t table_id() const override
    {
        return _table_id;
    }

    auto move_transport_streams()
    {
        return std::move(_transport_streams);
    }

    auto transport_stream_id() const
    {
        return _transport_stream_id;
    }
    auto version_number() const
    {
        return _version_number;
    }
    auto current_next_indicator() const
    {
        return _current_next_indicator;
    }
    auto section_number() const
    {
        return _section_number;
    }
    auto last_section_number() const
    {
        return _last_section_number;
    }
    auto original_network_id() const
    {
        return _original_network_id;
    }

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const override;

    virtual std::size_t hash() const override
    {
        return SI_Table::hash();
    }
#endif
};

};
