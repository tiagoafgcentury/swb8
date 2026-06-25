#include "mb_task_demux.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <thread>
#include <cstring>
#include "fw_env.h"

#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "common/mb_satellites.h"
#include "common/mb_state_file.h"
#include "common/mb_version.h"
#include "dvb/mb_dvb_eit.h"
#include "hal/mb_demux.h"
#include "hal/mb_system.h"
#include "hal/mb_tuner.h"
#include "mb_demux_lineup_claro.h"
#include "mb_demux_channel_list.h"
#include "mb_demux_lineup_generic.h"
#include "mb_demux_lineup_sky.h"
#include "mb_events.h"
#include "mb_main.h"
#include "mb_task_application.h"
#include "mb_task_cas.h"
#include "mb_task_osd.h"
#include "mb_task_player.h"
#include "mb_task_tuner.h"

#ifndef NDEBUG
#include <iomanip>
#include <iostream>
#endif

using namespace std::chrono_literals;

namespace mb
{

std::string_view to_str(Task_Demux::State _state)
{
    switch (_state) {
        case Task_Demux::ST_IDLE:
            return "ST_IDLE";

        case Task_Demux::ST_LOCKING:
            return "ST_LOCKING";

        case Task_Demux::ST_LINEUP_RUNNING:
            return "ST_LINEUP_RUNNING";

        case Task_Demux::ST_LINEUP_FINISHED:
            return "ST_LINEUP_FINISHED";

        case Task_Demux::ST_PAT_UPDATING:
            return "ST_PAT_UPDATING";

        case Task_Demux::ST_CLOCK_UPDATING:
            return "ST_CLOCK_UPDATING";

        case Task_Demux::ST_START_EMM_FILTERING:
            return "ST_START_EMM_FILTERING";
    }

    return "<UNDEFINED>";
}

Task_Demux::Task_Demux() : m_demux(std::make_unique<Demux>())
{
    mb_assert(s_task_demux == nullptr);
    s_task_demux = this;
    using namespace std::placeholders;
    m_demux->set_cat_callback(std::bind(&Task_Demux::cat_callback, this, _1, _2));
    m_demux->set_eit_callback(std::bind(&Task_Demux::eit_callback, this, _1, _2));
    m_demux->set_ota_callback(std::bind(&Task_Demux::ota_callback, this, _1, _2));
    m_demux->set_pat_callback(std::bind(&Task_Demux::pat_callback, this, _1, _2));
    m_demux->set_pmt_callback(std::bind(&Task_Demux::pmt_callback, this, _1, _2));
    m_demux->set_tdt_callback(std::bind(&Task_Demux::tdt_callback, this, _1, _2));
    m_demux->set_tot_callback(std::bind(&Task_Demux::tot_callback, this, _1, _2));
}

Task_Demux::~Task_Demux()
{
    mb_assert(s_task_demux == this);
    s_task_demux = nullptr;
    m_demux->clear_cat_callback();
    m_demux->clear_ota_callback();
    m_demux->clear_pat_callback();
    m_demux->clear_pmt_callback();
    m_demux->clear_tot_callback();
}

void Task_Demux::cat_callback(PID_t _pid, CAT&& _cat)
{
    const auto version = _cat.version_number();
    const auto section_num = _cat.section_number();
    const auto last_section = _cat.last_section_number();

    if (!m_cat_version.has_value() || m_cat_version.value() != version)
    {
        DEBUG_MSG(DEMUX, INFO, "CAT version " << dec << (int)version << " started, resetting accumulation\n");
        m_cat_version = version;
        m_cat_section_seen.reset();
        for (auto &block : m_cat_descriptor_blocks)
        {
            block.clear();
        }
    }

    if (section_num >= m_cat_descriptor_blocks.size())
    {
        DEBUG_MSG(DEMUX, ERROR, "CAT section number out of range: " << dec << (int)section_num << "\n");
        return;
    }

    m_cat_section_seen[section_num] = true;
    auto section_data = _cat.move_cat_section_data();
    m_cat_descriptor_blocks[section_num].assign(section_data.data(), section_data.data() + section_data.size());

    DEBUG_MSG(DEMUX, INFO, "CAT section " << dec << (int)section_num << "/" << (int)last_section << " version " << (int)version << " received\n");

    bool is_last = true;
    for (int i = 0; i <= last_section && i < static_cast<int>(m_cat_section_seen.size()); ++i)
    {
        if (!m_cat_section_seen[i])
        {
            is_last = false;
            break;
        }
    }

    if (!is_last)
    {
        return;
    }

    DEBUG_MSG(DEMUX, INFO, "CAT complete version " << dec << (int)version << ", sending concatenated CAT to CAS\n");

    auto combined = std::vector<uint8_t>();
    combined.reserve(1024);

    if (!m_cat_descriptor_blocks[0].empty())
    {
        auto &section0 = m_cat_descriptor_blocks[0];
        combined.insert(combined.end(), section0.begin(), section0.begin() + std::min<size_t>(section0.size(), 8u));
    }
    else
    {
        combined.insert(combined.end(), 8, 0);
    }

    for (int i = 0; i <= last_section; ++i)
    {
        auto &block = m_cat_descriptor_blocks[i];
        if (block.size() < 12)
        {
            continue;
        }

        combined.insert(combined.end(), block.begin() + 8, block.end() - 4);
    }

    combined.insert(combined.end(), 4, 0);

    m_cat_section_seen.reset();
    m_cat_version.reset();
    for (auto &block : m_cat_descriptor_blocks)
    {
        block.clear();
    }

    auto cat_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(CAT_TABLE_ID);
    remove_need_table(TABLE_REQUIRE_NAME(CAT) _pid, cat_table_id, Demux::Data_Type::SECTION);
    cat_table_require(version);

    post_event_cas_send_cat_table_section(
    {
        .is_last = true,
        .cat_section_data = DVB_Table_Section(combined.data(), combined.size()),
    });
}

void Task_Demux::eit_callback(PID_t _pid, EIT&& _eit) {
#ifndef MBGUI_SAT_MONITOR

    if (g_mbgui_pause_low_priority_tasks.load(std::memory_order_relaxed))
    {
        return;
    }

    auto eit = Task_EIT_Events::s_eit_programs.find(_eit.service_id());

    if (eit == Task_EIT_Events::s_eit_programs.end())
    {
        auto [nit, _] = Task_EIT_Events::s_eit_programs.emplace(_eit.service_id(), Task_EIT_Events::EIT_Service_Events{});
        eit = nit;
    }
    else
    {
        if (m_eit_services_sections[_eit.service_id()] == _eit.section_number())
        {
            // TODO Figure out when we can stop reading EIT
            return;
            m_eit_services_sections.clear();
            auto eit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(_eit.table_id());
            remove_need_table(TABLE_REQUIRE_NAME(EIT) _pid, eit_table_id,                                                Demux::Data_Type::SECTION);
        }
    }

    m_eit_services_sections[_eit.service_id()] = _eit.section_number();

    auto events = _eit.move_events();
    auto now = UTC_MJD{System::get_system_time()};

    auto config = Config::get_config();
    auto is_sky = config->selected_satellite_config().network_id == Network_Id_Sky;

    for (auto e : events)
    {
        uint8_t rating{0};
        std::optional<Content_Descriptor::Content_Nibble> content_descriptor;
        std::string extended_event_descriptor;
        std::optional<CA_RS_Descriptor> ca_rs_descriptor;

        if (e.start_time + e.duration < now)
        {
            goto SKIP_EVENT;
        }

        for (const auto& evt : eit->second.events)
        {
            if (evt.start_time == e.start_time)
            {
                goto SKIP_EVENT;
            }
        }

        if (e.parental_rating_descriptor.has_value())
        {
            auto r = e.parental_rating_descriptor.value().move_ratings();

            if (r.size() == 1)
            {
                rating = r[0].rating;
            }
            else
            {
                if (r.size() > 1)
                {
                    for (const auto& rate : r)
                    {
                        if (strcasecmp(rate.country_code, "bra") == 0)
                        {
                            rating = rate.rating;
                            goto FOUND_RATING;
                        }
                    }

                    // No rating found, default first
                    rating = r[0].rating;
                }
            }
        }

    FOUND_RATING:

        if (rating >= 0x01 and rating <= 0x0f)
        {
            if (not is_sky)
            {
                rating += 3;
            }
        }
        else
        {
            if (rating)
            {
                DEBUG_MSG(DEMUX, DEBUG,
                                    "Rating defined by the broadcaster: 0x"
                                            << hex << setfill('0') << setw(2) << rating << "\n");
                rating = 0;
            }
        }

        if (e.content_descriptor.has_value())
        {
            content_descriptor = e.content_descriptor.value().content_nibble();
        }

        if (e.short_event_descriptor.has_value())
        {
            extended_event_descriptor = e.short_event_descriptor.value().move_text();
        }

        if (e.extended_event_descriptor.has_value())
        {
            auto ext_text = e.extended_event_descriptor.value().move_text();
            if (!ext_text.empty())
            {
                if (!extended_event_descriptor.empty())
                {
                    extended_event_descriptor += " ";
                }
                extended_event_descriptor += ext_text;
            }
        }

        if (e.ca_rs_descriptor.has_value())
        {
            ca_rs_descriptor = std::move(e.ca_rs_descriptor.value());
        }

        {
            auto events = &eit->second.events;
            auto pos = events->begin();

            if (!events->empty())
            {
                // set pos to the position of the first program that starts after the
                // new one
                while (pos != events->end() && pos->start_time < e.start_time)
                {
                    pos++;
                }
            }

            // add the new element
            DEBUG_MSG(
                    EPG, DEBUG,
                    "EIT " TERM_GREEN_BOLD "Add:" TERM_RESET " "
                            << dec << _eit.service_id() << "\t" << e.start_time.time_to_str()
                            << " - " << (e.start_time + e.duration).time_to_str() << "\t"
                            << e.short_event_descriptor.value().peek_event_name() << "\n");
            pos = events->insert(
                    pos,
                    Task_EIT_Events::EIT_Event{
                            .short_event_descriptor =
                                    e.short_event_descriptor.has_value()
                                            ? e.short_event_descriptor.value().move_event_name()
                                            : std::string{},
                            .content_descriptor = std::move(content_descriptor),
                            .extended_event_descriptor = std::move(extended_event_descriptor),
                            .parental_rating = rating,
                            .event_id = e.event_id,
                            .start_time = e.start_time,
                            .duration = e.duration,
                            .ca_rs_descriptor_bseid = std::move(ca_rs_descriptor->bseid())});
        }

    SKIP_EVENT:;  // NO-OP
    }  // for (auto e : events)

#else
    // SAT Monitor does not read EIT
    mb_assert(false);
    (void)(_pid);  // Suppres complier warning
    (void)(_eit);  // Suppres complier warning
#endif  // MBGUI_SAT_MONITOR
}

void Task_Demux::nit_callback(PID_t _pid, NIT&& _nit)
{

    auto config = Config::get_config();
    config->set_satellite_config(_nit.network_id());

    if (_nit.network_id() == Network_Id_Sky)
    {
        auto transport_streams = _nit.move_transport_streams();
        auto linkage_descriptors = _nit.move_linkage_descriptors();

        for (const auto& ld : linkage_descriptors)
        {
            switch (ld.linkage_type())
            {
                case Linkage_Descriptor::Linkage_Type::system_software_update_service:
                {
                    if (ld.ota_swdl_descriptor().has_value())
                    {
                        auto lineup = Lineup_Mutex_Ref::get_current_lineup();
                        OTA_TS_PID_List transponder_list;
                        auto ota_list = std::move(ld.ota_swdl_descriptor().value());
                        auto software_installed_str = MB_OSD_Version::get_major_minor_version_str();
                        uint16_t software_installed = std::stoi(software_installed_str);

                        DEBUG_MSG(DEMUX, DEBUG,"MBGUI_SKY_OTA_OUI: " << hex << MBGUI_SKY_OTA_OUI <<
                            ", MBGUI_SKY_OTA_HW_CODE: " << hex << MBGUI_SKY_OTA_HW_CODE <<
                            ", MBGUI_SKY_OTA_MODEL: " << hex << MBGUI_SKY_OTA_MODEL << "\n");
                        for (const auto& ota : ota_list)
                        {
                            DEBUG_MSG(
                                    DEMUX, INFO,
                                    "OTA SWDL Descriptor:"
                                            << hex << right << " TS ID: " << setw(4) << ota.tsid()
                                            << " ONID: " << setw(4) << ota.onid() << " SVC ID:"
                                            << setw(4) << ota.svc_id() << " OUI: " << setw(8)
                                            << ota.OUI() << " Manufactor Code: " << setw(2)
                                            << (int)ota.manufacturer_code() << " HW Code: " << setw(2)
                                            << (int)ota.hardware_code() << " Model Code: " << setw(2)
                                            << (int)ota.model_code() << " Download Mode: " << setw(2)
                                            << (int)ota.download_mode() << " SW Version: " << setw(4)
                                            << ota.software_version() << " PID: " << setw(4)
                                            << ota.pid() << " Reset Flag: " << setw(2)
                                            << (int)ota.factory_reset_flag() << "\n");

                            if (ota.OUI() == MBGUI_SKY_OTA_OUI and
                                ota.hardware_code() == MBGUI_SKY_OTA_HW_CODE and
                                ota.model_code() == MBGUI_SKY_OTA_MODEL and
                                ota.software_version() > software_installed )
                            {
                                for(const auto &tp : lineup->transponders)
                                {
                                    if(tp.transport_stream_id == ota.tsid())
                                    {
                                        transponder_list.emplace_back(OTA_TS_PID{tp, ota.pid(), ota.software_version(), ota.download_mode(), ota.factory_reset_flag()});
                                    }
                                }
                            }
                        }

                        if (not transponder_list.empty())
                        {
                            DEBUG_MSG(DEMUX, WARN, "Setting transponder list for OTA Sky\n");
                            MB_Satellites::set_transponder_list_for_ota(Satellite_Operator::Sky, std::move(transponder_list));
                            fw_env_open();
                            fw_env_write("ota_found", "1");
                            fw_env_close();
                        }
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }
    auto nit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(_nit.table_id());
    remove_need_table(TABLE_REQUIRE_NAME(NIT) _pid, nit_table_id, Demux::Data_Type::SECTION);
    // Reset callback
    m_demux->clear_nit_callback();
}

void Task_Demux::ota_callback(PID_t pid, OTA&& _ota)
{
    auto ota_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(_ota.table_id());
    // auto ota_table_id = std::make_shared<Demux::TS_Filter_OTA_Message_Id>(OTA_TABLE_ID, 0x1006);
    remove_need_table(TABLE_REQUIRE_NAME(OTA) pid, ota_table_id, Demux::Data_Type::SECTION);
    auto files = _ota.move_ota_files();

    for (auto f : files)
    {
        if (!m_ota_event_callback.expired())
        {
            auto p = m_ota_event_callback.lock();
            p->callback(f.product_id, f.sw_current, f.sw_min);
        }
    }
}

void Task_Demux::pat_callback(PID_t _pid, PAT&& _pat)
{
    auto pat_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(_pat.table_id());
    remove_need_table(TABLE_REQUIRE_NAME(PAT) _pid, pat_table_id, Demux::Data_Type::SECTION);
    auto programs{_pat.move_programs()};
    for (const auto& prg : programs)
    {
        if (prg.program != 0 && prg.pid > 0)
        {
            switch (state())
            {
                case ST_IDLE:
                case ST_CLOCK_UPDATING:
                    break;

                case ST_START_EMM_FILTERING:
                case ST_PAT_UPDATING:
                {
                    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                    auto [transponder_id, _] = Task_Tuner::get_current_transponder();
                    auto tp = current_lineup->get_transponder(transponder_id);

                    if (tp)
                    {
                        tp->pat_version_number = _pat.version_number();
                    }

                    for (auto& srv : current_lineup->services)
                    {
                         if (srv.service_id() == prg.program && srv.transponder_id() == transponder_id)
                        {
                            if (m_demux_lineup)  // on ST_START_EMM_FILTERING we read PAT but
                                                                     // we don't have a lineup
                            {
                                m_demux_lineup->update_pmts.emplace(prg.pid, prg.program);
                            }
                            pmt_table_require(prg.pid, prg.program);
                            srv.set_pmt_pid(prg.pid);
                            break;
                        }
                    }
                    break;
                }

                case ST_LOCKING:
                    mb_assert(false);
                    break;

                case ST_LINEUP_FINISHED:
                    break;

                case ST_LINEUP_RUNNING:
                {
                    if (m_demux_lineup)
                    {
                        m_demux_lineup->pat_program_callback(prg);
                    }

                    pmt_table_require(prg.pid, prg.program);
                    break;
                }
            }
        }
    }
}

void Task_Demux::handle_event_service_pmt_get_next_section(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number)
{
    pmt_table_require(_pid, _service_id, _current_version_number);
}

void Task_Demux::pmt_callback(PID_t _pid, PMT&& _pmt)
{
    DEBUG_MSG(DEMUX, DEBUG, "Got PMT version: " << dec << (int)_pmt.version_number() << "\n");
    auto table_id = _pmt.table_id();
    // Forward PMT to channel list parser if active
    if (m_channel_list)
    {
        m_channel_list->pmt_callback(_pid, std::move(_pmt));
        return;
    }

    switch (state()) {
        case ST_IDLE:
        case ST_CLOCK_UPDATING:
        case ST_PAT_UPDATING:
        case ST_START_EMM_FILTERING:
        {
            auto _service_id = _pmt.program_number();
            auto pmt_table_id = std::make_shared<Demux::TS_Filter_PMT>(table_id, _service_id);
            remove_need_table(TABLE_REQUIRE_NAME(PMT) _pid, pmt_table_id,                                                 Demux::Data_Type::SECTION);
            post_event_service_pmt_section(_pid, _service_id,                                                                     _pmt.pmt_section_data());
            // This scope is necessary to free Lineup_Mutex_Ref's mutex
            if (m_demux_lineup)
            {
                auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                m_demux_lineup->parse_pmt(std::move(_pmt), current_lineup.get());
            }

            if (m_demux_lineup)
            {
                m_demux_lineup->update_pmts.erase({_pid, _service_id});

                if (m_demux_lineup->update_pmts.empty())
                {
                    m_demux_lineup.reset();
                    set_state(ST_IDLE);
                }
            }

            break;
        }

        case ST_LOCKING:
            mb_assert(false);
            break;

        case ST_LINEUP_RUNNING:
        case ST_LINEUP_FINISHED:
        {
            auto service_id = _pmt.program_number();
            auto finished = m_demux_lineup->pmts.generic_table_callback(std::move(_pmt));

            if (finished)
            {
                auto pmt_table_id = std::make_shared<Demux::TS_Filter_PMT>(table_id, service_id);
                remove_need_table(TABLE_REQUIRE_NAME(PMT) _pid, pmt_table_id, Demux::Data_Type::SECTION);
                m_demux_lineup->done_tables |= ptPMT;
            }

            break;
        }
    }
}

void Task_Demux::tdt_callback(PID_t _pid, TDT&& _tdt)
{
    DEBUG_MSG(DEMUX, INFO, "Got TDT\n");
    auto utime{_tdt.move_utc_time()};

    if (static_cast<Clock_Type>(g_clock_set) != Clock_Type::Manual)
    {
        post_event_clock_set_time(Event_Time
        {
            .year = utime.year(),
            .month = utime.month(),
            .day = utime.day(),
            .hour = utime.hour(),
            .minute = utime.minute(),
            .second = utime.second()
        });
    }

    auto tdt_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(_tdt.table_id());
    remove_need_table(TABLE_REQUIRE_NAME(TDT) _pid, tdt_table_id, Demux::Data_Type::SECTION);

    if (state() == ST_CLOCK_UPDATING)
    {
        set_state(ST_IDLE);
    }
}

void Task_Demux::tot_callback(PID_t _pid, TOT&& _tot)
{
    DEBUG_MSG(DEMUX, INFO, "Got TOT\n" << std::dec);
#ifndef NDEBUG

    for (auto t : _tot.move_time_offsets())
    {
        auto min{t.local_time_offset.count()};
        DEBUG_MSG_NL(DEMUX, INFO,
                    "\t" << t.country_code << "\t" << (int)t.country_region_id
                         << "\t" << (t.local_time_offset_polarity ? "-" : "+")
                         << std::setw(2) << std::setfill('0') << min / 60 << ":"
                         << std::setw(2) << std::setfill('0') << min % 60 << "\n");
    }

#endif
    auto utime{_tot.move_utc_time()};

    if (static_cast<Clock_Type>(g_clock_set) != Clock_Type::Manual)
    {
        post_event_clock_set_time(Event_Time
        {
            .year = utime.year(),
            .month = utime.month(),
            .day = utime.day(),
            .hour = utime.hour(),
            .minute = utime.minute(),
            .second = utime.second()
        });
    }

    auto tot_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(_tot.table_id());
    remove_need_table(TABLE_REQUIRE_NAME(TOT) _pid, tot_table_id, Demux::Data_Type::SECTION);

    if (state() == ST_CLOCK_UPDATING)
    {
        set_state(ST_IDLE);
    }
}

void Task_Demux::handle_event_autodetect_lnbf_finished(bool _success)
{
    if (_success)
    {
        using namespace std::placeholders;
        m_demux->clear_nit_callback();
        m_demux->set_nit_callback(std::bind(&Task_Demux::nit_callback, this, _1, _2));
        auto nit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(NIT_TABLE_ID_ACTUAL);
        table_require(TABLE_REQUIRE_NAME(NIT) NIT_TSPID, nit_table_id);
    }
}

void Task_Demux::handle_event_lineup_build(std::weak_ptr<Event_List_Update> _event, bool restart)
{
    auto config = Config::get_config();
    const auto& sat_config = config->selected_satellite_config();
    DEBUG_MSG(DEMUX, INFO, "Building lineup for satellite: " << sat_config.name << "\n");
    DEBUG_MSG(DEMUX, INFO, "network_policies: " << sat_config.network_policies << "\n");

    m_restart_after_lock = restart;
    if (restart)
    {
        DEBUG_MSG(DEMUX, DEBUG, TERM_GREEN_BOLD "Will restart scan after lock\n" TERM_RESET);
    }
    else
    {
        DEBUG_MSG(DEMUX, DEBUG, TERM_RED_BOLD "Will not restart scan after lock\n" TERM_RESET);
    }

    switch (state()) {
        case ST_IDLE:
        case ST_PAT_UPDATING:
        case ST_CLOCK_UPDATING:
        case ST_START_EMM_FILTERING:
        {
            clear_table_queue();
            m_demux_lineup.reset();
            //if (!m_demux_lineup)
            {
                if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Sky))
                {
                    DEBUG_MSG(DEMUX, INFO, "Using Sky lineup demux\n");
                    m_demux_lineup = std::make_unique<Demux_Lineup_Sky>(this);
                }
                else if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::TVRO))
                {
                    DEBUG_MSG(DEMUX, INFO, "Using TVRO lineup demux\n");
                    m_demux_lineup = std::make_unique<Demux_Lineup_Claro>(this);
                }
                else
                {
                    DEBUG_MSG(DEMUX, INFO, "Using Generic lineup demux\n");
                    m_demux_lineup = std::make_unique<Demux_Lineup_Generic>(this);
                }
                m_demux_lineup->event_callback = std::move(_event);
                m_demux_lineup->set_current_satellite_id(config->get_current_satellite());
            }

            DEBUG_MSG(DEMUX, INFO, "\n");
            set_state(ST_LINEUP_RUNNING);
            auto lineup = Lineup_Mutex_Ref::get_current_lineup();
            m_demux_lineup->lineup = lineup.get();

            // Clear existing channels for the current satellite before scanning
            // to avoid duplicates and stale entries from the database load.
            if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Sky) or
                sat_config.network_policies == static_cast<uint32_t>(Network_Policies::TVRO))
            {
                DEBUG_MSG(DEMUX, INFO, "Clearing stale services for satellite " << config->get_current_satellite() << "\n");
                m_demux_lineup->lineup->clear_satellite(config->get_current_satellite());
            }

            {
                if (auto evt = _event.lock())
                {
                    if (evt->clear_table)
                    {
                        m_demux_lineup->lineup->clear();
                    }
                }
            }

            m_demux_lineup->start_list.resize(sat_config.tps.size());

            DEBUG("\nsat_config.tps: " << sat_config.tps.size());
            DEBUG_MSG(DEMUX, INFO, "\n");
            int i = 0;

            for (const auto& tp : sat_config.tps)  // TPs have to be manually copied
            {
                DEBUG_MSG(DEMUX, INFO, "\n");
                m_demux_lineup->start_list[i].transponder_id = tp.transponder_id;
                DEBUG("\nSAT_ID: " << tp.transponder_id.satellite_id());
                m_demux_lineup->start_list[i].symbol_rate = tp.symbol_rate;
                m_demux_lineup->start_list[i].dvb_mode = tp.dvb_mode;
                m_demux_lineup->start_list[i].transport_stream_id = tp.transport_stream_id;
                m_demux_lineup->start_list[i].original_network_id = tp.original_network_id;
                m_demux_lineup->start_list[i].network_id = tp.network_id;
                m_demux_lineup->start_list[i].is_home_channel = tp.is_home_channel;
                m_demux_lineup->start_list[i].satellite_id = tp.satellite_id;
#ifdef MBGUI_SAT_MONITOR
                m_demux_lineup->start_list[i].failed_to_lock = true;
#endif
                i++;
            }

            m_demux_lineup->start_list_idx = 0;
            m_demux_lineup->start();
        }

