#include "mb_task_database.h"

#include <thread>
#include <iostream>
#include <stdio.h>

#include "common/mb_globals.h"
#include "common/mb_state_file.h"
#include "common/mb_lineup.h"
#include "common/mb_config.h"

#include "mb_task_osd.h"
#include "mb_task_application.h"
#include "mb_main.h"
#include "mb_events.h"
#include "mb_zone_id.h"

#include "hal/mb_watchdog.h"

#include "fw_env.h"

#include <string.h>
#include <mutex>
#include "sqlite/sqlite3.h"
#include <iostream>
#include <filesystem>
#include <sys/stat.h>

namespace
{
std::mutex g_fw_env_mutex;
}

#define SQL_EXEC_DB(db, fn)                                               \
    do                                                                    \
    {                                                                     \
        auto ret = (fn);                                                  \
        if (ret != SQLITE_OK &&                                           \
            ret != SQLITE_ROW &&                                          \
            ret != SQLITE_DONE)                                           \
        {                                                                 \
            DEBUG_MSG(DB, ERROR, file_name(__FILE__) << ":" << std::dec   \
                      << __LINE__ << " SQLite Error(" << ret              \
                      << "): " << (db ? sqlite3_errmsg(db) : "NULL DB")    \
                      << " in " << #fn << "\n");                          \
            mb_assert(false);                                             \
        }                                                                 \
    } while (false)

#define SQL_EXEC(fn) SQL_EXEC_DB(db, fn)


namespace {

namespace TP {
enum TP_TABLE_COLS
{
    TRANSPONDER_ID,
    SYMBOL_RATE,
    DVB_MODE,
    TRANSPORT_STREAM_ID,
    ORIGINAL_NETWORK_ID,
    NETWORK_ID,
    IS_HOME_CHANNEL,
    SATELLITE_ID,
};
}

namespace SRV {
enum SRV_TABLE_COLS
{
    SRV_ID,
    TRANSPONDER_ID,
    SERVICE_ID,
    SERVICE_TYPE,
    NAME,
    EPG_PID,
    REGIONALIZACAO,
    VIEWER_CHANNEL,
    BOUQUET_ID,
    IS_FAVORITE,
    ZONES,
    ORDER_IN_FULL,
    ORDER_IN_FAVORITE,
};
}

}

namespace mb {

Task_Database::Task_Database()
{
    mb_assert(s_task_database == nullptr);
    s_task_database = this;
    sqlite3_initialize();
    // SQLite does NOT need thread-safety
    //mb_assert(sqlite3_threadsafe() == 0);
}

Task_Database::~Task_Database()
{
    mb_assert(s_task_database == this);
    s_task_database = nullptr;
    sqlite3_shutdown();
}

sqlite3 *Task_Database::open_local_db()
{
    sqlite3 *db { nullptr };
    std::error_code _;
    auto dir = std::filesystem::path(MBGUI_CACHE_PATH);

    if(!std::filesystem::exists(dir, _))
    {
        std::filesystem::create_directory(dir, _);
    }

    auto ret = sqlite3_open(MBGUI_LOCAL_DB, &db);

    if(ret != SQLITE_OK)
    {
        DEBUG_MSG(DB, DEBUG, "Open " MBGUI_LOCAL_DB " failed with: " << dec << ret << endl);
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
    }
    else
    {
        sqlite3_busy_timeout(db, 5000);

        if(m_first_open)
        {
            m_first_open = false;
            auto check_or_execute {[db](const char *_check_stmt, const char *_execute_stmt)
            {
                bool need_object { false };
                SQL_EXEC(sqlite3_exec(db, _check_stmt,
                                      [](void *param, int colcount [[maybe_unused]], char **colvalue, char ** /*colname*/)
                {
                    mb_assert(colcount == 1);

                    if(strcmp(colvalue[0], "1") != 0)
                    {
                        *(static_cast<bool *>(param)) = true;
                    }

                    return SQLITE_OK;
                }, &need_object, nullptr));

                if(need_object)
                {
                    SQL_EXEC(sqlite3_exec(db, _execute_stmt, nullptr, nullptr, nullptr));
                }
            }};
            auto check_create_object {[check_or_execute](const char *_type, const char *_object_name, const char *_create_cmd)
            {
                char buffer[512];
                snprintf(buffer, sizeof(buffer), R"sql(SELECT count(*) FROM sqlite_master WHERE type='%s' AND name='%s')sql", _type, _object_name);
                check_or_execute(buffer, _create_cmd);
            }};
            auto check_table_field {[check_or_execute](const char *_table_name, const char *_field_name, const char *_add_cmd)
            {
                char buffer[512];
                snprintf(buffer, sizeof(buffer), R"sql(select count(*) from pragma_table_info('%s') where name = "%s")sql", _table_name, _field_name);
                check_or_execute(buffer, _add_cmd);
            }};
            check_create_object("table", "tp", R"sql(create table tp (
transponder_id int not null primary key,
symbol_rate int not null,
dvb_mode int not null,
transport_stream_id int not null,
original_network_id int not null,
network_id int not null,
is_home_channel int not null,
satellite_id int not null
))sql");
            check_create_object("table", "srv", R"sql(create table srv (
srv_id integer primary key autoincrement,
transponder_id int not null,
service_id int not null,
service_type int,
name text,
epg_pid int,
regionalizacao int,
viewer_channel int,
bouquet_id int,
is_favorite int,
zones text,
order_in_full int,
order_in_favorite int,
constraint srv_service_id_idx unique (transponder_id, service_id)))sql");
            check_create_object("table", "sat", R"sql(create table sat (
sat_id integer not null,
name text,
band integer,
type integer,
position integer,
switch_type integer,
switch_pos integer,
orbital real,
is_mandatory boolean,
network_policies integer,
constraint sat_id primary key (sat_id)))sql");
            check_create_object("index", "idx_sat_name", "create unique index idx_sat_name on sat(name)");
            check_table_field("sat", "network_policies", "alter table sat add network_policies integer");
            check_create_object("table", "agenda", R"sql(create table agenda (
agenda_id integer primary key autoincrement,
srv_id integer not null,
start integer, -- unix epoch, GMT+0
end integer, -- unix epoch, GMT+0
oper integer, -- Gravar ou Lembrar
repeat integer, -- weekly, Monthly, etc
is_active boolean default true))sql");

            populate_satellite(db);
        }
    }

    return db;
}

