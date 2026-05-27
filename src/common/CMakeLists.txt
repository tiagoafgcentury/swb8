add_library(mbgui_common STATIC)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mb_service_categories.h
                    COMMENT "Gerando classificação hard-coded de serviços"
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../../extra/gen_service_category_map.sh > ${CMAKE_CURRENT_BINARY_DIR}/mb_service_categories.h
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(mbgui_common PRIVATE
    mb_assert.h
    mb_config.cpp
    mb_config.h
    mb_hash.cpp
    mb_hash.h
    mb_globals.cpp
    mb_globals.h
    mb_lineup.cpp
    mb_lineup.h
    mb_types.cpp
    mb_types.h
    mb_version.h
    mb_satellites.cpp
    mb_satellites.h
    ${CMAKE_CURRENT_BINARY_DIR}/mb_service_categories.h
)

if(MBGUI_SAT_MONITOR)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mb_ip_sat_monitor_seed.h ${CMAKE_CURRENT_SOURCE_DIR}/mb_ip_sat_monitor.h)
    target_compile_definitions(mbgui_common PRIVATE MBGUI_SAT_MONITOR)
endif()

target_include_directories(mbgui_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(mbgui_common PUBLIC sqlite3 debug cap)