        case ST_LINEUP_RUNNING:
        case ST_LINEUP_FINISHED:
        case ST_LOCKING:
            break;
    }
}

void Task_Demux::handle_event_lineup_update(std::weak_ptr<Event_List_Update_Channel_List> _event)
{
    auto evt = _event.lock();
    if (!evt)
    {
        return;
    }
    const auto& tps = evt->transponders;
    DEBUG_MSG(DEMUX, INFO, "Channel List Update: " << dec << tps.size() << " transponders\n");

    // Destroy any previous channel list parser
    m_channel_list.reset();

    // Instantiate and start the channel list parser
    m_channel_list = std::make_unique<Demux_Channel_List>(this);
    m_channel_list->set_event_callback(_event);
    m_channel_list->start();
}

void Task_Demux::handle_event_transponder_locked(const Event_Tuner_Lock& _event)
{
    DEBUG_MSG(DEMUX, INFO, "\n");

    if (m_channel_list)
    {
        m_channel_list->transponder_lock(_event);
    }

    switch (state())
    {
        case ST_PAT_UPDATING:
        case ST_CLOCK_UPDATING:
        case ST_LINEUP_RUNNING:
        case ST_LINEUP_FINISHED:
            break;

        case ST_IDLE:
        case ST_START_EMM_FILTERING:
        {
            if(!_event.success)
            {
                break;
            }

#if defined(MBGUI_APP_CAS)
            auto task_cas = Task_CAS::get_instance();
            if (task_cas && !_event.already_locked)
            {
                DEBUG_MSG(DEMUX, INFO, "Discarding pending EMM filtering request before transport change\n");
                task_cas->discard_emm_filtering_request();
            }
            else if(task_cas)
            {
                DEBUG_MSG(DEMUX, DEBUG, "Keeping EMM filtering request on already locked transport\n");
            }
#endif
            cat_table_require();
            break;
        }

        case ST_LOCKING: {
            if (m_demux_lineup)
            {
                m_demux_lineup->transponder_lock(_event);
            }

            break;
        }
    }
}