void Task_Database::populate_satellite(sqlite3 *db)
{
    auto satellites = std::array
    {
        Satellite{1, "Star One D2", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 65, true, Network_Policies::TVRO},
        Satellite{2, "Sky B1", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 70, true, Network_Policies::Sky},
        Satellite{3, "Telstar 12", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 40, false, Network_Policies::Generic},
        Satellite{4, "NSS 7", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 70, false, Network_Policies::Generic },
        Satellite{5, "SES 6", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 40, false, Network_Policies::Generic },
        Satellite{6, "Intelsat 14", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 70, false, Network_Policies::Generic },
        Satellite{7, "Intelsat 1R", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 70, false, Network_Policies::Generic },
        Satellite{8, "Intelsat 21", Band::Ku, LNBF_Type::Universal, LNBF_Position::Normal, DiseqC_Type::None, 0, 70, false, Network_Policies::Generic },
    };

    if(db == nullptr)
    {
        return;
    }

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));

    sqlite3_stmt *stmt { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select count(*) from sat)sql", -1, &stmt, nullptr));
    SQL_EXEC(sqlite3_step(stmt));
    auto count = sqlite3_column_int(stmt, 0);
    SQL_EXEC(sqlite3_reset(stmt));

    if(count == 0)
    {
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(insert or ignore into sat(sat_id, name, band, type, position, switch_type, switch_pos, orbital, is_mandatory, network_policies)
            values (:id, :name, :band, :type, :position, :switch_type, :switch_pos, :orbital, :is_mandatory, :network_policies ) )sql", -1, &stmt, nullptr));

        for(const auto& sat : satellites)
        {
            DEBUG_MSG(TASK, DEBUG, "ID: " << sat.id << "\n");
            DEBUG_MSG(TASK, DEBUG, "Name: " << sat.name << "\n");
            SQL_EXEC(sqlite3_bind_int(stmt, 1, sat.id));
            SQL_EXEC(sqlite3_bind_text(stmt, 2, sat.name.c_str(), -1, SQLITE_TRANSIENT));
            SQL_EXEC(sqlite3_bind_int(stmt, 3, (int)sat.band));
            SQL_EXEC(sqlite3_bind_int(stmt, 4, (int)sat.type));
            SQL_EXEC(sqlite3_bind_int(stmt, 5, (int)sat.position));
            SQL_EXEC(sqlite3_bind_int(stmt, 6, (int)sat.switch_type));
            SQL_EXEC(sqlite3_bind_int(stmt, 7, sat.switch_pos));
            SQL_EXEC(sqlite3_bind_double(stmt, 8, sat.orbital));
            SQL_EXEC(sqlite3_bind_int(stmt, 9, sat.is_mandatory));
            SQL_EXEC(sqlite3_bind_int(stmt, 10, (int)sat.network_policies));
            SQL_EXEC(sqlite3_step(stmt));
            SQL_EXEC(sqlite3_reset(stmt));
        }
    }

    if (stmt)
    {
        SQL_EXEC(sqlite3_finalize(stmt));
    }

    SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
}

void Task_Database::handle_event_add_satellite(Satellite sat)
{
    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);
    if(db == nullptr)
    {
        return;
    }

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
    sqlite3_stmt *stmt { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(
insert into sat (name, band, type, position, switch_type, switch_pos, orbital, is_mandatory, network_policies)
values (:name, :band, :type, :position, :switch_type, :switch_pos, :orbital, :is_mandatory, :network_policies)
on conflict(name)
do update set
    band = excluded.band,
    type = excluded.type,
    position = excluded.position,
    switch_type = excluded.switch_type,
    switch_pos = excluded.switch_pos,
    orbital = excluded.orbital,
    is_mandatory = excluded.is_mandatory,
    network_policies = excluded.network_policies)sql", -1, &stmt, nullptr));

    SQL_EXEC(sqlite3_bind_text(stmt, 1, sat.name.c_str(), -1, SQLITE_TRANSIENT));
    SQL_EXEC(sqlite3_bind_int(stmt, 2, (int)sat.band));
    SQL_EXEC(sqlite3_bind_int(stmt, 3, (int)sat.type));
    SQL_EXEC(sqlite3_bind_int(stmt, 4, (int)sat.position));
    SQL_EXEC(sqlite3_bind_int(stmt, 5, (int)sat.switch_type));
    SQL_EXEC(sqlite3_bind_int(stmt, 6, sat.switch_pos));
    SQL_EXEC(sqlite3_bind_double(stmt, 7, sat.orbital));
    SQL_EXEC(sqlite3_bind_int(stmt, 8, sat.is_mandatory));
    SQL_EXEC(sqlite3_bind_int(stmt, 9, (int)sat.network_policies));
    SQL_EXEC(sqlite3_step(stmt));
    SQL_EXEC(sqlite3_finalize(stmt));
    int rc = sqlite3_exec(db, "commit", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        DEBUG_MSG(DB, DEBUG, "Failed to commit transaction: " << rc << "\n");
    }
    else
    {
        DEBUG_MSG(DB, DEBUG, "Satellite added/updated successfully.\n");
        Config::get_config()->load_satellite_list();
    }
}

void Task_Database::handle_event_update_satellite(Satellite sat)
{
    DEBUG_MSG(TASK, DEBUG, "ID: " << sat.id << "\n");
    DEBUG_MSG(TASK, DEBUG, "Name: " << sat.name << "\n");
    DEBUG_MSG(TASK, DEBUG, "Band: " << static_cast<int>(sat.band) << "\n");
    DEBUG_MSG(TASK, DEBUG, "Type: " << static_cast<int>(sat.type) << "\n");
    DEBUG_MSG(TASK, DEBUG, "Position: " << static_cast<int>(sat.position) << "\n");
    DEBUG_MSG(TASK, DEBUG, "Switch Type: " << static_cast<int>(sat.switch_type) << "\n");
    DEBUG_MSG(TASK, DEBUG, "Switch Pos: " << static_cast<int>(sat.switch_pos) << "\n");
    DEBUG_MSG(TASK, DEBUG, "Orbital: " << sat.orbital << "\n");
    DEBUG_MSG(TASK, DEBUG, "Is Mandatory: " << sat.is_mandatory << "\n");
    DEBUG_MSG(TASK, DEBUG, "Network Policies: " << static_cast<int>(sat.network_policies) << "\n");
    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db == nullptr)
    {
        return;
    }

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
    sqlite3_stmt *stmt { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(update sat
            set name = :name, band = :band, type = :type, position = :position, switch_type = :switch_type, switch_pos = :switch_pos, orbital = :orbital, is_mandatory = :is_mandatory, network_policies = :network_policies
            where sat_id = :id )sql", -1, &stmt, nullptr));
    SQL_EXEC(sqlite3_bind_text(stmt, 1, sat.name.c_str(), -1, SQLITE_TRANSIENT));
    SQL_EXEC(sqlite3_bind_int(stmt, 2, (int)sat.band));
    SQL_EXEC(sqlite3_bind_int(stmt, 3, (int)sat.type));
    SQL_EXEC(sqlite3_bind_int(stmt, 4, (int)sat.position));
    SQL_EXEC(sqlite3_bind_int(stmt, 5, (int)sat.switch_type));
    SQL_EXEC(sqlite3_bind_int(stmt, 6, sat.switch_pos));
    SQL_EXEC(sqlite3_bind_double(stmt, 7, sat.orbital));
    SQL_EXEC(sqlite3_bind_int(stmt, 8, sat.is_mandatory));
    SQL_EXEC(sqlite3_bind_int(stmt, 9, (int)sat.network_policies));
    SQL_EXEC(sqlite3_bind_int(stmt, 10, sat.id));
    SQL_EXEC(sqlite3_step(stmt));
    SQL_EXEC(sqlite3_finalize(stmt));
    SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
    Config::get_config()->load_satellite_list();
}

