#pragma once

#include <functional>
#include <memory>

#include "hal/mb_remote_control_keys.h"
#include "hal/mb_gpio.h"
#include <aui_gpio.h>

namespace mb {

class Remote_Control
{
public:
    typedef std::function<bool(Remote_Control_Key)> Key_Handler;

private:
    struct Data;
    std::unique_ptr<Data> m_p;

    void set_standby_wakeup();
    void remote_control_init();

    Key_Handler m_key_handler;
public:
    Remote_Control();
    ~Remote_Control();

    void read_keys();
    void read_keys_front_panel();

    void set_key_handler(Key_Handler _handler)
    {
        m_key_handler = _handler;
    }
};

} // namespace mb