void Task_Demux::handle_event_cas_request_descramble_done(bool _result)
{
    if (_result)
    {
        cat_table_require();
    }
}

void Task_Demux::handle_event_cas_start_emm_filtering(const Transponder* _tp)
{
    set_state(ST_START_EMM_FILTERING);
    post_event_transponder_lock(POST_CALLER _tp);
}

void Task_Demux::handle_event_services_update()
{
    auto config = Config::get_config();
    const auto& sat_config = config->selected_satellite_config();

    switch (state())
    {
        case ST_IDLE:
        case ST_CLOCK_UPDATING:
        case ST_PAT_UPDATING:
        case ST_START_EMM_FILTERING: {
            if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Sky))
            {
#if defined(MBGUI_APP_CAS)
                auto task_cas = Task_CAS::get_instance();
                if (state() == ST_START_EMM_FILTERING && (task_cas == nullptr || !task_cas->is_emm_filtering_ready()))
                {
                    DEBUG_MSG(DEMUX, INFO, "Waiting for EMM filtering before configuring SKY list\n");
                    return;
                }
#endif
                m_demux_lineup = std::make_unique<Demux_Lineup_Sky>(this);
            }
            else if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Generic))
            {
                m_demux_lineup = std::make_unique<Demux_Lineup_Generic>(this);
            }
            else
            {
                m_demux_lineup = std::make_unique<Demux_Lineup_Claro>(this);
            }
            m_demux_lineup->set_current_satellite_id(config->get_current_satellite());

            set_state(ST_PAT_UPDATING);
            pat_table_require();
            break;
        }

        case ST_LOCKING:
        case ST_LINEUP_RUNNING:
        case ST_LINEUP_FINISHED:
            break;
    }
}

