#include "mb_dvb_subtitle_parser.h"
#include "dvb/mb_dvb_globals.h"
#include "mb_demux.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_player.h"


#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#endif


namespace mb
{

DVB_Subtitle_Parser::DVB_Subtitle_Parser()
{

}
DVB_Subtitle_Parser::~DVB_Subtitle_Parser()
{

}

void DVB_Subtitle_Parser::parse_subtitle_segment(const uint8_t* data, size_t size, uint64_t pts)
{

    if (!data || size <= 10)
    {
        return;
    }

    Event_Subtitle_Image evt;
    evt.pts = pts;
    evt.data.assign(data, data + size);

    Task::post_event_subtitle(evt);
}


} // namespace mb
