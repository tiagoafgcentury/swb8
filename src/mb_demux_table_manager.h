#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <memory>

#include "hal/mb_demux.h"

namespace mb {

class IDemux_Table_Manager
{
protected:
    struct TableItem
    {
        TableItem(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid_id, Demux::TS_Filter_Table_Ptr _filters, bool _check_crc, Demux::Data_Type _data_type):
            pid(_pid_id),
            filters(_filters),
            check_crc(_check_crc),
            data_type(_data_type)
        {
#ifndef NDEBUG
            table_name = _table_name;
#endif
        }

        PID_t pid { 0 };
        Demux::TS_Filter_Table_Ptr filters;
        bool active { false };
        bool done { false };
        bool check_crc { true };
        Demux::Data_Type data_type = Demux::Data_Type::SECTION;

#ifndef NDEBUG
        const char *table_name { nullptr };
#endif

        bool operator==(const TableItem &_other) const
        {
            Service_ID_t _prog_number = 0;
            Service_ID_t _other_prog_number = 0;
            if (pid != _other.pid)
            {
                return false;
            }

            if (filters->value[0] == PMT_TABLE_ID)
            {
                _prog_number = (filters->value[3] << 8) | filters->value[4];
                _other_prog_number = (_other.filters->value[3] << 8) | _other.filters->value[4];
            }

            if (filters->value[0] != _other.filters->value[0])
            {
                return false;
            }

            if (_prog_number != _other_prog_number)
            {
                    return false;
            }

            return true;
        }

        bool operator<(const TableItem &_other) const
        {
            Service_ID_t _prog_number = 0;
            Service_ID_t _other_prog_number = 0;
            if (pid != _other.pid)
            {
                return pid < _other.pid;
            }

            if (filters->value[0] == PMT_TABLE_ID)
            {
                _prog_number = (filters->value[3] << 8) | filters->value[4];
                _other_prog_number = (_other.filters->value[3] << 8) | _other.filters->value[4];
            }

            if (filters->value[0] != _other.filters->value[0])
            {
                return filters->value[0] < _other.filters->value[0];
            }

            if (_prog_number != _other_prog_number)
            {
                return _prog_number < _other_prog_number;
            }

            return false;
        }

        size_t hash() const;
    };

    typedef std::vector<TableItem> Table_Queue;
    Table_Queue m_table_queue;

    uint8_t max_parallel_tables() const
    {
        return 30;
    }

public:
    void remove_need_table(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t pid, Demux::TS_Filter_Table_Ptr _filters, Demux::Data_Type _data_type);
    bool table_require(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t pid, Demux::TS_Filter_Table_Ptr _filters, bool _replace_if_present = false, bool _check_crc = true, Demux::Data_Type _data_type = Demux::Data_Type::SECTION);
    virtual void clear_table_queue();

    void process_tables();

    virtual Demux *get_demux() = 0;

#ifdef MBGUI_PERIODIC_DUMP
    void dump_need_tables();
#endif
};


}
