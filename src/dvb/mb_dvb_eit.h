#pragma once

#include <cstdint>
#include <chrono>
#include <optional>
#include <unordered_set>

#include "mb_dvb_globals.h"
#include "mb_dvb_si_table.h"
#include "mb_dvb_utc_mjd.h"

namespace mb {

class EIT final : public SI_Table
{
private:
    Table_ID_t _table_id { 0 };
    Service_ID_t _service_id { 0 };
    uint8_t _version_number: 5;
    uint8_t _current_next_indicator: 1;
    Section_Number_t _section_number { 0 };
    Section_Number_t _last_section_number { 0 };
    TS_ID_t _transport_stream_id { 0 };
    NID_t _original_network_id { 0 };
    uint8_t _segment_last_section_number { 0 };
    Table_ID_t _last_table_id { 0 };

    struct Event: public IDescriptor_Interface
    {
        Event(EIT *_parent, const uint8_t *_data, size_t &_size);
        virtual ~Event() {};

        EIT *parent;

        Event_ID_t event_id { 0 };
        UTC_MJD start_time;
        std::chrono::seconds duration { 0 };
        uint8_t running_status: 3;
        uint8_t free_CA_mode: 1;
        std::optional<Parental_Rating_Descriptor> parental_rating_descriptor;
        std::optional<Short_Event_Descriptor> short_event_descriptor;
        std::optional<Content_Descriptor> content_descriptor;
        std::optional<Extended_Event_Descriptor> extended_event_descriptor;
        std::optional<CA_RS_Descriptor> ca_rs_descriptor;

        virtual uint8_t table_id() const override
        {
            return parent->table_id();
        }

        uint8_t rating { 0 };

        virtual void set_parental_rating_descriptor(Parental_Rating_Descriptor _descriptor) override;
        virtual void set_short_event_descriptor(Short_Event_Descriptor _descriptor) override;
        virtual void set_content_descriptor(Content_Descriptor _descriptor) override;
        virtual void set_extended_event_descriptor(Extended_Event_Descriptor _descriptor) override;
        virtual void set_linkage_descriptor(Linkage_Descriptor _descriptor) override;

#ifndef NDEBUG
        virtual void hash(std::size_t &seed) const override;
#endif
    };
    std::vector<Event> _events;

    void load_event(const uint8_t *_data, size_t _size);

    static std::unordered_set<EIT_Service_ID_t> s_service_filter;

public:
    explicit EIT(const uint8_t *_data, size_t _size);
    virtual ~EIT() {}

    static void set_service_filter(std::unordered_set<EIT_Service_ID_t> &&_service_filter)
    {
#ifndef NDEBUG
        DEBUG_MSG(DVB, DEBUG, "EPG Service Filter:\n");

        for(const auto &s : _service_filter)
        {
            DEBUG_MSG(DVB, DEBUG, "\t" << (int)s << "\n");
        }

#endif
        s_service_filter = std::move(_service_filter);
    }

    static bool check_service_filter(const uint8_t *_data);

    virtual Table_ID_t table_id() const override
    {
        return _table_id;
    }

    auto service_id() const
    {
        return _service_id;
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

    auto transport_stream_id() const
    {
        return _transport_stream_id;
    }

    auto original_network_id() const
    {
        return _original_network_id;
    }

    auto segment_last_section_number() const
    {
        return _segment_last_section_number;
    }

    auto last_table_id() const
    {
        return _last_table_id;
    }

    auto move_events()
    {
        return std::move(_events);
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
