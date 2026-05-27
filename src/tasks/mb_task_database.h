
#pragma once

#include "mb_task.h"
#include "common/mb_types.h"
#include "common/mb_lineup.h"

#include "hal/mb_tuner_signal_info.h"
#include "hal/mb_system.h"
#include "dvb/mb_dvb_utc_mjd.h"


#include <atomic>

typedef struct sqlite3 sqlite3;

namespace mb {

class Task_Database final: public Task
{
    friend class Task;

private:
    enum State
    {
        ST_STARTING,
        ST_IDLE,
        ST_LINEUP_SAVE,
    };
    std::atomic<State> m_state { ST_STARTING };

    struct Data;
    std::unique_ptr<Data> m_p;
    bool m_first_open { true };

    sqlite3 *open_local_db();

    void load_lineup();
    void save_lineup();
    void verify_next_schedule();

    void abort_save();
    void populate_satellite(sqlite3 *db);
    bool m_save_needs_reschedule { false };

    typedef std::chrono::system_clock::time_point Time_Point;
    Time_Point m_next_agendamento = Time_Point::max();

    static constexpr auto time_start_offset = 20;

protected:
    virtual void handle_event_application_state_load() override;
    virtual void handle_event_application_state_save(const Event_Save_Application_State &_event) override;
    virtual void handle_event_easy_install_save(bool _easy_install_finish) override;
    virtual void handle_event_lnbf_config_save(const Event_LNBF_Params &_event) override;
    virtual void handle_event_lineup_load() override;
    virtual void handle_event_lineup_save() override;
    virtual void handle_event_lineup_save_zone_id(Satellite_Operator _operator, Zone_ID_t _zone_id) override;
    virtual void handle_event_add_satellite(Satellite satellite) override;
    virtual void handle_event_update_satellite(Satellite satellite) override;
    virtual void handle_event_delete_satellite(unsigned int id) override;
    virtual void handle_event_satellite_list_load(std::function<void(std::vector<Satellite> &satellites)>) override;
    virtual void handle_event_update_channel_list(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_data) override;
    virtual void handle_event_service_favorite_changed(Service *_srv) override;
    virtual void handle_event_system_display_settings_load() override;
    virtual void handle_event_system_display_settings_save(const Event_System_Settings &_event) override;
    virtual void handle_event_pin_reset() override;
    virtual void handle_event_schedule_load(std::function<void(std::vector<ScheduleEntry> scheds)> _schedule) override;
    virtual void handle_event_insert_schedule(ScheduleEntry entry) override;
    virtual void handle_event_delete_schedule(int agenda_id) override;
    virtual void handle_event_update_schedule(ScheduleEntry entry) override;
    virtual void handle_event_delete_all_services() override;

public:
    Task_Database();
    virtual ~Task_Database();

    virtual void process() override;

    bool is_empty();
};

}
