#include "mb_remote_control_handler.h"

#include "common/mb_globals.h"
#include <list>

#ifndef NDEBUG
#include <typeinfo>
#endif

namespace mb {

static std::list<Remote_Control_Handler *> s_handlers;

Remote_Control_Handler::~Remote_Control_Handler()
{
    remove_focus();
}

void Remote_Control_Handler::post_event_remote_control(Event_Remote_Control _event)
{
    for(auto it = s_handlers.rbegin(); it != s_handlers.rend(); it++)
    {
        auto hnd = *it;
        //DEBUG_MSG(COMMON, INFO, "Post: " << to_str(_event.key) << " to " << typeid(*hnd).name() << "\n");
        if(hnd->handle_event_remote_control(_event))
        {
            return;
        }
    }
}

void Remote_Control_Handler::set_focus()
{
    //DEBUG_MSG(COMMON, INFO, "Set focus: " << typeid(*this).name() << "\n");
    s_handlers.push_back(this);
}

void Remote_Control_Handler::remove_focus()
{
    auto current_focus = *s_handlers.rbegin();

    for(auto it = s_handlers.begin(); it != s_handlers.end();)
    {
        if(*it == this)
        {
            //DEBUG_MSG(COMMON, INFO, "Remove focus: " << typeid(this).name() << "\n");
            it = s_handlers.erase(it);
        }
        else
        {
            it++;
        }
    }

    if(!s_handlers.empty() && *s_handlers.rbegin() != current_focus)
    {
        auto new_focus = *s_handlers.rbegin();
        //DEBUG_MSG(COMMON, INFO, "Got focus: " << typeid(*new_focus).name() << "\n");
        new_focus->got_focus();
    }
}

void Remote_Control_Handler::got_focus()
{
}

} // namespace mb
