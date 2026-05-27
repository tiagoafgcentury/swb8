#pragma once

#include "dvb/mb_dvb_tables.h"

#include <vector>
#include <optional>
#include <unordered_map>

namespace mb {

template<class TABLE>
inline bool check_if_done(const std::vector<std::optional<TABLE>> &vec)
{
    for(const auto &section : vec)
    {
        if(not section.has_value())
        {
            return false;
        }
    }

    return true;
}

template<>
inline bool check_if_done(const std::vector<std::optional<BAT>> &)
{
    return false;
}

template<class TABLE>
struct Parsable_Table
{
    Parsable_Table(TABLE &&_table):
        table(std::move(_table))
    {
    }

    bool is_parsed { false };
    TABLE table;
};

template<typename KEY, class TABLE>
class Table_Map : public std::unordered_map<KEY, std::vector<std::optional<Parsable_Table<TABLE>>>>
{
public:
    bool generic_table_callback(TABLE &&_table)
    {
        auto id = get_table_id(_table);
        auto it = Table_Map<KEY, TABLE>::find(id);
        const size_t section_number = _table.section_number();
        const size_t last_section_number = _table.last_section_number();

        if(it == Table_Map<KEY, TABLE>::end())
        {
            std::vector<std::optional<Parsable_Table<TABLE>>> vec;
            vec.resize(last_section_number + 1);
            vec[section_number] = std::move(_table);
            Table_Map<KEY, TABLE>::emplace(id, std::move(vec));
            return (section_number == 0) && (last_section_number == 0);
        }
        else
        {
            auto &vec = it->second;

            if(vec.size() != last_section_number + 1)
            {
                goto RESET_VECTOR;
            }

            for(const auto &t : vec)
            {
                if(t.has_value() && t->table.version_number() != _table.version_number())
                {
                    goto RESET_VECTOR;
                }
            }

            if(vec[section_number].has_value())
            {
                return true;
            }

            vec[section_number] = std::move(_table);
            return check_if_done(vec);
RESET_VECTOR:
            vec.clear();
            vec.resize(last_section_number + 1);
            vec[section_number] = std::move(_table);
            return check_if_done(vec);
        }
    }
};

} // namespace mb
