#pragma once

#include "hal/mb_hdmi.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include <cstdint>
#include <memory>
#include <tuple>

namespace mb {

class System
{
protected:
    struct Data;
    std::unique_ptr<Data> m_p;

    static System *s_instance;

private:
    enum class LED_Color
    {
        RED,
        GREEN,
        ORANGE,
    };

    std::unique_ptr<HDMI> m_hdmi;

    void fake_power_off();
    void fake_power_on();
    void set_led_color(LED_Color _color);

public:
    System(bool _production_final_test);
    ~System();

    static void poweroff_tpm();
    static bool stand_by(bool _production_final_test);
    static void reboot();
    static void check_standby_mode(bool _production_final_test);

    enum class Suppliers
    {
        Askey,
        Skyworth,
        UNKNOWN
    };

    Suppliers read_supplier();

    static bool set_system_time(UTC_MJD _time);
    static UTC_MJD get_system_time();

    bool static fake_stand_by_mode();

    static std::string get_board_fingerprint();
};

} // namespace mb
