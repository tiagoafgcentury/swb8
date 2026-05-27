#pragma once

#include <cstdint>

#include "mb_dvb_si_table.h"
#include "mb_dvb_idescriptor_interface.h"
#include "mb_dvb_itransport_stream_interface.h"
#include "mb_dvb_globals.h"

namespace mb {

class BAT final : public SI_Table, public IDescriptor_Interface, public ITransport_Stream_Interface
{
private:
    Bouquet_ID_t _bouquet_id { 0 };
    uint8_t _version_number: 5 { 0 };
    uint8_t _current_next_indicator: 1 { 0 };
    Section_Number_t _section_number { 0 };
    uint8_t _last_section_number { 0 };

    std::string _bouquet_name;

    std::vector<Linkage_Descriptor> _linkage_descriptors;

    Viewer_Channel_t _viewer_channel { 0 };

protected:
    virtual void set_bouquet_name(const char *_name, size_t _length) override
    {
        _bouquet_name.assign(_name, _length);
    };

    virtual void set_linkage_descriptor(Linkage_Descriptor _descriptor) override
    {
        _linkage_descriptors.emplace_back(std::move(_descriptor));
    }

    void set_sinalizacao_dos_canais_regionalizados(Sinalizacao_dos_Canais_Regionalizados _descriptor) override
    {
        _viewer_channel = _descriptor.viewer_channel();
    }

public:
    explicit BAT():
        ITransport_Stream_Interface(nullptr)
    {
    }

    explicit BAT(const uint8_t *_data, size_t _size);
    ~BAT() {}

    virtual Table_ID_t table_id() const override
    {
        return BAT_TABLE_ID;
    }

    auto bouquet_id() const
    {
        return _bouquet_id;
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
    auto move_bouquet_name()
    {
        return std::move(_bouquet_name);
    }
    auto move_linkage_descriptors()
    {
        return std::move(_linkage_descriptors);
    }
    auto viewer_channel() const
    {
        return _viewer_channel;
    }

#ifndef NDEBUG
    const auto &peek_bouquet_name()
    {
        return _bouquet_name;
    }

    virtual void hash(std::size_t &seed) const override;

    virtual std::size_t hash() const override
    {
        return SI_Table::hash();
    }
#endif
};

};
