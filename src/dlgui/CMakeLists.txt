add_executable(mbdlgui)

target_sources(mbdlgui PRIVATE
    mb_main_dlgui.cpp
    ../hal/ALi/mb_ali_globals.cpp
    ${GIT_VERSION_FILE}
)

target_link_libraries(mbdlgui PRIVATE
    lvgl
    aui
)

install(TARGETS mbdlgui DESTINATION bin)
