#pragma once

#include "mb_dvb_bat.h"
#include "mb_dvb_cat.h"
#include "mb_dvb_eitv.h"
#include "mb_dvb_century.h"
#include "mb_dvb_eit.h"
#include "mb_dvb_ota.h"
#include "mb_dvb_globals.h"
#include "mb_dvb_idescriptor_interface.h"
#include "mb_dvb_itransport_stream_interface.h"
#include "mb_dvb_nit.h"
#include "mb_dvb_pat.h"
#include "mb_dvb_pmt.h"
#include "mb_dvb_sdt.h"
#include "mb_dvb_si_table.h"
#include "mb_dvb_skw.h"
#include "mb_dvb_tdt.h"
#include "mb_dvb_tot.h"
#include "mb_dvb_transport_stream.h"
#include "mb_dvb_utc_mjd.h"

namespace mb {

inline auto get_table_id(const BAT &_bat)
{
    return _bat.bouquet_id();
}

inline auto get_table_id(const PMT &_pmt)
{
    return _pmt.program_number();
}

inline auto get_table_id(const SDT &_sdt)
{
    return _sdt.transport_stream_id();
}

};
