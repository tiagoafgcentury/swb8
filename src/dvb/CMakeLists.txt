add_library(mbgui_dvb STATIC)

target_sources(mbgui_dvb PUBLIC
    mb_dvb_bat.h
    mb_dvb_cat.h
    mb_dvb_century.h
    mb_dvb_eit.h
    mb_dvb_eitv.h
    mb_dvb_globals.h
    mb_dvb_idescriptor_interface.h
    mb_dvb_itransport_stream_interface.h
    mb_dvb_nit.h
    mb_dvb_ota.h
    mb_dvb_pat.h
    mb_dvb_pmt.h
    mb_dvb_sdt.h
    mb_dvb_si_table.h
    mb_dvb_skw.h
    mb_dvb_tables.h
    mb_dvb_tdt.h
    mb_dvb_tot.h
    mb_dvb_transport_stream.h
    mb_dvb_types.h
    mb_dvb_utc_mjd.h
)

target_sources(mbgui_dvb PRIVATE
    mb_dvb_bat.cpp
    mb_dvb_cat.cpp
    mb_dvb_century.cpp
    mb_dvb_eit.cpp
    mb_dvb_eitv.cpp
    mb_dvb_globals.cpp
    mb_dvb_idescriptor_interface.cpp
    mb_dvb_itransport_stream_interface.cpp
    mb_dvb_nit.cpp
    mb_dvb_ota.cpp
    mb_dvb_pat.cpp
    mb_dvb_pmt.cpp
    mb_dvb_sdt.cpp
    mb_dvb_si_table.cpp
    mb_dvb_skw.cpp
    mb_dvb_tdt.cpp
    mb_dvb_tot.cpp
    mb_dvb_transport_stream.cpp
    mb_dvb_types.cpp
    mb_dvb_utc_mjd.cpp
)

target_include_directories(mbgui_dvb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