void Task_Demux::handle_event_ota_update_get( PID_t _pid, std::weak_ptr<Event_OTA_DSI> _event)
{
    m_ota_event_callback = std::move(_event);
    ota_table_require(_pid);
}

void Task_Demux::handle_event_dvb_subtitle_get(PID_t _pid)
{
    dvb_subtitle_table_require(_pid);
}

void Task_Demux::handle_event_dvb_subtitle_del(PID_t _pid)
{
    auto dvb_sub_actual = std::make_shared<Demux::TS_Filter_Table_Id>(0x00);
    remove_need_table(TABLE_REQUIRE_NAME(DVB_SUBTITLE) _pid, dvb_sub_actual,                                     Demux::Data_Type::PES);
}

void Task_Demux::handle_event_clock_need_update()
{
    switch (state())
    {
        case ST_IDLE:
        {
            set_state(ST_CLOCK_UPDATING);
            tdt_table_require();
            tot_table_require();
            break;
        }

        case ST_CLOCK_UPDATING:
            break;

        case ST_PAT_UPDATING:
        case ST_LOCKING:
        case ST_LINEUP_RUNNING:
        case ST_LINEUP_FINISHED:
        case ST_START_EMM_FILTERING:
            tdt_table_require();
            tot_table_require();
            break;
    }
}

