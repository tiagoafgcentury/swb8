#pragma once

#include <cstdint>
#include <cstring>

#include "mb_dvb_globals.h"
#include "mb_dvb_si_table.h"
#include "mb_dvb_idescriptor_interface.h"
#include "common/mb_types.h"

namespace mb {

class PMT final : public SI_Table, public IDescriptor_Interface
{
private:
    Program_Number_t _program_number { 0 };
    uint8_t _version_number: 5;
    uint8_t _current_next_indicator: 1;
    Section_Number_t _section_number { 0 };
    Section_Number_t _last_section_number { 0 };
    PID_t _pcr_pid: 13;
    std::vector<CA_Info> _ca_types;
    std::vector<DVB_Subtitle_Info> _dvb_subtitle;

    uint8_t _component_tag { 0 };

    DVB_Table_Section _pmt_section_data;

    struct Stream : public IDescriptor_Interface
    {
        Stream(PMT *_parent, const uint8_t *_data, size_t &_size);
        virtual ~Stream() {}

        PMT *m_parent = nullptr;
        uint8_t stream_type { 0 };
        uint8_t stream_sub_type { 0 };
        PID_t pid: 13;

        uint8_t component_tag { 0 };

        uint8_t audio_type { 0 };
        char language_code[3 + 1] { 0 };

        Regionalizacao regionalizacao { Regionalizacao::Undefined };
        Viewer_Channel_t viewer_channel { 0 };
        bool dvb_subtitle_present { false };

        virtual Table_ID_t table_id() const override
        {
            return PMT_TABLE_ID;
        }

        virtual void set_stream_identifier_descriptor(Stream_Identifier_Descriptor _descriptor) override
        {
            component_tag = _descriptor.component_tag();
        }

        virtual void set_language_descriptor(Language_Descriptor _descriptor) override
        {
            strncpy(language_code, _descriptor.language_code(), 3);
        }

        virtual void set_multilingual_service_name(Multilingual_Service_Name _descriptor) override;
        virtual void set_registration_descriptor(Registration_Descriptor _descriptor) override;

        virtual void set_subtitling_descriptor(Subtitling_Descriptor _descriptor) override;
        virtual void set_ca_descriptor(CA_Descriptor _descriptor) override;

#ifndef NDEBUG
        virtual void hash(std::size_t &seed) const override;
#endif
    };

    std::vector<Stream> _streams;

    virtual void set_stream_identifier_descriptor(Stream_Identifier_Descriptor _descriptor) override
    {
        _component_tag = _descriptor.component_tag();
    }

    virtual void set_ca_descriptor(CA_Descriptor _descriptor) override;
    virtual void set_registration_descriptor(Registration_Descriptor _descriptor) override;
    virtual void set_subtitling_descriptor(Subtitling_Descriptor _descriptor) override;

public:
    PMT();
    explicit PMT(uint8_t **_data, size_t _size);
    virtual ~PMT() {}

    explicit PMT(PMT &&_other);
    PMT &operator=(PMT &&_other);

    virtual Table_ID_t table_id() const override
    {
        return PMT_TABLE_ID;
    }

    auto move_ca_types() const
    {
        return std::move(_ca_types);
    }

    auto move_dvb_subtitles() const
    {
        return std::move(_dvb_subtitle);
    }

    auto program_number() const
    {
        return _program_number;
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
    auto pcr_pid() const
    {
        return _pcr_pid;
    };
    auto component_tag() const
    {
        return _component_tag;
    }

    auto move_streams()
    {
        return std::move(_streams);
    }

    DVB_Table_Section &pmt_section_data()
    {
        return _pmt_section_data;
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
