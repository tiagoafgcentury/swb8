#pragma once

#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "hal/mb_system.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include <map>
#include <chrono>
#include <ctime>

namespace mb {

class Channel_Ranking
{
private:
#ifdef MBGUI_USE_CHANNEL_RANKING
    typedef struct
    {
        std::string name;
        int minutes;
        int service_id;
    } ranking_t;

    std::map<int, ranking_t> service_ranking;

    int current_service_id = 0;
    int current_service_start = 0;

    std::string previous_name;
    int previous_service_id = 0;
    Viewer_Channel_t previous_viewer_channel = 0;
    UTC_MJD previous_start;

    std::chrono::time_point<std::chrono::system_clock> start_service;
    std::chrono::time_point<std::chrono::system_clock> end_service;

    static Channel_Ranking ranking_instance;

public:
    virtual ~Channel_Ranking();

#endif // MBGUI_USE_CHANNEL_RANKING

public:
    void save_ranking();
    static void new_channel_ranking(const Service *srv);
};

} // namespace mb