void Task_Demux::handle_event_eit_update()
{
    switch (state())
    {
        case ST_IDLE:
        case ST_CLOCK_UPDATING:
        case ST_PAT_UPDATING:
        case ST_LINEUP_FINISHED:
        {
            eit_table_require();
            break;
        }

        case ST_START_EMM_FILTERING:
        case ST_LOCKING:
        case ST_LINEUP_RUNNING:
            DEBUG_MSG(DEMUX, DEBUG, "Ignore EIT\n");
            break;
    }
}

void Task_Demux::handle_event_lineup_ready(const Event_Lineup_Ready&)
{
    std::unordered_set<EIT_Service_ID_t> service_filter;
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    service_filter.reserve(current_lineup->services.size());

    for (const auto& svc : current_lineup->services)
    {
        service_filter.emplace(svc.service_id());
    }

    EIT::set_service_filter(std::move(service_filter));
}

void Task_Demux::handle_event_player_started()
{
    check_for_otas();
}

void Task_Demux::check_for_otas()
{
    DEBUG_MSG(DEMUX, DEBUG, "Checking for OTAs...\n");
    auto config = Config::get_config();
    const auto& sat_config = config->selected_satellite_config();

    // Load current set frequency
    auto [tp, is_locked] = Task::s_task_tuner->get_current_transponder();
    auto frequency = tp.frequency();
    if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Sky))
    {
        DEBUG_MSG(DEMUX, DEBUG, "Current frequency: " << std::dec << frequency << "MHz\n");
        check_for_ota_sky(frequency);
    }
    else if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::TVRO))
    {
        DEBUG_MSG(DEMUX, DEBUG, "Current frequency: " << std::dec << frequency << "MHz\n");
        check_for_ota_claro(frequency);
    }
    else if (sat_config.network_policies == static_cast<uint32_t>(Network_Policies::Generic))
    {
        DEBUG_MSG(DEMUX, INFO, "OTA check not supported for current network policies\n");
    }

    // If the system time is wrong, we can have problems reading the NIT, so we check if the year is reasonable before checking for OTA
    auto system_time = System::get_system_time().to_local_time();
    if ( (int)system_time.year() < 2025 )
    {
        DEBUG_MSG(DEMUX, WARN, "Hora do sistema inválida\n");
        post_event_clock_need_update();        
        return;
    }
}

