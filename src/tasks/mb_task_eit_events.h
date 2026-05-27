#pragma once

#include "common/mb_types.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "dvb/mb_dvb_idescriptor_interface.h"
#include "mb_task.h"

#include <chrono>
#include <string>
#include <chrono>
#include <unordered_map>
#include <optional>
#include <tuple>

namespace mb {

class Task_EIT_Events final: public Task
{
    friend class Task;

public:
    struct EIT_Event
    {
        std::string short_event_descriptor;
        std::optional<Content_Descriptor::Content_Nibble> content_descriptor;
        std::string extended_event_descriptor;
        uint8_t parental_rating;
        Event_ID_t event_id { 0 };
        UTC_MJD start_time;
        std::chrono::seconds duration { 0 };
        std::optional<uint16_t> ca_rs_descriptor_bseid;
    };
    typedef std::vector<EIT_Event> EIT_Events;

    struct EIT_Service_Events
    {
        EIT_Events events;
    };

    typedef std::unordered_map<EIT_Service_ID_t, EIT_Service_Events> EIT_Programs_Database;

protected:
    bool m_do_update_eit = false;
    std::chrono::steady_clock::time_point m_last_eit_update;

    void handle_event_application_ready() override;
    void handle_event_lineup_ready(const Event_Lineup_Ready &_event) override;
    void handle_event_transponder_locked(const Event_Tuner_Lock &_event) override;
    void handle_event_system_memory_is_low() override;
    void update_eit();

public:
    Task_EIT_Events();
    virtual ~Task_EIT_Events();

    virtual void process() override;

    static EIT_Programs_Database s_eit_programs;

    static std::tuple<std::optional<EIT_Event>, std::optional<EIT_Event>> get_event_for_service(const Service *_srv);
    static Task_EIT_Events::EIT_Events get_all_events_for_service(const Service *_srv);
    static std::map<std::string, std::string> get_event_for_service_str(const Service *_srv);
};

} // namespace mb
