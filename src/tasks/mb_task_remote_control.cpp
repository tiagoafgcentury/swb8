#include "mb_task_remote_control.h"

#include "hal/mb_remote_control.h"
#include "hal/mb_system.h"
#include "common/mb_assert.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"

#include "mb_remote_control_handler.h"
#include "mb_events.h"
#include "mb_main.h"

namespace mb {

Task_Remote_Control::Task_Remote_Control():
    m_remote_control(std::make_unique<Remote_Control>())
{
    mb_assert(s_task_remote_control == nullptr);
    s_task_remote_control = this;
    m_remote_control->set_key_handler([](Remote_Control_Key _key)
    {
        if(_key == Remote_Control_Key::KEY_POWER)
        {
            auto _mp_state = Task::s_task_player->get_media_player_state();
            auto _mpvr_state = Task::s_task_player->get_pvr_player_state();
            if ((_mp_state != Media_Player::State::Idle) or (_mpvr_state != Task_Player::PVR_State::Idle))
            {
                Remote_Control_Handler::post_event_remote_control({ .key = _key });
            }
            else
            {
                post_event_toggle_power();
            }
        }
        else
        {
            Remote_Control_Handler::post_event_remote_control({ .key = _key });
        }

        return true;
    });
}

Task_Remote_Control::~Task_Remote_Control()
{
    mb_assert(s_task_remote_control == this);
    s_task_remote_control = nullptr;
}

void Task_Remote_Control::handle_event_system_factory_reset()
{
    m_state = ST_FACTORY_RESET;
}

void Task_Remote_Control::process()
{
    if(ST_IDLE == m_state)
    {
        m_remote_control->read_keys();
        m_remote_control->read_keys_front_panel();
    }
}

} // namespace mb