void Task_Demux::check_for_ota_sky(uint32_t _frequency)
{
    // If stb is in stand-by and frequency is 0, it means that we came from power on
    bool stand_by = Task::s_task_application && Task::s_task_application->is_in_stand_by();
    if (stand_by and _frequency == 0 )
    {
        DEBUG_MSG(DEMUX, INFO, "Checking for OTAs on power on...\n");
        auto tps = MB_Satellites::get_transponder_list_for_snr(Satellite_Operator::Sky);
        if (tps.size() == 0) {
            DEBUG_MSG(DEMUX, DEBUG, "No transponders with SNR found, skipping OTA check\n");
            return;
        }

        auto tp = tps[0]; // Just check the first one, since we only need to trigger the NIT read
        m_tps_to_check.transponder_id.set_frequency(tp.transponder_id.frequency(), tp.transponder_id.polarity(), tp.transponder_id.satellite_id());
        m_tps_to_check.symbol_rate = tp.symbol_rate;
        m_tps_to_check.dvb_mode = DVB_Mode::DVBS2;
        m_tps_to_check.transport_stream_id = tp.transport_stream_id;
        m_tps_to_check.original_network_id = tp.original_network_id;
        Task::post_event_transponder_lock(POST_CALLER &m_tps_to_check);
    }

    // Check if OTA was found by previous check
    auto tps = MB_Satellites::get_transponder_list_for_ota(Satellite_Operator::Sky);
    if (tps.size() == 0)
    {
        using namespace std::placeholders;
        m_demux->clear_nit_callback();
        m_demux->set_nit_callback(std::bind(&Task_Demux::nit_callback, this, _1, _2));
        auto nit_table_id = std::make_shared<Demux::TS_Filter_Table_Id>(NIT_TABLE_ID_ACTUAL);
        table_require(TABLE_REQUIRE_NAME(NIT) NIT_TSPID, nit_table_id);
        DEBUG_MSG(DEMUX, DEBUG, "Checking for OTAs...\n");
    }
    else
    {
        DEBUG_MSG(DEMUX, DEBUG, "OTA already found, skipping check\n");
    }
}

