#include <thread>
#include <chrono>

using namespace std::chrono_literals;

#include "mb_nagra.h"
#include "tasks/mb_task.h"

int main()
{
    mb::Nagra nagra;

    while(true)
    {
        std::this_thread::sleep_for(1s);
    }
}

//Empty function implementations missing in this build

namespace mb {

void Task::post_event_lineup_save_zone_id(Zone_ID_t, Segment_ID_t)
{
}

void Task::post_event_pin_reset()
{
}

}
