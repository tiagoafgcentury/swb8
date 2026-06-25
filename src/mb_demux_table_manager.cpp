#include "mb_demux_table_manager.h"
#include "common/mb_hash.h"
#include "common/mb_globals.h"

#include <algorithm>
#include <chrono>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

#define DUMP_TABLE(EVENT) DEBUG_MSG(DEMUX, DEBUG, EVENT << hex << " " << _table_name << " PID: 0x" << setfill('0') << setw(4) << (int)_pid << " Table: 0x" << setfill('0') << setw(2) << (int)_filters->value[0] << "\n" << dec)

namespace mb {

namespace {

bool same_filter(const Demux::TS_Filter_Table& lhs, const Demux::TS_Filter_Table& rhs)
{
    if(lhs.size != rhs.size)
    {
        return false;
    }

    if(lhs.size == 0)
    {
        return true;
    }

    return lhs.value && rhs.value &&
           lhs.mask && rhs.mask &&
           lhs.reverse && rhs.reverse &&
           std::equal(lhs.value, lhs.value + lhs.size, rhs.value) &&
           std::equal(lhs.mask, lhs.mask + lhs.size, rhs.mask) &&
           std::equal(lhs.reverse, lhs.reverse + lhs.size, rhs.reverse);
}

} // namespace

bool IDemux_Table_Manager::table_require(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, Demux::TS_Filter_Table_Ptr _filters, bool _replace_if_present, bool _check_crc, Demux::Data_Type _data_type)
{
    mb_assert(get_demux()->is_in_read_data() == false);
    using namespace std;
    auto it = find_if(begin(m_table_queue), end(m_table_queue),
                      [ = ](const auto & t)
    {
        if (t.pid != _pid)
            return false;

        if (t.filters->value[0] != _filters->value[0])
            return false;

        if (t.filters->value[0] == PMT_TABLE_ID)
        {
            auto _t_prog_number = (t.filters->value[3] << 8) | t.filters->value[4];
            auto _prog_number = (_filters->value[3] << 8) | _filters->value[4];
            if (_t_prog_number != _prog_number)
            {
                return false;
            }
        }
        return true;
    }
                     );

    if(it == end(m_table_queue))
    {
        DUMP_TABLE("Start " << _table_name << "\n");
        m_table_queue.emplace_back(TABLE_REQUIRE_NAME_PARAM _pid, std::move(_filters), _check_crc, _data_type);
        return true;
    }
    else
    {
        if(_replace_if_present)
        {
            if(!it->done && it->data_type == _data_type && same_filter(*it->filters, *_filters))
            {
                DUMP_TABLE("Already started " << _table_name << "\n");
                return false;
            }

            DUMP_TABLE("Replace " << _table_name << "\n");
            Service_ID_t _service_id = 0;
            if (it->filters->value[0] == PMT_TABLE_ID)
            {
                _service_id = (it->filters->value[3] << 8) | it->filters->value[4];
            }

            auto demux { get_demux() };
            demux->stop(
#ifndef NDEBUG
                it->table_name,
#endif
                it->pid, it->filters->value[0], _service_id);
            m_table_queue.erase(it);
            m_table_queue.emplace_back(TABLE_REQUIRE_NAME_PARAM _pid, std::move(_filters), _check_crc, _data_type);
            return true;
        }
        else
        {
           // DUMP_TABLE("Already Started " << _table_name << "\n");
        }
    }

    return false;
}

void IDemux_Table_Manager::clear_table_queue()
{
    mb_assert(get_demux()->is_in_read_data() == false);
    m_table_queue = Table_Queue();
    get_demux()->stop();
}

void IDemux_Table_Manager::remove_need_table(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, Demux::TS_Filter_Table_Ptr _filters, Demux::Data_Type _data_type)
{
    DUMP_TABLE("Stop");

    Service_ID_t _t_service_id = 0;
    Service_ID_t _service_id = 0;
    if (!_filters)
    {
        return;
    }

    if (_filters->value[0] == PMT_TABLE_ID)
    {
        _service_id = (_filters->value[3] << 8) | _filters->value[4];
    }

    for(auto &t : m_table_queue)
    {
        if (t.filters->value[0] == PMT_TABLE_ID)
        {
            _t_service_id = (t.filters->value[3] << 8) | t.filters->value[4];
        }
        if(_data_type == Demux::Data_Type::SECTION)
        {
            if (t.pid == _pid and t.filters->value[0] == _filters->value[0] and ((t.filters->value[0] == PMT_TABLE_ID) ? _t_service_id == _service_id : true))
            {

                t.done = true;
                get_demux()->pause(
        #ifndef NDEBUG
                    t.table_name,
        #endif
                    t.pid, t.filters->value[0], _service_id);
                    return;
            }
        }
        else
        {
            if(t.pid == _pid)
            {
                t.done = true;
                get_demux()->pause(
    #ifndef NDEBUG
                    t.table_name,
    #endif
                    t.pid, t.filters->value[0], _service_id);
                return;
            }
        }
    }
}

void IDemux_Table_Manager::process_tables()
{
    using namespace std::chrono_literals;

    if(m_table_queue.empty())
    {
        return;
    }

    auto demux { get_demux() };
    demux->read_data(5ms);
    auto active_count { 0 };

    for(auto it = m_table_queue.begin(); it != m_table_queue.end();)
    {
        if(it->done)
        {
            Service_ID_t _service_id = 0;
            if (it->filters->value[0] == PMT_TABLE_ID)
            {
                _service_id = (it->filters->value[3] << 8) | it->filters->value[4];
            }

            demux->stop(
#ifndef NDEBUG
                it->table_name,
#endif
                it->pid, it->filters->value[0], _service_id);
            m_table_queue.erase(it);
        }
        else
        {
            if(it->active)
            {
                active_count++;
            }

            it++;
        }
    }

    for(auto it = m_table_queue.begin(); it != m_table_queue.end(); it++)
    {
        if(active_count >= max_parallel_tables())
        {
            break;
        }

        if(!it->active)
        {
            active_count++;
            it->active = true;
            demux->start(
#ifndef NDEBUG
                it->table_name,
#endif
                it->pid, it->filters, it->check_crc, it->data_type);
        }
    }
}

#ifdef MBGUI_PERIODIC_DUMP
void IDemux_Table_Manager::dump_need_tables()
{
    using namespace std;
    cout << "Need Tables:" << dec << m_table_queue.size() << "\n" << hex;

    for(const auto &t : m_table_queue)
    {
        cout << "\t" << t.table_name <<
             " PID: 0x" << setfill('0') << setw(4) << (int)t.pid  <<
             " Table: 0x" << setfill('0') << setw(2) << (int)t.filters->value[0] <<
             "\n";
    }

    cout << "\n";
}

#endif // MBGUI_PERIODIC_DUMP

} // namespace mb
