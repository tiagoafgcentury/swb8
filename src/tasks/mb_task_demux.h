#pragma once

#include "common/mb_lineup.h"
#include "common/mb_types.h"
#include "dvb/mb_dvb_tables.h"
#include "hal/mb_demux_macros.h"
#include "mb_demux_lineup_claro.h"
#include "mb_demux_channel_list.h"
#include "mb_demux_lineup_sky.h"
#include "mb_demux_table_manager.h"
#include "mb_table_map.h"
#include "mb_task_eit_events.h"

#include "mb_task.h"

#include <bitset>
#include <chrono>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <unordered_set>

namespace mb {

class Demux;

class Task_Demux final: public Task, public IDemux_Table_Manager
{
    friend class Task;
    friend class Demux_Lineup;
    friend class Demux_Channel_List;

public:
    enum State
    {
        ST_IDLE,
        ST_LOCKING,
        ST_LINEUP_RUNNING,
        ST_LINEUP_FINISHED,
        ST_PAT_UPDATING,
        ST_CLOCK_UPDATING,
        ST_START_EMM_FILTERING,
    };

private:
    std::unique_ptr<Demux> m_demux;
    std::unique_ptr<Demux_Lineup> m_demux_lineup;
    std::unique_ptr<Demux_Channel_List> m_channel_list;

    State m_state { ST_IDLE };

    std::chrono::steady_clock::time_point m_last_ota_check;

    void set_state(State _state)
    {
        m_state = _state;
    }

    std::unordered_map<EIT_Service_ID_t, uint8_t> m_eit_services_sections;
    std::weak_ptr<Event_OTA_DSI> m_ota_event_callback;
    std::bitset<32> m_cat_section_seen;

    void check_for_otas();
    void check_for_ota_sky(uint32_t frequency);
    void check_for_ota_claro(uint32_t frequency);

    void cat_callback(PID_t _pid, CAT &&_cat);
    void eit_callback(PID_t _pid, EIT &&_eit);
    void nit_callback(PID_t _pid, NIT &&_nit);
    void ota_callback(PID_t _pid, OTA &&_ota);
    void pat_callback(PID_t _pid, PAT &&_pat);
    void pmt_callback(PID_t _pid, PMT &&_pmt);
    void tdt_callback(PID_t _pid, TDT &&_tot);
    void tot_callback(PID_t _pid, TOT &&_tot);

protected:
    virtual void handle_event_autodetect_lnbf_finished(bool _success) override;
    virtual void handle_event_cas_request_descramble_done(bool _result) override;
    virtual void handle_event_cas_start_emm_filtering(const Transponder *_tp) override;
    virtual void handle_event_clock_need_update() override;
    virtual void handle_event_eit_update() override;
    virtual void handle_event_lineup_ready(const Event_Lineup_Ready &_event) override;
    virtual void handle_event_ota_update_get(PID_t _pid, std::weak_ptr<Event_OTA_DSI> _event) override;
    virtual void handle_event_dvb_subtitle_get(PID_t _pid) override;
    virtual void handle_event_dvb_subtitle_del(PID_t _pid);
    virtual void handle_event_player_started() override;
    virtual void handle_event_service_pmt_get_next_section(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number) override;
    virtual void handle_event_services_update() override;
    virtual void handle_event_transponder_locked(const Event_Tuner_Lock &_event) override;

public:
    Task_Demux();
    virtual ~Task_Demux();

    virtual void handle_event_lineup_build(std::weak_ptr<Event_List_Update> _event, bool restart = true) override;
    virtual void handle_event_lineup_update(std::weak_ptr<Event_List_Update_Channel_List> _event) override;

    virtual void process() override;

    virtual Demux *get_demux() override
    {
        return m_demux.get();
    }

