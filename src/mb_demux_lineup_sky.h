#pragma once

#include "mb_demux_table_manager.h"
#include "mb_demux_lineup.h"
#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_tables.h"
#include "hal/mb_demux_macros.h"
#include "mb_table_map.h"

#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <map>
#include <limits>
#include <unordered_set>

namespace mb {

class Demux_Lineup_Sky : public Demux_Lineup
{
public:
    Demux_Lineup_Sky(Task_Demux *_task_demux);
    virtual ~Demux_Lineup_Sky();

    virtual void parse_bat(BAT &&_bat, Lineup *_lineup) override;
    virtual void parse_nit(NIT &&_nit, Lineup *_lineup) override;
    virtual void parse_sdt(SDT &&_sdt, Lineup *_lineup) override;
    virtual void parse_tables(Lineup *_lineup) override;

    virtual bool check_bats_is_done(Bouquet_ID_t _bat_id) override;
    virtual bool check_sdts_is_done() override;

    virtual void transponder_lock(const Event_Tuner_Lock &_event) override;

    virtual void start_tp() override;
    virtual void finish_tp() override;
};

}
