target_sources(mbgui_hal PRIVATE
    mb_ali_globals.cpp
    mb_demux.cpp
    mb_diseqc.cpp
    mb_display.cpp
    mb_gpio.cpp
    mb_hdmi.cpp
    mb_otp.cpp
    mb_player.cpp
    mb_media_player.cpp
    mb_remote_control.cpp
    mb_sound.cpp
    mb_system.cpp
    mb_tuner.cpp
    mb_watchdog.cpp
)

if(MBGUI_NET_HLS)
    find_package(CURL REQUIRED)
    target_compile_definitions(mbgui_hal PRIVATE MBGUI_NET_HLS)
    target_sources(mbgui_hal PRIVATE
        mb_player_hls.cpp
    )
    target_link_libraries(mbgui_hal PRIVATE CURL::CURL)

    find_package(Boost REQUIRED COMPONENTS regex exception)
    target_link_libraries(mbgui_hal PRIVATE Boost::regex Boost::exception)
endif()

target_link_libraries(mbgui_hal PRIVATE aui) #alisldis

