#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

#include "mb_dvb_si_table.h"
#include "mb_dvb_idescriptor_interface.h"
#include "mb_dvb_itransport_stream_interface.h"

namespace mb {

class NIT final : public SI_Table, public IDescriptor_Interface, public ITransport_Stream_Interface
{
private:
    Table_ID_t _table_id { 0 };
    NID_t _network_id: 16 { 0 };
    uint8_t _version_number: 5 { 0 };
    uint8_t _current_next_indicator: 1 { 0 };
    Section_Number_t _section_number { 0 };
    Section_Number_t _last_section_number { 0 };

    std::string _network_name;

    std::vector<Linkage_Descriptor> _linkage_descriptors;

protected:
    virtual void set_network_name(const char *_name, size_t _length) override
    {
        _network_name.assign(_name, _length);
    };

    virtual void set_linkage_descriptor(Linkage_Descriptor _descriptor) override
    {
        _linkage_descriptors.emplace_back(std::move(_descriptor));
    }

public:
    explicit NIT():
        ITransport_Stream_Interface(nullptr)
    {}

    explicit NIT(const uint8_t *_data, size_t _size);
    virtual ~NIT() {}

    virtual Table_ID_t table_id() const override
    {
        return _table_id;
    }
    auto network_id() const
    {
        return _network_id;
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
    auto move_network_name()
    {
        return std::move(_network_name);
    }
    auto move_linkage_descriptors()
    {
        return std::move(_linkage_descriptors);
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
