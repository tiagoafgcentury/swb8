#pragma once

#include "mb_events.h"

namespace mb {

class Remote_Control_Handler
{
    friend class Task_Remote_Control;

public:
    virtual ~Remote_Control_Handler();

protected:
    void set_focus();
    void remove_focus();

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) = 0;
    virtual void got_focus();

public:
    static void post_event_remote_control(Event_Remote_Control _event);
};

} // namespace mb
