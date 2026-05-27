pkg_check_modules(CJSON libcjson)

add_subdirectory(cas)
add_subdirectory(common)
add_subdirectory(dlgui)
add_subdirectory(dvb)
add_subdirectory(hal)
add_subdirectory(tpm)

target_sources(${PROJECT_NAME} PRIVATE
    mb_channel_ranking.cpp
    mb_channel_ranking.h
    mb_demux_lineup.cpp
    mb_demux_lineup.h
    mb_demux_lineup_claro.cpp
    mb_demux_lineup_claro.h
    mb_demux_lineup_sky.cpp
    mb_demux_lineup_sky.h
    mb_demux_channel_list.cpp
    mb_demux_channel_list.h
    mb_demux_lineup_generic.cpp
    mb_demux_lineup_generic.h
    mb_demux_table_manager.cpp
    mb_demux_table_manager.h
    mb_events.h
    mb_main.cpp
    mb_main.h
    mb_remote_control_handler.cpp
    mb_remote_control_handler.h
    mb_zone_id.cpp
    mb_zone_id.h
    tasks/mb_task.cpp
    tasks/mb_task.h
    tasks/mb_task_application.cpp
    tasks/mb_task_application.h
    tasks/mb_task_cas.cpp
    tasks/mb_task_cas.h
    tasks/mb_task_database.cpp
    tasks/mb_task_database.h
    tasks/mb_task_demux.cpp
    tasks/mb_task_demux.h
    #tasks/mb_task_easy_install.cpp
    #tasks/mb_task_easy_install.h
    tasks/mb_task_eit_events.cpp
    tasks/mb_task_eit_events.h
    tasks/mb_task_osd.cpp
    tasks/mb_task_osd.h
    tasks/mb_task_player.cpp
    tasks/mb_task_player.h
    tasks/mb_task_remote_control.cpp
    tasks/mb_task_remote_control.h
    tasks/mb_task_tuner.cpp
    tasks/mb_task_tuner.h
    tasks/private/mb_task_dump_params.cpp
    tasks/private/mb_task_dump_params.h
    tasks/private/mb_task_ipc_messages.cpp
    tasks/private/mb_task_ipc_messages.h
    tasks/private/mb_task_macros.h
    tasks/private/mb_task_mqueue_helpers.cpp
    tasks/private/mb_task_mqueue_helpers.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    MBGUI_APP_GUI=1
    CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}
    SKIP_PEER_VERIFICATION
    CENTURY_CHIP_FAMILY="${CMAKE_SYSTEM_VERSION}"
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wnarrowing
    -Wno-literal-suffix
    -fno-exceptions
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CJSON_INCLUDES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    mbversion
    mbgui_hal
    mbgui_dvb
    mbgui_common
    mbgui_lvgl
    sqlite3
    rt
    pthread
    ${CJSON_LIBRARIES}
)

if(MBGUI_SAT_MONITOR)
    target_sources(sat_monitor PRIVATE
        sm_main.cpp
        sm_main.h
        mb_demux_table_manager.cpp
        mb_demux_table_manager.h
        mb_pg.cpp
        mb_pg.h
        tasks/mb_task.cpp
        tasks/mb_task.h
        tasks/mb_task_demux.cpp
        tasks/mb_task_demux.h
        tasks/mb_task_player.cpp
        tasks/mb_task_player.h
        tasks/mb_task_remote_control.cpp
        tasks/mb_task_remote_control.h
        tasks/mb_task_osd.cpp
        tasks/mb_task_osd.h
        tasks/sm_task_sat_monitor.cpp
        tasks/sm_task_sat_monitor.h
        tasks/mb_task_tuner.cpp
        tasks/mb_task_tuner.h
        tasks/private/mb_task_dump_params.cpp
        tasks/private/mb_task_dump_params.h
        tasks/private/mb_task_macros.h
    )

    pkg_check_modules(LIBPQ REQUIRED libpq)
    message(STATUS "LibPQ CFLAGS: ${LIBPQ_CFLAGS}")
    message(STATUS "LibPQ INCLUDE_DIRS: ${CMAKE_SYSROOT}/${LIBPQ_INCLUDE_DIR}")
    message(STATUS "LibPQ LIBRARY_DIRS: ${CMAKE_SYSROOT}/${LIBPQ_LIBRARY_DIRS}")
    message(STATUS "LibPQ LIBRARIES: ${LIBPQ_LIBRARIES}")

    target_compile_definitions(sat_monitor PRIVATE MBGUI_SAT_MONITOR)
    #target_compile_options(sat_monitor PRIVATE ${LIBPQ_CFLAGS})
    target_include_directories(sat_monitor PRIVATE ${CMAKE_SYSROOT}/usr/include/postgresql/server/;${CMAKE_SYSROOT}/usr/include/postgresql/server/libpq/;${CMAKE_SYSROOT}/usr/include/postgresql/server/lib/)
    target_link_directories(sat_monitor PRIVATE ${CMAKE_SYSROOT}/${LIBPQ_LIBRARY_DIRS})
    target_link_libraries(sat_monitor PRIVATE ${LIBPQ_LIBRARIES})

    set_target_properties(sat_monitor
        PROPERTIES
            VERSION "${PROJECT_VERSION}"
    )

    target_compile_definitions(sat_monitor PRIVATE
        MBGUI_APP_SAT_MONITOR=1
        CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}
        SKIP_PEER_VERIFICATION
        CENTURY_CHIP_FAMILY="${CMAKE_SYSTEM_VERSION}"
        MBGUI_SAT_MONITOR
    )

    target_compile_options(sat_monitor PRIVATE
        -Wall
        -Wextra
        -Wnarrowing
        -Wno-literal-suffix
        -fno-exceptions
    )

    target_include_directories(sat_monitor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIRS}
    )

    target_link_directories(sat_monitor PRIVATE
        ${PostgreSQL_LIBRARY_DIRS}
    )

    target_link_libraries(sat_monitor PRIVATE
        mbversion
        mbgui_hal
        mbgui_dvb
        mbgui_common
        mbgui_lvgl
        sqlite3
        ${CURL_LIBRARIES}
        ${PostgreSQL_LIBRARIES}
        rt
        pthread
    )
endif()
