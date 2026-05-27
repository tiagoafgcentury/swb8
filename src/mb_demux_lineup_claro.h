#pragma once

#include "mb_demux_lineup.h"

#include <unordered_map>

namespace mb {

class Demux_Lineup_Claro : public Demux_Lineup
{
public:
    Demux_Lineup_Claro(Task_Demux *_task_demux);
    virtual ~Demux_Lineup_Claro();

    static constexpr auto BAT_RETRY_COUNT = 2;

protected:
    typedef uint32_t BAT_Index_t;

    std::unordered_map<Bouquet_ID_t, Viewer_Channel_t> m_bouquet_cannels;
    std::unordered_map<BAT_Index_t, uint32_t> m_bouquet_seen_count;

public:
    virtual void parse_bat(BAT &&_bat, Lineup *_lineup) override;
    virtual void parse_nit(NIT &&_nit, Lineup *_lineup) override;
    virtual void parse_sdt(SDT &&_sdt, Lineup *_lineup) override;
    virtual void parse_tables(Lineup *_lineup) override;

    virtual bool check_bats_is_done(Bouquet_ID_t _bat_id) override;
    virtual bool check_sdts_is_done() override;
};

}