    void cat_table_require(std::optional<uint8_t> _version = {})
    {
        if(_version.has_value())
        {
            DEBUG_MSG(DEMUX, DEBUG, "Require CAT version !" << dec << (int)_version.value() << TERM_RESET << "\n");
            auto cat_table_id = std::make_shared<Demux::TS_Filter_CAT_Next>(CAT_TABLE_ID, _version.value());
            Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(CAT) CAT_TSPID, cat_table_id, true, true, Demux::Data_Type::SECTION));
        }
        else
        {
            DEBUG_MSG(DEMUX, DEBUG, "Require CAT any version " << TERM_RESET << "\n");
            auto cat_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(CAT_TABLE_ID);
            Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(CAT) CAT_TSPID, cat_table_id, true, true, Demux::Data_Type::SECTION));
        }
    }

    void eit_table_require(PID_t _pid = EIT_TSPID)
    {
        auto eit_actual = std::make_shared<Demux::TS_Filter_Table_Id>(EIT_TABLE_ID_PF_ACTUAL);
        Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(EIT) _pid, std::move(eit_actual), false, true, Demux::Data_Type::SECTION));

        //auto eit_other = std::make_shared<Demux::TS_Filter_Table_Id>(EIT_TABLE_ID_PF_OTHER);
        //Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(EIT) _pid, eit_other, false, true));

        for (auto eit_id = EIT_TABLE_ID_SCHEDULE_ACTUAL_LOW; eit_id <= EIT_TABLE_ID_SCHEDULE_ACTUAL_HIGH; eit_id++)
        {
            auto eit_filter = std::make_shared<Demux::TS_Filter_Table_Id>(eit_id);
            Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(EIT) _pid, std::move(eit_filter), false, true, Demux::Data_Type::SECTION));
        }
    }

    void tdt_table_require()
    {
        auto tdt_actual = std::make_shared<Demux::TS_Filter_Table_Id>(TDT_TABLE_ID);
        Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(TDT) TDT_TSPID, tdt_actual, false, false, Demux::Data_Type::SECTION));
    }

    void sdt_table_require()
    {
        auto sdt_actual = std::make_shared<Demux::TS_Filter_Table_Id>(SDT_TABLE_ID_ACTUAL);
        Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(SDT) SDT_TSPID, sdt_actual, false, true, Demux::Data_Type::SECTION));
    }

    void tot_table_require()
    {
        auto tot_actual = std::make_shared<Demux::TS_Filter_Table_Id>(TOT_TABLE_ID);
        Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(TOT) TOT_TSPID, tot_actual, false, true, Demux::Data_Type::SECTION));
    }

    void pat_table_require()
    {
        if(state() != ST_LOCKING)
        {
            auto pat_actual = std::make_shared<Demux::TS_Filter_Table_Id>(PAT_TABLE_ID);
            Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(PAT) PAT_TSPID, pat_actual, false, true, Demux::Data_Type::SECTION));
        }
    }

    void pmt_table_require(PID_t _program_id, Service_ID_t _prog_number, std::optional<uint8_t> _version = {})
    {
        if(_version.has_value())
        {
            DEBUG_MSG(DEMUX, DEBUG, "Require PMT version !" << dec << (int)_version.value() << "\n");
            auto pmt_table_id = std::make_shared<Demux::TS_Filter_PMT_Next>(PMT_TABLE_ID, _prog_number, _version.value());
            Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(PMT) _program_id, pmt_table_id, true, true, Demux::Data_Type::SECTION));
        }
        else
        {
            DEBUG_MSG(DEMUX, DEBUG, "Require PMT any version\n");
            auto pmt_table_id = std::make_shared<Demux::TS_Filter_PMT>(PMT_TABLE_ID, _prog_number);
            Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(PMT) _program_id, pmt_table_id, true, true, Demux::Data_Type::SECTION));
        }
    }

    void ota_table_require(PID_t _pid, int _dsi = 0)
    {
        auto ota_actual = std::shared_ptr<Demux::TS_Filter_Table> {};

        if(_dsi)
        {
            ota_actual = std::make_shared<Demux::TS_Filter_OTA_Message_Id>(OTA_TABLE_ID, 0x1006);
        }
        else
        {
            ota_actual = std::make_shared<Demux::TS_Filter_Table_Id>(OTA_TABLE_ID);
        }

        Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(OTA) _pid, ota_actual, false, true, Demux::Data_Type::SECTION));
    }

    void dvb_subtitle_table_require(PID_t _pid)
    {
        auto dvb_sub_actual = std::make_shared<Demux::TS_Filter_Table_Id>(0x00);
        Task::post_event(std::bind(&IDemux_Table_Manager::table_require, this, TABLE_REQUIRE_NAME(DVB_SUBTITLE) _pid, dvb_sub_actual, false, true, Demux::Data_Type::PES));
    }

    virtual void clear_table_queue() override
    {
        IDemux_Table_Manager::clear_table_queue();
    }

    State state() const
    {
        return m_state;
    }

    static void process_ota_callback_claro(uint32_t _product, uint16_t _sw, uint16_t _sw_min);
    std::shared_ptr<Event_OTA_DSI> m_ota_callback;
    Transponder m_tps_to_check;
    bool m_restart_after_lock = true;


#ifdef MBGUI_PERIODIC_DUMP
    virtual void handle_event_debbug_dump_status() override;
#endif // MBGUI_PERIODIC_DUMP
};

}