void Task_Database::handle_event_delete_satellite(unsigned int id)
{
    DEBUG_MSG(DB, DEBUG, "Apagando satélite " << id << "\n");
    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db != nullptr)
    {
        SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
        sqlite3_stmt *stmt { nullptr };
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(delete from sat where sat_id = :sat_id)sql", -1, &stmt, nullptr));
        SQL_EXEC(sqlite3_bind_int(stmt, 1, id));
        SQL_EXEC(sqlite3_step(stmt));
        SQL_EXEC(sqlite3_finalize(stmt));
        SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
        Config::get_config()->load_satellite_list();
    }
}

void Task_Database::load_lineup()
{
    auto start = std::chrono::steady_clock::now();
#ifndef MBGUI_SAT_MONITOR
    auto db = scoped_var(open_local_db(), sqlite3_close);
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

    if(db != nullptr)
    {
        current_lineup->clear();
        sqlite3_stmt *stmt_tp { nullptr };
        sqlite3_stmt *stmt_srv { nullptr };
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select
    transponder_id,
    symbol_rate,
    dvb_mode,
    transport_stream_id,
    original_network_id,
    network_id,
    is_home_channel,
    satellite_id
from tp)sql", -1, &stmt_tp, nullptr));
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select
    srv_id,
    transponder_id,
    service_id,
    service_type,
    name,
    epg_pid,
    regionalizacao,
    viewer_channel,
    bouquet_id,
    is_favorite,
    zones,
    order_in_full,
    order_in_favorite
from srv)sql", -1, &stmt_srv, nullptr));

        while(sqlite3_step(stmt_tp) == SQLITE_ROW)
        {
            Transponder tp;
            tp.transponder_id.set_id(sqlite3_column_int64(stmt_tp, TP::TRANSPONDER_ID));
            tp.symbol_rate = sqlite3_column_int(stmt_tp, TP::SYMBOL_RATE);
            tp.dvb_mode = static_cast<DVB_Mode>(sqlite3_column_int(stmt_tp, TP::DVB_MODE));
            tp.transport_stream_id = sqlite3_column_int(stmt_tp, TP::TRANSPORT_STREAM_ID);
            tp.original_network_id = sqlite3_column_int(stmt_tp, TP::ORIGINAL_NETWORK_ID);
            tp.network_id = sqlite3_column_int(stmt_tp, TP::NETWORK_ID);
            tp.is_home_channel = sqlite3_column_int(stmt_tp, TP::IS_HOME_CHANNEL) == 1;
            tp.set_satellite_id(sqlite3_column_int(stmt_tp, TP::SATELLITE_ID));
            current_lineup->transponders.emplace_back(std::move(tp));
        }

        while(sqlite3_step(stmt_srv) == SQLITE_ROW)
        {
            Transponder_Id transponder_id;
            transponder_id.set_id(sqlite3_column_int64(stmt_srv, SRV::TRANSPONDER_ID));
            Service s(transponder_id, sqlite3_column_int(stmt_srv, SRV::SERVICE_ID));
            s.set_service_type(static_cast<Service_Type>(sqlite3_column_int(stmt_srv, SRV::SERVICE_TYPE)));
            auto txt = sqlite3_column_text(stmt_srv, SRV::NAME);
            s.set_name(txt ? std::string_view(reinterpret_cast<const char*>(txt)) : std::string_view{});
            s.set_epg_pid(sqlite3_column_int(stmt_srv, SRV::EPG_PID));
            s.set_regionalizacao(static_cast<Regionalizacao>(sqlite3_column_int(stmt_srv, SRV::REGIONALIZACAO)));
            s.set_viewer_channel(sqlite3_column_int(stmt_srv, SRV::VIEWER_CHANNEL));
            s.set_bouquet_id(sqlite3_column_int(stmt_srv, SRV::BOUQUET_ID));
            s.set_is_favorite(sqlite3_column_type(stmt_srv, SRV::IS_FAVORITE) == SQLITE_NULL ? true : sqlite3_column_int(stmt_srv, SRV::IS_FAVORITE) > 0);
            const char* zones = reinterpret_cast<const char*>(sqlite3_column_text(stmt_srv, SRV::ZONES));
            s.set_order_in_full(sqlite3_column_int64(stmt_srv, SRV::ORDER_IN_FULL));
            s.set_order_in_favorite(sqlite3_column_int64(stmt_srv, SRV::ORDER_IN_FAVORITE));
            s.set_satellite_id(transponder_id.satellite_id());


            if(zones)
            {
                char buffer[3];
                auto end = zones + strlen(zones);
                auto z = zones;

                while(z < end)
                {
                    buffer[0] = z[0];
                    buffer[1] = z[1];
                    buffer[2] = 0;
                    z += 2;
                    s.insert_zone(strtoul(buffer, nullptr, 16));
                }
            }

            current_lineup->services.emplace_back(std::move(s));
        }

        SQL_EXEC(sqlite3_finalize(stmt_tp));
        SQL_EXEC(sqlite3_finalize(stmt_srv));
    }
    else
    {
        DEBUG_MSG(DB, ERROR, "load_lineup: failed to open local database\n");
        current_lineup->clear();
    }

#endif // MBGUI_SAT_MONITOR
    // Make sure we load the Zone ID
    post_event_lineup_changed();
    post_event_lineup_ready(
    {
        .origin = Lineup_Origin::LO_DATABASE,
        .duration = std::chrono::duration_cast<std::chrono::milliseconds>(decltype(start)::clock::now() - start),
        .restart = true
    });
}

struct Task_Database::Data
{
    enum
    {
        SS_TRANSPONDER,
        SS_SERVICE,
        SS_HIDDEN_SERVICE,
        SS_CLEANUP,
    } save_step { SS_TRANSPONDER };

    sqlite3 *db { nullptr };
    sqlite3_stmt *stmt_tp { nullptr };
    sqlite3_stmt *stmt_srv { nullptr };
    unsigned int it { 0 };

    Data(sqlite3 *_db) :
        db(_db)
    {
    }

    ~Data()
    {
        if(stmt_srv)
        {
            sqlite3_finalize(stmt_srv);
        }

        if(stmt_tp)
        {
            sqlite3_finalize(stmt_tp);
        }

        if(db)
        {
            sqlite3_close(db);
        }
    }
};

