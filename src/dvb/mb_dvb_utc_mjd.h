#pragma once

#include <cstdint>
#include <functional>
#include <chrono>

namespace mb {

class UTC_MJD
{
    friend struct std::hash<UTC_MJD>;
private:
    uint16_t m_year { 0 };
    uint8_t m_month { 0 };
    uint8_t m_day { 0 };

    uint8_t m_hour { 0 };
    uint8_t m_minute { 0 };
    uint8_t m_second { 0 };
    uint8_t m_wday { 0 };

    void calculate_wday()
    {
        // Zeller's Congruence algorithm to calculate the day of the week
        int y = m_year - (m_month < 3);
        int m = (m_month + 9) % 12 + 1; // March is month 1, February is month 12
        int d = m_day;
        m_wday = ((d + (13 * m + 1) / 5 + y + y / 4 - y / 100 + y / 400) + 13) % 7;
    }

public:
    explicit UTC_MJD() {}
    explicit UTC_MJD(const uint8_t _data[5]);
    explicit UTC_MJD(uint16_t _year, uint8_t _month, uint8_t _day,
                     uint8_t _hour, uint8_t _minute, uint8_t _second = 0):
        m_year(_year), m_month(_month), m_day(_day),
        m_hour(_hour), m_minute(_minute), m_second(_second)
    {
        calculate_wday();
    }

    explicit UTC_MJD(std::tuple<uint16_t, uint16_t, uint16_t,
                     uint16_t, uint16_t, uint16_t> _time):
        m_year(std::get<0>(_time)), m_month(std::get<1>(_time)), m_day(std::get<2>(_time)),
        m_hour(std::get<3>(_time)), m_minute(std::get<4>(_time)), m_second(std::get<5>(_time))
    {
    }

    explicit UTC_MJD(const std::chrono::system_clock::time_point &t):
        UTC_MJD(std::chrono::system_clock::to_time_t(t))
    {
    }

    explicit UTC_MJD(const time_t &t)
    {
        auto tm = localtime(&t);
        m_year = tm->tm_year + 1900;
        m_month = tm->tm_mon + 1;
        m_day = tm->tm_mday;
        m_hour = tm->tm_hour;
        m_minute = tm->tm_min;
        m_second = tm->tm_sec;
        m_wday = tm->tm_wday;
    }

    auto year() const
    {
        return m_year;
    };
    auto month() const
    {
        return m_month;
    };
    auto day() const
    {
        return m_day;
    };
    auto wday() const
    {
        return m_wday;
    };

    auto hour() const
    {
        return m_hour;
    };
    auto minute() const
    {
        return m_minute;
    };
    auto second() const
    {
        return m_second;
    };

    bool operator==(const UTC_MJD &_other) const
    {
        return m_second == _other.m_second and
               m_minute == _other.m_minute and
               m_hour == _other.m_hour and
               m_day == _other.m_day and
               m_month == _other.m_month and
               m_year == _other.m_year;
    }

    bool operator<(const UTC_MJD &_other) const
    {
        if(m_year < _other.m_year)
        {
            return true;
        }

        if(m_year > _other.m_year)
        {
            return false;
        }

        if(m_month < _other.m_month)
        {
            return true;
        }

        if(m_month > _other.m_month)
        {
            return false;
        }

        if(m_day < _other.m_day)
        {
            return true;
        }

        if(m_day > _other.m_day)
        {
            return false;
        }

        if(m_hour < _other.m_hour)
        {
            return true;
        }

        if(m_hour > _other.m_hour)
        {
            return false;
        }

        if(m_minute < _other.m_minute)
        {
            return true;
        }

        if(m_minute > _other.m_minute)
        {
            return false;
        }

        if(m_second < _other.m_second)
        {
            return true;
        }

        return false;
    }

    bool operator>=(const UTC_MJD &_other) const
    {
        return not(*this < _other);
    }

    UTC_MJD operator+(const std::chrono::seconds &_s) const;
    UTC_MJD operator-(const UTC_MJD &_other) const;

    const char *time_to_str() const;
    const char *date_time_to_str() const;
    UTC_MJD to_local_time() const;
    UTC_MJD to_system_time() const;
    time_t to_unix_epoch() const;

    template<typename Clock>
    std::chrono::time_point<Clock> to_time_point() const
    {
        return Clock::from_time_t(to_unix_epoch());
    }

    static UTC_MJD now();
};

};

#ifdef MBGUI_HASH_COMBINE
namespace std {
template<>
struct hash<mb::UTC_MJD>
{
    size_t operator()(const mb::UTC_MJD &v) const noexcept
    {
        size_t result = { 0 };
        mb::hash_combine(result, v.m_year, v.m_month, v.m_day, v.m_hour, v.m_minute, v.m_second);
        return result;
    }
};
}
#endif // MBGUI_HASH_COMBINE