void Task_Demux::check_for_ota_claro(uint32_t _frequency)
{
    // If not in stand-by, do not check for OTA
    bool stand_by = Task::s_task_application && Task::s_task_application->is_in_stand_by();
    if (!stand_by)
    {
        return;
    }

    // Check if OTA was found by previous check
    const char* ota_env = fw_getenv("ota_found");
    bool ota_found = ota_env != nullptr && std::strcmp(ota_env, "1") == 0;
    // If ota was found, skip check
    if (ota_found)
    {
        DEBUG_MSG(DEMUX, INFO, "OTA already found, skipping check\n");
        return;
    }

    // If frequency is 0, it means that we came from power on, start looking for OTA
    auto tps = MB_Satellites::get_transponder_list_for_ota(Satellite_Operator::Claro);
    if (tps.size() == 0) {
        DEBUG_MSG(DEMUX, INFO, "No transponders with OTA found, skipping OTA check\n");
        return;
    }
    auto tp_to_check = tps.begin();
    auto current_tp = std::find_if(tps.begin(), tps.end(), [_frequency](const auto& tp)
    {
        return tp.transponder.transponder_id.frequency() == _frequency;
    });

    if (current_tp != tps.end()) 
    {
        tp_to_check = std::next(current_tp);
        if (tp_to_check == tps.end())
        {
            tp_to_check = tps.begin();
        }
    }

    m_tps_to_check = tp_to_check->transponder;
    DEBUG_MSG(DEMUX, DEBUG, "Checking Claro OTA on frequency: " << std::dec << m_tps_to_check.transponder_id.frequency() << "\n");
    Task::post_event_transponder_lock(POST_CALLER &m_tps_to_check);
    
    // Start OTA check
    m_ota_callback = std::make_shared<Event_OTA_DSI>();
    m_ota_callback->callback = process_ota_callback_claro;
    Task::post_event_ota_update_get(tp_to_check->pid, m_ota_callback);
    DEBUG_MSG(DEMUX, INFO, "Checking for OTAs...\n");
}

