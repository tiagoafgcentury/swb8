add_subdirectory(cak)
add_subdirectory(dpt)
add_subdirectory(dvl)
add_subdirectory(talts)

add_executable(mbcas
    mb_cas_main.cpp
    mb_ird_command.cpp
    mb_ird_command.h
    mb_nagra.h
    mb_nagra.cpp
    mb_pvr.cpp
    mb_pvr.h
    mb_acs_reencrypt.cpp
    mb_acs_reencrypt.h
    ../../common/mb_hash.cpp
    ../../common/mb_hash.h
    ../../common/mb_types.cpp
    ../../common/mb_types.h
    ../../tasks/mb_task.cpp
    ../../tasks/mb_task.h
    ../../tasks/mb_task_cas.cpp
    ../../tasks/mb_task_cas.h
    ../../tasks/private/mb_task_dump_params.cpp
    ../../tasks/private/mb_task_dump_params.h
    ../../tasks/private/mb_task_ipc_messages.cpp
    ../../tasks/private/mb_task_ipc_messages.h
    ../../tasks/private/mb_task_macros.h
    ../../tasks/private/mb_task_mqueue_helpers.cpp
    ../../tasks/private/mb_task_mqueue_helpers.h
)

strip_executable(mbcas)

string(TIMESTAMP PROJECT_COMPILE_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ")
set(PROJECT_VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/project_version.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../../project_version_seed.h ${PROJECT_VERSION_HEADER})

target_compile_definitions(mbcas PUBLIC
    MBGUI_APP_CAS=1
    CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}
    SKIP_PEER_VERIFICATION
    CENTURY_CHIP_FAMILY="${CMAKE_SYSTEM_VERSION}"
    $<$<BOOL:${MBGUI_USE_NAGRA_IRD_TESTS}>:MBCAS_NAGRA_IRD_TEST=1>
    MBCAS_CAK="$<IF:$<BOOL:${MBCAS_NAGRA_NAK}>,nak,tvd>"
)

target_include_directories(mbcas PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

target_link_libraries(mbcas PRIVATE
    mbversion
    Nagra::CAK
    $<$<BOOL:${MBCAS_NAGRA_NAK}>:Nagra::NAK>
    $<$<NOT:$<BOOL:${MBCAS_NAGRA_NAK}>>:Nagra::TVD>
    # Nagra::DVL
    Nagra_DPT
    nexguard_client
    ca_sec
    ca_cert
    nocs_crypto
    nocs_csd
    nv-dal
    mbgui_hal
    pthread
    rt
)

target_compile_options(mbcas PUBLIC
    -Wall -Wextra -fPIC -fstack-protector-all -funwind-tables
)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(mbcas PUBLIC
        -s -EL -mabi=32 -Wl,-z,relro -Wl,-z,now -march=mips32r2
    )
endif()

set_target_properties(mbcas PROPERTIES CXX_STANDARD 20)

install(TARGETS mbcas DESTINATION "bin")

set(MBGUI_BUILD_TALTS OFF CACHE BOOL "Cria teste TALTS de CAK Driver Verification da Nagra")
if(MBGUI_BUILD_TALTS)
    add_executable(mbtalts
        mb_talts_main.cpp
        mb_nagra.h
        mb_nagra.cpp
        mb_ird_command.cpp
        ../../../src/common/mb_types.cpp
    )

    install(TARGETS mbtalts DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

    target_link_libraries(mbtalts PRIVATE
        mbversion
        Nagra::TALTS
        Nagra_DPT
        nexguard_client
        ca_sec
        ca_cert
        nocs_crypto
        nocs_csd
        nv-dal
        mbgui_hal
        pthread
        rt
    )

    target_compile_options(mbtalts PUBLIC
        -Wall -Wextra -fPIC -fstack-protector-all -funwind-tables
    )

    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(mbtalts PUBLIC
            -s -EL -mabi=32 -Wl,-z,relro -Wl,-z,now -march=mips32r2
        )
    endif()

    set_target_properties(mbtalts PROPERTIES CXX_STANDARD 20)
endif()

## Extra Tools

add_executable(mbcasn_tool mb_casn.cpp)
strip_executable(mbcasn_tool)

target_include_directories(mbcasn_tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

target_link_libraries(mbcasn_tool PRIVATE
    Nagra_DPT
    nexguard_client
    ca_sec
    ca_cert
    nocs_crypto
    nocs_csd
)

set_target_properties(mbcasn_tool PROPERTIES CXX_STANDARD 20)
target_link_options(mbcasn_tool PRIVATE -Wl,--as-needed)

add_executable(mbnuid_tool mb_nuid.cpp)
strip_executable(mbcasn_tool)

target_include_directories(mbnuid_tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

target_link_libraries(mbnuid_tool PRIVATE
    Nagra_DPT
    nexguard_client
    ca_sec
    ca_cert
    nocs_crypto
    nocs_csd
)

set_target_properties(mbnuid_tool PROPERTIES CXX_STANDARD 20)
target_link_options(mbnuid_tool PRIVATE -Wl,--as-needed)
strip_executable(mbnuid_tool)

install(TARGETS mbcasn_tool mbnuid_tool DESTINATION ${CMAKE_INSTALL_PREFIX_TPM}/bin)