void Task_Database::abort_save()
{
    if(m_p)
    {
        sqlite3_exec(m_p->db, "rollback", nullptr, nullptr, nullptr);
        m_p.reset();
        m_state.store(ST_IDLE, std::memory_order_release);
        m_save_needs_reschedule = true;
        DEBUG_MSG(DB, DEBUG, "save_lineup aborted to serve a priority write event; will reschedule.\n");
    }
}

void Task_Database::save_lineup()
{
    if(!m_p)
    {
        auto db = open_local_db();
        if (!db)
        {
            DEBUG_MSG(DB, ERROR, "save_lineup: failed to open local database; reverting to idle\n");
            m_state.store(ST_IDLE, std::memory_order_release);
            return;
        }
        m_p = std::make_unique<Data>(db);
        m_p->save_step = Data::SS_TRANSPONDER;
        m_p->it = 0;
        SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
        
        // Create temporary tables to track which records are in the current lineup
        SQL_EXEC(sqlite3_exec(db, "create temporary table if not exists current_tp_ids (id integer primary key)", nullptr, nullptr, nullptr));
        SQL_EXEC(sqlite3_exec(db, "create temporary table if not exists current_srv_ids (tid integer, sid integer, primary key(tid, sid))", nullptr, nullptr, nullptr));
        SQL_EXEC(sqlite3_exec(db, "delete from current_tp_ids", nullptr, nullptr, nullptr));
        SQL_EXEC(sqlite3_exec(db, "delete from current_srv_ids", nullptr, nullptr, nullptr));

        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(insert into tp(transponder_id,
    symbol_rate,
    dvb_mode,
    transport_stream_id,
    original_network_id,
    network_id,
    is_home_channel,
    satellite_id)
values(:transponder_id, :symbol_rate, :dvb_mode, :transport_stream_id, :original_network_id, :network_id, :is_home_channel, :satellite_id)
on conflict(transponder_id) do update set
    symbol_rate=excluded.symbol_rate,
    dvb_mode=excluded.dvb_mode,
    transport_stream_id=excluded.transport_stream_id,
    original_network_id=excluded.original_network_id,
    network_id=excluded.network_id,
    is_home_channel=excluded.is_home_channel,
    satellite_id=excluded.satellite_id)sql", -1, &m_p->stmt_tp, nullptr));

        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(insert into srv(transponder_id,
    service_id,
    service_type,
    name,
    epg_pid,
    regionalizacao,
    viewer_channel,
    bouquet_id,
    is_favorite,
    zones,
    order_in_full,
    order_in_favorite)
values(:transponder_id, :service_id, :service_type, :name, :epg_pid,
:regionalizacao, :viewer_channel, :bouquet_id, :is_favorite, :zones, :order_in_full, :order_in_favorite)
on conflict(transponder_id, service_id) do update set
    service_type=excluded.service_type,
    name=excluded.name,
    epg_pid=excluded.epg_pid,
    regionalizacao=excluded.regionalizacao,
    viewer_channel=excluded.viewer_channel,
    bouquet_id=excluded.bouquet_id,
    is_favorite=excluded.is_favorite,
    zones=excluded.zones,
    order_in_full=excluded.order_in_full,
    order_in_favorite=excluded.order_in_favorite)sql", -1, &m_p->stmt_srv, nullptr));
    }

    auto db = m_p->db;

    switch(m_p->save_step)
    {
        case Data::SS_TRANSPONDER:
        {
            auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
            auto stmt_tp = m_p->stmt_tp;

            if(m_p->it >= current_lineup->transponders.size())
            {
                m_p->save_step = Data::SS_SERVICE;
                m_p->it = 0;
                break;
            }

            const auto &tp = current_lineup->transponders[m_p->it];
            SQL_EXEC(sqlite3_exec(db, (std::string("insert or ignore into current_tp_ids values(") + std::to_string(tp.transponder_id.id()) + ")").c_str(), nullptr, nullptr, nullptr));
            SQL_EXEC(sqlite3_bind_int64(stmt_tp, TP::TRANSPONDER_ID + 1, tp.transponder_id.id()));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::SYMBOL_RATE + 1, tp.symbol_rate));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::DVB_MODE + 1, static_cast<int>(tp.dvb_mode)));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::TRANSPORT_STREAM_ID + 1, tp.transport_stream_id));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::ORIGINAL_NETWORK_ID + 1, tp.original_network_id));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::NETWORK_ID + 1, tp.network_id));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::IS_HOME_CHANNEL + 1, tp.is_home_channel));
            SQL_EXEC(sqlite3_bind_int(stmt_tp, TP::SATELLITE_ID + 1, tp.satellite_id));
            SQL_EXEC(sqlite3_step(stmt_tp));
            SQL_EXEC(sqlite3_reset(stmt_tp));
            ++m_p->it;
            break;
        }

        case Data::SS_SERVICE:
        case Data::SS_HIDDEN_SERVICE:
        {
            auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
            auto stmt_srv = m_p->stmt_srv;
            const Service *srv { nullptr };

            if(m_p->save_step == Data::SS_SERVICE)
            {
                if(m_p->it >= current_lineup->services.size())
                {
                    m_p->save_step = Data::SS_HIDDEN_SERVICE;
                    m_p->it = 0;
                    return;
                }

                srv = &current_lineup->services[m_p->it];
            }
            else
            {
                if(m_p->it >= current_lineup->hidden_services.size())
                {
                    m_p->save_step = Data::SS_CLEANUP;
                    return;
                }

                srv = &current_lineup->hidden_services[m_p->it];
            }

            int idx = 1;
            SQL_EXEC(sqlite3_exec(db, (std::string("insert or ignore into current_srv_ids values(") + std::to_string(srv->transponder_id().id()) + "," + std::to_string(srv->service_id()) + ")").c_str(), nullptr, nullptr, nullptr));
            SQL_EXEC(sqlite3_bind_int64(stmt_srv, idx++, srv->transponder_id().id()));
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, srv->service_id()));
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, static_cast<int>(srv->service_type())));
            SQL_EXEC(sqlite3_bind_text(stmt_srv, idx++, srv->name().data(), -1, SQLITE_TRANSIENT));
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, srv->epg_pid()));
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, static_cast<int>(srv->regionalizacao())));
            uint32_t _number = srv->viewer_channel();
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, _number));
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, srv->bouquet_id()));
            SQL_EXEC(sqlite3_bind_int(stmt_srv, idx++, srv->is_favorite() ? 1 : 0));

            std::string zones;
            zones.reserve(srv->zones().size() * 2);

            for (auto z : srv->zones())
            {
                char tmp[3];
                snprintf(tmp, sizeof(tmp), "%.2x", z);
                zones += tmp;
            }

            SQL_EXEC(sqlite3_bind_text(stmt_srv, idx++, zones.c_str(), -1, SQLITE_TRANSIENT));
            uint64_t _number_long = (static_cast<uint64_t>(srv->satellite_id()) << 48u) | (static_cast<uint64_t>(_number) << 16u) | static_cast<uint64_t>(srv->service_id());
            SQL_EXEC(sqlite3_bind_int64(stmt_srv, idx++, srv->get_order_in_full() ? srv->get_order_in_full() : _number_long));
            SQL_EXEC(sqlite3_bind_int64(stmt_srv, idx++, srv->get_order_in_favorite() ? srv->get_order_in_favorite() : _number_long));
            SQL_EXEC(sqlite3_step(stmt_srv));
            SQL_EXEC(sqlite3_reset(stmt_srv));
            ++m_p->it;
            break;
        }

        case Data::SS_CLEANUP:
        {
            DEBUG_MSG(DB, INFO, "Performing cleanup of old lineup records\n");
            // Delete records that are no longer in the lineup
            SQL_EXEC(sqlite3_exec(db, "delete from tp where transponder_id not in (select id from current_tp_ids)", nullptr, nullptr, nullptr));
            SQL_EXEC(sqlite3_exec(db, "delete from srv where (transponder_id, service_id) not in (select tid, sid from current_srv_ids)", nullptr, nullptr, nullptr));
            
            SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
            m_p.reset();
            m_state.store(ST_IDLE, std::memory_order_release);
            DEBUG_MSG(DB, INFO, "Lineup save completed successfully\n");
            break;
        }
    }
}

