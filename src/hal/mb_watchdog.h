#pragma once

#include <memory>

namespace mb {

class Watchdog
{
protected:
    struct Data;
    std::unique_ptr<Data> m_p;

public:
    Watchdog();
    ~Watchdog();

    void enable();
    void disable();

    void ping();

    static void system_reset();
};

} // namespace mb