void Task_Demux::process_ota_callback_claro(uint32_t _product_id, uint16_t _sw_current, uint16_t _sw_min)
{
    // Valores do software instalado
    auto software_installed_str = MB_OSD_Version::get_major_minor_version_str();
    uint16_t software_installed = std::stoi(software_installed_str);

    DEBUG_MSG(DEMUX, DEBUG, "Product ID: " << std::dec << _product_id << "\n");
    DEBUG_MSG(DEMUX, DEBUG, "Software Version: " << std::dec << _sw_current << "\n");
    DEBUG_MSG(DEMUX, DEBUG, "Software Minimum Version: " << std::dec << _sw_min << "\n");
    DEBUG_MSG(DEMUX, DEBUG, "Software Installed Version: " << std::dec << software_installed << "\n");

    if (software_installed < _sw_current)
    {
        DEBUG_MSG(DEMUX, WARN, "OTA Claro found, setting environment variable\n");
        fw_env_open();
        fw_env_write("ota_found", "1");
        fw_env_close();
    }
}

void Task_Demux::process()
{
    process_tables();
    switch (state())
    {
        case ST_IDLE:
        case ST_PAT_UPDATING:
        case ST_CLOCK_UPDATING:
        case ST_LOCKING:
        case ST_START_EMM_FILTERING:
        {
           if (m_channel_list)
            {
                m_channel_list->process();
            }

            // Check if the channel list parser has finished
            if (m_channel_list && m_channel_list->is_done())
            {
                DEBUG_MSG(DEMUX, INFO, "Channel List parsing complete, finalizing...\n");
                m_channel_list.reset();  // Destructor logs found services
            }

            bool stand_by = Task::s_task_application && Task::s_task_application->is_in_stand_by();
            if (stand_by )
            {
                auto now = decltype(m_last_ota_check)::clock::now();
                if (now - m_last_ota_check > std::chrono::seconds{60})
                {
                    check_for_otas();
                    m_last_ota_check = now;
                }
            }
            break;
        }

        case ST_LINEUP_RUNNING:
        {
            if (m_demux_lineup)
            {
                m_demux_lineup->process();
            }
            break;
        }

        case ST_LINEUP_FINISHED:
        {
            DEBUG_MSG(DEMUX, INFO, "\n");
            if (m_demux_lineup)
            {
                m_demux_lineup->finish();
                m_demux_lineup.reset();
            }
            break;
        }
    }
}

#ifdef MBGUI_PERIODIC_DUMP
void Task_Demux::handle_event_debbug_dump_status()
{
    DEBUG_MSG(TASK, DEBUG, "Task_Demux: " << to_str(state()) << "\n");
    dump_need_tables();
}

#endif  // MBGUI_PERIODIC_DUMP

}  // namespace mb