void Task_Database::process()
{
    switch(m_state.load(std::memory_order_relaxed))
    {
        case ST_STARTING:
        {
            const bool is_in_standby = Task::s_task_application && Task::s_task_application->is_in_stand_by();
            if(g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed) && !is_in_standby)
            {
                return;
            }
            auto db = scoped_var(open_local_db(), sqlite3_close);
            if (!db)
            {
                DEBUG_MSG(DB, ERROR, "Database failed to open\n");
            }
            verify_next_schedule();
            m_state.store(ST_IDLE, std::memory_order_release);
            break;
        }

        case ST_IDLE:
        {
            const bool is_in_standby = Task::s_task_application && Task::s_task_application->is_in_stand_by();
            if(g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed) && !is_in_standby)
            {
                return;
            }
            if(m_save_needs_reschedule)
            {
                m_save_needs_reschedule = false;
                handle_event_lineup_save();
                break;
            }

            auto time_to_start = System::get_system_time().to_unix_epoch();
            time_to_start += time_start_offset;
            auto now = std::chrono::system_clock::from_time_t(time_to_start);
            if (m_next_agendamento < now)
            {
                verify_next_schedule();
            }
            break;
        }

        case ST_LINEUP_SAVE:
            save_lineup();
            break;
    }
}

void Task_Database::handle_event_easy_install_save(bool _easy_install_finish)
{
    State_File::App_State_File file;
    file.easy_install_finish = _easy_install_finish;
    file.write();
}

void Task_Database::handle_event_application_state_save(const Event_Save_Application_State &_event)
{
    State_File::App_State_File file;
    file.current_channel = _event.current_channel;
    file.volume = _event.volume;
    file.mute = _event.mute;
    file.stand_by = _event.stand_by;
    file.channel_list_type = static_cast<unsigned int>(_event.channel_list_type);
    file.current_satellite_id = _event.current_satellite_id;
    file.write();
}

void Task_Database::handle_event_lineup_save_zone_id(Zone_ID_t _zone_id, Segment_ID_t _segment_id)
{
    Zone_ID::set_zone_id(_zone_id, _segment_id);
}

void Task_Database::handle_event_application_state_load()
{
    Event_Save_Application_State event;
    State_File::App_State_File file(&event.got_state);

    if(event.got_state)
    {
        event.volume = file.volume;
        event.mute = file.mute != 0;
        event.stand_by = file.stand_by;
        event.channel_list_type = static_cast<Channel_List_Type>(file.channel_list_type);
        event.current_channel = file.current_channel;
    }

    post_event_application_state_loaded(std::move(event));
}

void Task_Database::handle_event_satellite_list_load(std::function<void(std::vector<Satellite> &satellites)> _cb)
{
    auto db = scoped_var(open_local_db(), sqlite3_close);
    if(!db)
    {
        DEBUG_MSG(DB, DEBUG, "Failed to open local database.\n");
        return;
    }

    sqlite3_stmt *stmt_tp { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select sat_id, name, band, type, position, switch_type, switch_pos, orbital, is_mandatory, network_policies from sat)sql", -1, &stmt_tp, nullptr));
    std::vector<Satellite> sats;

    while(sqlite3_step(stmt_tp) == SQLITE_ROW)
    {
        Satellite sat;
        sat.id = sqlite3_column_int(stmt_tp, 0);
        sat.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt_tp, 1));
        sat.band = static_cast<Band>(sqlite3_column_int(stmt_tp, 2));
        sat.type = static_cast<LNBF_Type>(sqlite3_column_int(stmt_tp, 3));
        sat.position = static_cast<LNBF_Position>(sqlite3_column_int(stmt_tp, 4));
        sat.switch_type = static_cast<DiseqC_Type>(sqlite3_column_int(stmt_tp, 5));
        sat.switch_pos = sqlite3_column_int(stmt_tp, 6);
        sat.orbital = sqlite3_column_double(stmt_tp, 7);
        sat.is_mandatory = sqlite3_column_int(stmt_tp, 8);
        sat.network_policies = static_cast<Network_Policies>(sqlite3_column_int(stmt_tp, 9));
        sats.push_back(sat);
    }

    for (auto& sat : sats)
    {
        DEBUG_MSG(DB, DEBUG,  static_cast<int>(sat.id) << ": "
                << std::setfill(' ') << std::left << std::setw(13) << sat.name <<
                std::setw(9) << to_str(sat.band) <<
                std::setw(11) << to_str(sat.type) << "\n");
    }

    SQL_EXEC(sqlite3_finalize(stmt_tp));

    if(_cb)
    {
        _cb(sats);
    }
}

void Task_Database::handle_event_update_channel_list(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_data)
{
    DEBUG_MSG(TASK, DEBUG, "Task_Database::handle_event_update_channel_list()\n");

    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);
    if (!db)
        return;

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));

    sqlite3_stmt* stmt = nullptr;

    const char* sql = (_channel_list_type == Channel_List_Type::MY_TV_CHANNELS ||
                       _channel_list_type == Channel_List_Type::MY_RADIO_CHANNELS)
        ? R"sql(update srv set order_in_favorite = ?
                where service_id = ?
                and is_favorite = ?
                and viewer_channel = ?)sql"
        : R"sql(update srv set order_in_full = ?
                where service_id = ?
                and is_favorite = ?
                and viewer_channel = ?)sql";

    SQL_EXEC(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    uint16_t it = 0;

    for (const auto& _chd : _channel_data)
    {
        SQL_EXEC(sqlite3_bind_int(stmt, 1, it));
        SQL_EXEC(sqlite3_bind_int(stmt, 2, _chd.service_id));
        SQL_EXEC(sqlite3_bind_int(stmt, 3, _chd.favorite));
        SQL_EXEC(sqlite3_bind_int(stmt, 4, _chd.viewer_channel));

        SQL_EXEC(sqlite3_step(stmt));
        SQL_EXEC(sqlite3_reset(stmt));
        SQL_EXEC(sqlite3_clear_bindings(stmt));

        ++it;
    }

    SQL_EXEC(sqlite3_finalize(stmt));
    SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
}

