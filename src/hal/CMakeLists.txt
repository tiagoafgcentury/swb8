add_library(mbgui_hal STATIC)

target_sources(mbgui_hal PRIVATE
    mb_demux.h
    mb_demux_common.cpp
    mb_demux_macros.h
    mb_dvb_subtitle_parser.h
    mb_dvb_subtitle_parser.cpp
    mb_diseqc.h
    mb_display.h
    mb_hdmi.h
    mb_lnb_config.cpp
    mb_lnb_config.h
    mb_player.h
    mb_player_cc.cpp
    mb_media_player.h
    mb_remote_control.h
    mb_remote_control_keys.cpp
    mb_remote_control_keys.h
    mb_system.h
    mb_tuner.h
    mb_tuner_signal_info.h
    mb_watchdog.h
)

target_include_directories(mbgui_hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mb_hal_system_select_seed.h ${CMAKE_CURRENT_SOURCE_DIR}/mb_hal_system_select.h)

add_subdirectory(${CMAKE_SYSTEM_VERSION})
