add_library(mb_hal STATIC)

target_sources(mbgui_hal PRIVATE
    mb_demux.cpp
    mb_display.cpp
    mb_tuner.cpp
    mb_hdmi.cpp
    mb_player_hls.cpp
    mb_player.cpp
    mb_remote_control.cpp
    mb_sound.cpp
    mb_system.cpp
    mb_montage_globals.cpp
    mb_montage_globals.h
)
