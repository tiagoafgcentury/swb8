pkg_check_modules(MICROHTTPD libmicrohttpd)

if (CJSON_FOUND AND MICROHTTPD_FOUND)

    # Main mbgui

    target_sources(${PROJECT_NAME} PRIVATE
        mb_task_http_server.cpp
        mb_task_http_server.h
        mb_http_utils.cpp
        mb_http_utils.h
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${MICROHTTPD_LIBRARIES}
    )

    target_include_directories(${PROJECT_NAME} PRIVATE ${MICROHTTPD_INCLUDES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE MB_USE_MICROHTTPD)

    # TPM

    add_executable(mbtpm)

    target_sources(mbtpm PRIVATE
        ../../git_version.cpp
        ../tasks/mb_task.cpp
        ../tasks/mb_task.h
        ../mb_remote_control_handler.cpp
        ../mb_remote_control_handler.h

        mb_connection_context.cpp
        mb_connection_context.h
        mb_main_tpm.cpp
        mb_task_http_server.cpp
        mb_task_http_server.h
        mb_http_utils.cpp
        mb_http_utils.h
        mb_tpm.cpp
        mb_tpm.h
        tpm_api.c
        tpm_api.h
        tpm_init.c
        tpm_init.h
        mb_tpm_utils.cpp
        mb_tpm_utils.h
        ${GIT_VERSION_FILE}
    )

    target_link_libraries(mbtpm PRIVATE ${MICROHTTPD_LIBRARIES} ${CJSON_LIBRARIES}
        mbgui_common
        mbgui_hal
        lvgl
        Nagra_DPT
        ca_sec
        ca_cert
        nocs_csd
        nocs_crypto
        rt
    )

    target_link_options(mbtpm PRIVATE -static-libgcc -static-libstdc++)

    target_include_directories(mbtpm PRIVATE ${MICROHTTPD_INCLUDES} ${CJSON_INCLUDES})
    target_compile_definitions(mbtpm PRIVATE MBGUI_APP_TPM=1 MB_USE_MICROHTTPD)

    install(TARGETS mbtpm DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

endif()

if (NOT $ENV{USER} STREQUAL "century")
    target_compile_definitions(mbtpm PUBLIC MBMODO_NFS)
endif()