void Task_Database::verify_next_schedule()
{
    const bool is_in_standby =
        g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed) ||
        (Task::s_task_application && Task::s_task_application->is_in_stand_by());

    // Verifica se a data é válida, caso contrário, não faz nada (ex: erro ao ler o relógio do sistema).
    auto system_time = System::get_system_time();
    auto time_to_start = system_time.to_unix_epoch();
    time_to_start += time_start_offset;
    if (system_time.year() <= 2025)
    {
        DEBUG_MSG(DB, ERROR, "System time is not valid (year " << system_time.year() << "); skipping schedule verification\n");
        return;
    }

    auto db = scoped_var(open_local_db(), sqlite3_close);
    if(!db)
    {
        return;
    }

    sqlite3_stmt *stmt_tp { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select agenda_id, srv_id, start, end, oper, repeat, is_active from agenda where is_active and end > ? order by start)sql", -1, &stmt_tp, nullptr));
    SQL_EXEC(sqlite3_bind_int(stmt_tp, 1, time_to_start));

    std::vector<ScheduleEntry> schedule_rows;
    while(sqlite3_step(stmt_tp) == SQLITE_ROW)
    {
        ScheduleEntry sc_entry = {};
        sc_entry.id = sqlite3_column_int(stmt_tp, 0);
        sc_entry.service_id = sqlite3_column_int(stmt_tp, 1);
        sc_entry.time_to_start = std::chrono::system_clock::from_time_t(sqlite3_column_int(stmt_tp, 2));
        sc_entry.time_to_end = std::chrono::system_clock::from_time_t(sqlite3_column_int(stmt_tp, 3));
        sc_entry.operation = (Schedule_Operation)sqlite3_column_int(stmt_tp, 4);
        sc_entry.repeat = (Schedule_Repeat)sqlite3_column_int(stmt_tp, 5);
        sc_entry.status = (Schedule_Status)sqlite3_column_int(stmt_tp, 6);
        schedule_rows.emplace_back(std::move(sc_entry));
    }
    SQL_EXEC(sqlite3_finalize(stmt_tp));

    if (is_in_standby)
    {
        Time_Point next_agendamento = Time_Point::max();
        auto curr_time_to_start = std::chrono::system_clock::from_time_t(time_to_start);

        if(!schedule_rows.empty())
        {
            const auto &next_entry = schedule_rows.front();
            next_agendamento = next_entry.time_to_start;
            auto time_until_event = std::chrono::duration_cast<std::chrono::seconds>(next_agendamento - curr_time_to_start).count();

            if (time_until_event <= time_before_poweron)
            {
                // Cleanup possible ota found by the tuner while in standby, to ensure it doesn't interfere with the scheduled recording.
                {
                    std::lock_guard<std::mutex> lk(g_fw_env_mutex);
                    fw_env_open();
                    fw_env_write("ota_found", "0");
                    fw_env_close();
                }
                DEBUG_MSG(DB, INFO, "Standby: next event in " << time_until_event << "s, powering on receiver\n");
                post_event_toggle_power();

                // After requesting power-on, keep the next trigger at event-time window.
                m_next_agendamento = next_agendamento;
                return;
            }

            // Schedule the next verification to happen time_before_poweron before event start.
            auto next_start_epoch = std::chrono::system_clock::to_time_t(next_entry.time_to_start);
            auto next_standby_check_epoch = next_start_epoch - time_before_poweron + time_start_offset;
            next_agendamento = std::chrono::system_clock::from_time_t(next_standby_check_epoch);
        }

        m_next_agendamento = next_agendamento;
        return;
    }

    Time_Point next_agendamento = Time_Point::max();
    std::vector<int> delete_once_ids;

    auto curr_time_to_start = std::chrono::system_clock::from_time_t(time_to_start);

    for(const auto &sc_entry : schedule_rows)
    {
        if (sc_entry.time_to_start <= curr_time_to_start)
        {
            DEBUG_MSG(DB, DEBUG, "ID: " << dec << sc_entry.id << "\n");
            DEBUG_MSG(DB, DEBUG, "Service_ID: " << dec << sc_entry.service_id << "\n");
            DEBUG_MSG(DB, DEBUG, "curr_time_to_start: " << curr_time_to_start << "\n");
            DEBUG_MSG(DB, DEBUG, "Time to Start: " << dec << sc_entry.time_to_start << "\n");
            DEBUG_MSG(DB, DEBUG, "Time to End: " << dec << sc_entry.time_to_end << "\n");
            DEBUG_MSG(DB, DEBUG, "Operation: " << dec << static_cast<int>(sc_entry.operation) << "\n");
            DEBUG_MSG(DB, DEBUG, "Repeat: " << dec << static_cast<int>(sc_entry.repeat) << "\n");
            DEBUG_MSG(DB, DEBUG, "Status: " << dec << static_cast<int>(sc_entry.status) << "\n");
            post_event_send_message_to_start_record(sc_entry);

            if(sc_entry.repeat == Schedule_Repeat::ONCE)
            {
                delete_once_ids.push_back(sc_entry.id);
            }
            continue;
        }

        next_agendamento = sc_entry.time_to_start;
        break;
    }

    for(auto id : delete_once_ids)
    {
        handle_event_delete_schedule(id);
    }

    m_next_agendamento = next_agendamento;

    if(next_agendamento != Time_Point::max())
    {
        DEBUG_MSG(DB, DEBUG, "m_next_agendamento: " << m_next_agendamento << "\n");
        auto time_to_start_now = System::get_system_time().to_unix_epoch();
        auto now = std::chrono::system_clock::from_time_t(time_to_start_now);
        DEBUG_MSG(DB, DEBUG, "NOW: " << now << "\n");
    }
}

