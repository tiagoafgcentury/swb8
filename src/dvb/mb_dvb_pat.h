#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include "mb_dvb_globals.h"
#include "mb_dvb_si_table.h"
#include "common/mb_types.h"

namespace mb {

class PAT final : public SI_Table
{
public:
    struct Program
    {
        explicit Program(uint16_t _program, PID_t _pid):
            program(_program),
            pid(_pid)
        {}
        uint16_t program { 0 };
        PID_t pid: 13;

#ifndef NDEBUG
        void hash(std::size_t &seed) const;
#endif
    };

private:
    uint8_t _version_number: 5;
    uint8_t _current_next: 1;
    Section_Number_t _section_number { 0 };
    Section_Number_t _last_section_number { 0 };
    std::vector<Program> _programs;

public:
    explicit PAT(const uint8_t *_data, size_t _size);
    virtual ~PAT() {}

    virtual Table_ID_t table_id() const override
    {
        return PAT_TABLE_ID;
    }

    auto version_number() const
    {
        return _version_number;
    }

    auto current_next() const
    {
        return _current_next;
    }

    auto section_number() const
    {
        return _section_number;
    }

    auto last_section_number() const
    {
        return _last_section_number;
    }

    auto move_programs()
    {
        return std::move(_programs);
    };

#ifndef NDEBUG
    virtual void hash(std::size_t &seed) const override;

    virtual std::size_t hash() const override
    {
        return SI_Table::hash();
    }
#endif
};

};
