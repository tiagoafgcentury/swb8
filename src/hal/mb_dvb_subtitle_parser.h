#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <list>
#include <memory>
#include <iostream>
#include <algorithm>

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_events.h"



namespace mb {


class DVB_Subtitle_Parser
{

public:
    DVB_Subtitle_Parser();
    virtual ~DVB_Subtitle_Parser();

    void parse_subtitle_segment(const uint8_t* data, size_t len, uint64_t pts);
};

} // namespace mb