void Task_Database::handle_event_insert_schedule(ScheduleEntry entry)
{
    auto time_to_start = std::chrono::system_clock::to_time_t(entry.time_to_start);
    auto time_to_end = std::chrono::system_clock::to_time_t(entry.time_to_end);

    DEBUG_MSG(DB, DEBUG, "ID: " << dec << entry.id << "\n");
    DEBUG_MSG(DB, DEBUG, "Service_ID: " << dec << entry.service_id << "\n");
    auto curr_time_to_start = std::chrono::system_clock::from_time_t(System::get_system_time().to_unix_epoch());
    DEBUG_MSG(DB, DEBUG, "curr_time_to_start: " << curr_time_to_start << "\n");
    DEBUG_MSG(DB, DEBUG, "Time to Start: " << dec << time_to_start << "\t" << entry.time_to_start << "\n");
    DEBUG_MSG(DB, DEBUG, "Time to End: " << dec << time_to_end << "\t" << entry.time_to_end << "\n");
    DEBUG_MSG(DB, DEBUG, "Operation: " << dec << static_cast<int>(entry.operation) << "\n");
    DEBUG_MSG(DB, DEBUG, "Repeat: " << dec << static_cast<int>(entry.repeat) << "\n");
    DEBUG_MSG(DB, DEBUG, "Status: " << dec << static_cast<int>(entry.status) << "\n");
    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db == nullptr)
    {
        return;
    }

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
    sqlite3_stmt *stmt { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(insert into agenda (srv_id, start, end, oper, repeat, is_active)
values (:srv_id, :start, :end, :oper, :repeat, :is_active))sql", -1, &stmt, nullptr));
    SQL_EXEC(sqlite3_bind_int(stmt, 1, entry.service_id));
    SQL_EXEC(sqlite3_bind_int(stmt, 2, (int)time_to_start));
    SQL_EXEC(sqlite3_bind_int(stmt, 3, (int)time_to_end));
    SQL_EXEC(sqlite3_bind_int(stmt, 4, (int)entry.operation));
    SQL_EXEC(sqlite3_bind_int(stmt, 5, (int)entry.repeat));
    SQL_EXEC(sqlite3_bind_int(stmt, 6, (int)entry.status));
    SQL_EXEC(sqlite3_step(stmt));
    SQL_EXEC(sqlite3_finalize(stmt));
    SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));

    m_next_agendamento = Time_Point::max();
    verify_next_schedule();
}

void Task_Database::handle_event_delete_schedule(int agenda_id)
{
    DEBUG_MSG(TASK, DEBUG, "Apagando agenda " << agenda_id << "\n");
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db != nullptr)
    {
        SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
        sqlite3_stmt *stmt { nullptr };
        if(agenda_id == 0)
        {
            SQL_EXEC(sqlite3_prepare_v2(db, R"sql(delete from agenda)sql", -1, &stmt, nullptr));
        }
        else
        {
            SQL_EXEC(sqlite3_prepare_v2(db, R"sql(delete from agenda where agenda_id = :agenda_id)sql", -1, &stmt, nullptr));
            SQL_EXEC(sqlite3_bind_int(stmt, 1, agenda_id));
        }

        SQL_EXEC(sqlite3_step(stmt));
        SQL_EXEC(sqlite3_finalize(stmt));
        SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
    }
}

void Task_Database::handle_event_update_schedule(ScheduleEntry entry)
{
    auto time_to_start = std::chrono::system_clock::to_time_t(entry.time_to_start);
    auto time_to_end = std::chrono::system_clock::to_time_t(entry.time_to_end);
    DEBUG_MSG(DB, DEBUG, "ID: " << dec << entry.id << "\n");
    DEBUG_MSG(DB, DEBUG, "Service_ID: " << dec << entry.service_id << "\n");
    DEBUG_MSG(DB, DEBUG, "Time to Start: " << dec << time_to_start << "\n");
    DEBUG_MSG(DB, DEBUG, "Time to End: " << dec << time_to_end << "\n");
    DEBUG_MSG(DB, DEBUG, "Operation: " << dec << static_cast<int>(entry.operation) << "\n");
    DEBUG_MSG(DB, DEBUG, "Repeat: " << dec << static_cast<int>(entry.repeat) << "\n");
    DEBUG_MSG(DB, DEBUG, "Status: " << dec << static_cast<int>(entry.status) << "\n");
    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db != nullptr)
    {
        SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
        sqlite3_stmt *stmt { nullptr };
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(update agenda set srv_id = :srv_id, start = :start, end = :end, oper = :oper, repeat = :repeat, is_active = :is_active where agenda_id = :agenda_id)sql", -1, &stmt, nullptr));
        SQL_EXEC(sqlite3_bind_int(stmt, 1, entry.service_id));
        SQL_EXEC(sqlite3_bind_int(stmt, 2, time_to_start));
        SQL_EXEC(sqlite3_bind_int(stmt, 3, time_to_end));
        SQL_EXEC(sqlite3_bind_int(stmt, 4, (int)entry.operation));
        SQL_EXEC(sqlite3_bind_int(stmt, 5, (int)entry.repeat));
        SQL_EXEC(sqlite3_bind_int(stmt, 6, (int)entry.status));
        SQL_EXEC(sqlite3_bind_int(stmt, 7, (int)entry.id));
        SQL_EXEC(sqlite3_step(stmt));
        SQL_EXEC(sqlite3_finalize(stmt));
        SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
    }

    m_next_agendamento = Time_Point::max();
    verify_next_schedule();
}

void Task_Database::handle_event_schedule_load(std::function<void(std::vector<ScheduleEntry> schedule_entry)> _schedule)
{
    DEBUG_MSG(DB, DEBUG, "Task_Database::handle_event_schedule_load()\n");
    if(_schedule)
    {
        auto db = scoped_var(open_local_db(), sqlite3_close);

        if(!db)
        {
            return;
        }

        sqlite3_stmt *stmt_tp { nullptr };
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select agenda_id, srv_id, start, end, oper, repeat, is_active from agenda)sql", -1, &stmt_tp, nullptr));
        std::vector<ScheduleEntry> scheds;

        while(sqlite3_step(stmt_tp) == SQLITE_ROW)
        {
            ScheduleEntry sc_entry;
            sc_entry.id = sqlite3_column_int(stmt_tp, 0);
            sc_entry.service_id = sqlite3_column_int(stmt_tp, 1);
            sc_entry.time_to_start = std::chrono::system_clock::from_time_t(sqlite3_column_int(stmt_tp, 2));
            sc_entry.time_to_end = std::chrono::system_clock::from_time_t(sqlite3_column_int(stmt_tp, 3));
            sc_entry.operation = (Schedule_Operation)sqlite3_column_int(stmt_tp, 4);
            sc_entry.repeat = (Schedule_Repeat)sqlite3_column_int(stmt_tp, 5);
            sc_entry.status = (Schedule_Status)sqlite3_column_int(stmt_tp, 6);
            scheds.push_back(sc_entry);
        }

        SQL_EXEC(sqlite3_finalize(stmt_tp));
        _schedule(std::move(scheds));
    }
}

void Task_Database::handle_event_lineup_load()
{
    auto state = m_state.load(std::memory_order_acquire);
    if(ST_IDLE == state)
    {
        load_lineup();
    }
}

void Task_Database::handle_event_lineup_save()
{
    auto state = m_state.load(std::memory_order_acquire);
    if(ST_IDLE == state)
    {
        m_state.store(ST_LINEUP_SAVE, std::memory_order_relaxed);
        save_lineup();
        save_locked_satellites();
    }
}

