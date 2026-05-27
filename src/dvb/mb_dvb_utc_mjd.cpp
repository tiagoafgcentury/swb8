#include "mb_dvb_utc_mjd.h"
#include "common/mb_globals.h"

#include <cstring>
#include <common/mb_types.h>
#include <hal/mb_system.h>

namespace {

int bcd2dec(int bcd)
{
    return ((bcd >> 4) * 10) + bcd % 16;
}

}

namespace mb {

UTC_MJD::UTC_MJD(const uint8_t _data[5])
{
    UTC_MJD result;
    uint16_t mjd = (_data[0] << 8) | _data[1];
    uint32_t bcd = ((_data[2] << 16) | (_data[3] << 8)) | _data[4];

    if(mjd > 0)
    {
        // Copied from ETSI EN 300 468 (ANNEX C)
        result.m_year  = (int)((mjd - 15078.2) / 365.25);
        result.m_month = (int)((mjd - 14956.1 - (int)(result.m_year * 365.25)) / 30.6001);
        result.m_day   = mjd - 14956 - (int)(result.m_year * 365.25) - (int)(result.m_month * 30.6001);
        auto tmp = (result.m_month == 14 || result.m_month == 15) ? 1 : 0;
        result.m_year  = result.m_year + tmp;
        result.m_month = result.m_month - 1 - tmp * 12;
        result.m_year  += 1900;
    }

    if(bcd > 0)
    {
        result.m_hour    = bcd2dec((bcd & ~ 0xff00ffff) >> 16);                         // 11111111 xxxxxxxx xxxxxxxx
        result.m_minute  = bcd2dec((bcd & ~ 0xffff00ff) >> 8);                          // xxxxxxxx 11111111 xxxxxxxx
        result.m_second  = bcd2dec((bcd & ~ 0xffffff00));                               // xxxxxxxx xxxxxxxx 11111111
    }

    *this = result; //+ std::chrono::hours(g_timezone_offset);
    calculate_wday();
}

UTC_MJD UTC_MJD::operator+(const std::chrono::seconds &_s) const
{
    std::tm t;
    MB_ZERO(t);
    t.tm_year = m_year - 1900;
    t.tm_mon = m_month - 1;
    t.tm_mday = m_day;
    t.tm_hour = m_hour;
    t.tm_min = m_minute;
    t.tm_sec = m_second;
    return UTC_MJD(mktime(&t) + _s.count());
}

UTC_MJD UTC_MJD::operator-(const UTC_MJD &_other) const
{
    std::tm t;
    MB_ZERO(t);
    t.tm_year = m_year - 1900;
    t.tm_mon = m_month - 1;
    t.tm_mday = m_day;
    t.tm_hour = m_hour;
    t.tm_min = m_minute;
    t.tm_sec = m_second;
    return UTC_MJD(mktime(&t) - _other.to_unix_epoch());
}

static char buffer[18];

const char *UTC_MJD::time_to_str() const
{
    snprintf(buffer, sizeof(buffer), "%.2d:%.2d", m_hour, m_minute);
    return buffer;
}

const char *UTC_MJD::date_time_to_str() const
{
    snprintf(buffer, sizeof(buffer), "%.2d/%.2d/%.4d %.2d:%.2d", m_day, m_month, m_year, m_hour, m_minute);
    return buffer;
}

UTC_MJD UTC_MJD::to_local_time() const
{
    if(static_cast<Clock_Type>(g_clock_set) != Clock_Type::Manual)
    {
        UTC_MJD result {*this};
        result = result + std::chrono::seconds{g_timezone_offset * 3600};  // Converter horas para segundos
        return result;
    }
    else
    {
        return *this;
    }
}

UTC_MJD UTC_MJD::to_system_time() const
{
    if(static_cast<Clock_Type>(g_clock_set) != Clock_Type::Manual)
    {
        UTC_MJD result {*this};
        result = result + std::chrono::seconds{g_timezone_offset * -3600};  // Converter horas para segundos
        return result;
    }
    else
    {
        return *this;
    }
}

time_t UTC_MJD::to_unix_epoch() const
{
    std::tm t = {};
    t.tm_year = m_year - 1900;
    t.tm_mon = m_month - 1;
    t.tm_mday = m_day;
    t.tm_hour = m_hour;
    t.tm_min = m_minute;
    t.tm_sec = m_second;
    return mktime(&t);
}

UTC_MJD UTC_MJD::now()
{
    return System::get_system_time();
}

} // namespace mb
