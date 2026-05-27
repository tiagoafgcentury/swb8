#pragma once

#include "mb_task.h"
#include "common/mb_types.h"

namespace mb {

class Remote_Control;

class Task_Remote_Control final: public Task
{
    friend class Task;

private:
    std::unique_ptr<Remote_Control> m_remote_control;

    enum State
    {
        ST_IDLE,
        ST_FACTORY_RESET,
    };

    State m_state { ST_IDLE };

protected:
    virtual void handle_event_system_factory_reset() override;

public:
    Task_Remote_Control();
    virtual ~Task_Remote_Control();

    virtual void process() override;
};

}