void Task_Database::save_locked_satellites()
{
    uint8_t satellite_flags = 0 ;
    auto lineup = Lineup_Mutex_Ref::get_current_lineup();
    for (const auto& tp : lineup->transponders)
    {
        if(tp.satellite_id == 1)
        {
            satellite_flags |= 0x01;
        }
        else if(tp.satellite_id == 2)
        {
            satellite_flags |= 0x02;
        }
    }
    State_File::App_State_File file;
    file.satellite_flags = satellite_flags;
    file.write();
}

void Task_Database::handle_event_service_favorite_changed(Service *_srv)
{
    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db != nullptr and _srv)
    {
        SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
        sqlite3_stmt *stmt { nullptr };
        SQL_EXEC(sqlite3_prepare_v2(db, R"sql(update srv set is_favorite = :is_favorite
where transponder_id = :transponder_id
  and service_id = :service_id)sql", -1, &stmt, nullptr));
        SQL_EXEC(sqlite3_bind_int(stmt, 1, _srv->is_favorite() ? 1 : 0));
        SQL_EXEC(sqlite3_bind_int64(stmt, 2, _srv->transponder_id().id()));
        SQL_EXEC(sqlite3_bind_int(stmt, 3, _srv->service_id()));
        SQL_EXEC(sqlite3_step(stmt));
        SQL_EXEC(sqlite3_finalize(stmt));
        SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
    }
}

void Task_Database::handle_event_system_display_settings_save(const Event_System_Settings &_event)
{
    State_File::App_State_File file;

    switch(_event.time_zone)
    {
        case Timezone_Mode::Auto:
        case Timezone_Mode::Acre_UTC_5:
            g_timezone_offset = -5;
            break;

        case Timezone_Mode::F_Noronha_UTC_2:
            g_timezone_offset = -2;
            break;

        case Timezone_Mode::Amazonas_UTC_4:
            g_timezone_offset = -4;
            break;

        default:
        case Timezone_Mode::Brasilia_UTC_3:
            g_timezone_offset = -3;
            break;
    }

    file.time_zone = static_cast<uint8_t>(_event.time_zone);
    g_clock_set = static_cast<uint8_t>(_event.clock_status);
    file.clock_status = static_cast<uint8_t>(_event.clock_status);
    file.language_mode = static_cast<uint8_t>(_event.language_mode);
    file.resolution = static_cast<uint8_t>(_event.resolution);
    file.color_standard = static_cast<uint8_t>(_event.color_standard);
    file.aspect_mode = static_cast<uint8_t>(_event.aspect_mode);
    file.write();
    post_event_system_display_settings_saved();

    if(_event.clock_status == Clock_Type::Auto)
    {
        post_event_clock_need_update();
    }
}

void Task_Database::handle_event_system_display_settings_load()
{
    Event_System_Settings event;
    State_File::App_State_File file;

    switch(static_cast<Timezone_Mode>(file.time_zone))
    {
        case Timezone_Mode::Auto:
        case Timezone_Mode::Acre_UTC_5:
            g_timezone_offset = -5;
            break;

        case Timezone_Mode::F_Noronha_UTC_2:
            g_timezone_offset = -2;
            break;

        case Timezone_Mode::Amazonas_UTC_4:
            g_timezone_offset = -4;
            break;

        default:
        case Timezone_Mode::Brasilia_UTC_3:
            g_timezone_offset = -3;
            break;
    }

    event.time_zone = static_cast<Timezone_Mode>(file.time_zone);
    g_clock_set = static_cast<uint8_t>(file.clock_status);
    event.clock_status = static_cast<Clock_Type>(file.clock_status);
    event.language_mode = static_cast<Language_Mode>(file.language_mode);
    event.resolution = static_cast<Resolution_Standard>(file.resolution);
    event.color_standard = static_cast<Color_Standard>(file.color_standard);
    event.aspect_mode = static_cast<Aspect_Mode>(file.aspect_mode);
    post_event_system_display_settings_loaded(event);
}

void Task_Database::handle_event_lnbf_config_save(const Event_LNBF_Params &_event)
{
    DEBUG_MSG(DB, DEBUG, "Saving LNBF Config: "
              << to_str(_event.lnbf_type) << " "
              << to_str(_event.band) << " "
              << (_event.inverted ? "Inverted" : "Normal") << "\n");

    State_File::App_State_File file;
    file.band = _event.band;
    file.lnbf_inverted = _event.inverted;
    file.lnbf_type = _event.lnbf_type;
    file.current_satellite_id = _event.sat_id;
    file.write();

    abort_save();
    auto db = scoped_var(open_local_db(), sqlite3_close);
    if(!db)
    {
        DEBUG_MSG(DB, ERROR, "Failed to open local database.\n");
        return;
    }

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));

    sqlite3_stmt* stmt { nullptr };

    SQL_EXEC(sqlite3_prepare_v2(
        db,
        "update sat set type = ? where sat_id = ?",
        -1,
        &stmt,
        nullptr));

    SQL_EXEC(sqlite3_bind_int(stmt, 1, static_cast<int>(_event.lnbf_type)));
    SQL_EXEC(sqlite3_bind_int(stmt, 2, _event.sat_id));

    SQL_EXEC(sqlite3_step(stmt));
    SQL_EXEC(sqlite3_finalize(stmt));

    SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));

    Config::get_config()->load_satellite_list();

    DEBUG_MSG(DB, DEBUG, "Satellite " << _event.sat_id << " updated successfully.\n");
}

void Task_Database::handle_event_pin_reset()
{
#ifndef NDEBUG
    post_event_cas_popup_message(
    {
        .message = "Pin Code reset to factory default",
        .category = Message_Categories::Event_Popup,
        .timeout = std::chrono::seconds(3),
    });
#endif
}

bool Task_Database::is_empty()
{
    auto db = scoped_var(open_local_db(), sqlite3_close);

    if(db == nullptr)
    {
        return false;
    }

    sqlite3_stmt *stmt { nullptr };
    SQL_EXEC(sqlite3_prepare_v2(db, R"sql(select count(*) from srv)sql", -1, &stmt, nullptr));
    SQL_EXEC(sqlite3_step(stmt));
    auto count = sqlite3_column_int(stmt, 0);
    SQL_EXEC(sqlite3_finalize(stmt));
    return count == 0;
}

void Task_Database::handle_event_delete_all_services()
{
    if(m_state.load(std::memory_order_acquire) != ST_IDLE)
        return;

    auto db = scoped_var(open_local_db(), sqlite3_close);
    if(!db)
        return;

    SQL_EXEC(sqlite3_exec(db, "begin immediate transaction", nullptr, nullptr, nullptr));
    SQL_EXEC(sqlite3_exec(db, "delete from srv", nullptr, nullptr, nullptr));
    SQL_EXEC(sqlite3_exec(db, "delete from tp", nullptr, nullptr, nullptr));
    SQL_EXEC(sqlite3_exec(db, "commit", nullptr, nullptr, nullptr));
    load_lineup();
}


} // namespace mb
