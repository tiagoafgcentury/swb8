cmake_minimum_required(VERSION 3.22)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31")
	cmake_policy(SET CMP0177 NEW)
endif()

project(mbgui
    #VERSION 0.3.12.2

    # VERSION 1.99.18.1

     VERSION 1.3.5.2

    #VERSION 77.78.0.1
    DESCRIPTION "MidiaBox GUI"
    HOMEPAGE_URL "https://centurybr.com.br"
    LANGUAGES C CXX)

# Code sanitizers
#set(CMAKE_CXX_CPPCHECK "cppcheck;--check-level=exhaustive")
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
#set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "clang-include-fixer")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(MBGUI_NET_HLS OFF CACHE BOOL "Habilita Streaming IP/HLS")
set(MBGUI_SAT_MONITOR OFF CACHE BOOL "Habilita SAT Monitor via PostgreSQL")
set(MBGUI_MBTEST OFF CACHE BOOL "Habilita MB Test")
set(MBGUI_USE_RLOTTIE ON CACHE BOOL "Habilita rlottie no LVGL")
set(MBGUI_USE_PGO_GEN OFF CACHE BOOL "Habilita geração de dados para PGO")
set(MBGUI_USE_PGO_USE OFF CACHE BOOL "Usa dados de PGO na compilação")
set(MBGUI_ANIMATION ON CACHE BOOL "Habilita animações")
set(MBGUI_USE_ASAN OFF CACHE BOOL "Habilita Address-Sanitizer para debug de memória")
set(MBGUI_USE_UBSAN OFF CACHE BOOL "Habilita Undefined Behaviour-Sanitizer para debug de códigos inválidos")
set(MBGUI_USE_NAGRA_IRD_TESTS OFF CACHE BOOL "Habilita testes de comandos Ird no Nagra")
set(MBGUI_USE_CHANNEL_RANKING OFF CACHE BOOL "Habilita ranking de canais")
set(MBGUI_USE_EXTRA_DEBUGGING OFF CACHE BOOL "Habilita debug completo")
set(MBGUI_FORCED_UPDATE OFF CACHE BOOL "Força o update no inicio da applicação")
set(MBGUI_USE_JPEG OFF CACHE BOOL "Habilita libturbojpeg")
set(SPLIT_DEBUG ON CACHE BOOL "Strip exe and put debug symbols in seperate file")
set(DEBUG_USER "root" CACHE STRING "User name for debugging processes")
set(MBCAS_NAGRA_NAK OFF CACHE BOOL "Habilita CAK para NAK")
set(MBCAS_USE_DIR_EXT OFF CACHE BOOL "Usa diretórios mnt_app.nak e mnt_app.tvd")
set(MBGUI_PERIODIC_DUMP OFF CACHE BOOL "Mostra no log periodicamente o status de vários tasks")
set(MBD_DEFAULT_LOG_LEVEL "LOG_LEVEL_INFO" CACHE STRING "Default LOG Level")

# Default values for LVGL
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "")
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "")
set(LOTTIE_MODULE OFF CACHE BOOL "")
set(LOTTIE_THREAD OFF CACHE BOOL "")

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ip_sat_monitor.txt)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/ip_sat_monitor.txt "192.168.30.7")
endif()
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/ip_sat_monitor.txt IP_SAT_MONITOR)

function(strip_executable TARGET)
    if (SPLIT_DEBUG)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_custom_command(TARGET ${TARGET} POST_BUILD
                                COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:${TARGET}> $<TARGET_FILE:${TARGET}>.debug
                                COMMENT "Objcopy $<TARGET_FILE:${TARGET}>.debug" )
            add_custom_command(TARGET ${TARGET} POST_BUILD
                                COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:${TARGET}> $<TARGET_FILE:${TARGET}>.nostrip
                                COMMENT "Rename ${TARGET}" )
            add_custom_command(TARGET ${TARGET} POST_BUILD
                                COMMAND ${CMAKE_STRIP} ${STRIP_ARGS} --strip-all -o $<TARGET_FILE:${TARGET}> $<TARGET_FILE:${TARGET}>.nostrip
                                COMMENT "Stripping ${TARGET}" )
            add_custom_command(TARGET ${TARGET} POST_BUILD
                                COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink="$<TARGET_FILE:${TARGET}>.debug" $<TARGET_FILE:${TARGET}>
                                COMMENT "Set debug info $<TARGET_FILE:${TARGET}>.debug" )
        endif()
    endif()
endfunction()

set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

find_package(PkgConfig)

add_executable(${PROJECT_NAME})
strip_executable(${PROJECT_NAME})

if(MBGUI_SAT_MONITOR)
    add_executable(sat_monitor)
    strip_executable(sat_monitor)
endif()

set(GIT_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/git_version.cpp)

add_custom_command(OUTPUT ${GIT_VERSION_FILE}
                    COMMENT "Gerando versão do GIT"
                    COMMAND git show --abbrev-commit --oneline | head -n1 | cut -c 1-8 | sed -e 's/^/auto GIT_VERSION = \"/' -e 's/\\s*$$/\"\;/'  > ${GIT_VERSION_FILE}
                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/.git/logs/HEAD
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

string(TIMESTAMP PROJECT_COMPILE_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ")
set(PROJECT_VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/project_version.h)
math(EXPR PROJECT_VERSION_HEX "(${PROJECT_VERSION_MAJOR} << 8) + (${PROJECT_VERSION_MINOR})" OUTPUT_FORMAT HEXADECIMAL)
string(SUBSTRING ${PROJECT_VERSION_HEX} 2 -1 PROJECT_VERSION_HEX)
string(TOUPPER ${PROJECT_VERSION_HEX} PROJECT_VERSION_HEX)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project_version_seed.h ${PROJECT_VERSION_HEADER})

add_library(mbversion STATIC
    ${GIT_VERSION_FILE}
)

if(MBGUI_NET_HLS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MBGUI_NET_HLS)
endif()

add_subdirectory(extern)
add_subdirectory(extra)
add_subdirectory(src)
add_subdirectory(ui)

# Get the Operator from the library's name.  Must be the first lib not named CAK
if (MBCAS_NAGRA_NAK)
    set(NAGRA_OPERATOR "NAK")
else()
    set(NAGRA_OPERATOR "TVD")
endif()

# This uses the NAGRA_OPERATOR above to set the file name
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/generate_release_seed.sh ${CMAKE_CURRENT_BINARY_DIR}/generate_release.sh)

if(MBGUI_USE_RLOTTIE)
    install(TARGETS rlottie DESTINATION "lib")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")
install(TARGETS sqlite3 lv_fonts DESTINATION "lib")
install(FILES
            res/logo_5_27x27.png
            res/logo_ajustes.png
            res/logo_ajustes_selecao.png
            res/logo_arrow_left_20x20.png
            res/logo_arrow_left_sel_20x20.png
            res/logo_arrow_rigth_20x20.png
            res/logo_arrow_rigth_sel_20x20.png
            res/logo_audio_lr_34x34.png
            res/logo_canais_de_radio.png
            res/logo_canais_de_radio_selecao.png
            res/logo_canais_de_tv.png
            res/logo_canais_de_tv_selecao.png
            res/logo_cc_34x34.png
            res/logo_century_branco.png
            res/logo_century_c_cinza.png
            res/logo_century_cinza_400x404.png
            res/logo_email_25x19.png
            res/logo_favoritos_branco_17x15.png
            res/logo_favoritos_branco_27x25.png
            res/logo_favoritos_cinza_17x15.png
            res/logo_info_34x34.png
            res/logo_inicio.png
            res/logo_inicio_selecao.png
            res/logo_link_25x25.png
            res/logo_mais_27x27.png
            res/logo_mais_34x34.png
            res/logo_midiabox_b8_150x22.png
            res/logo_midiabox_b8_208x30.png
            res/logo_midiabox_b8_225x33.png
            res/logo_midiabox_b8_256x37.png
            res/logo_midiabox_b8_518x75.png
            res/logo_multimidia.png
            res/logo_multimidia_selecao.png
            res/logo_ok_27x27.png
            res/logo_pc_10_27x28.png
            res/logo_pc_12_27x28.png
            res/logo_pc_14_27x28.png
            res/logo_pc_16_27x28.png
            res/logo_pc_18_27x28.png
            res/logo_pc_l_27x28.png
            res/logo_voltar_27x27.png
            res/logo_volume_off_25x25.png
            res/logo_volume_off_circ_25x25.png
            res/logo_volume_off_sel_25x25.png
            res/logo_cc_27x27.png
            res/logo_mais_27x27.png
            res/logo_voltar_27x27.png
            res/logo_ok_27x27.png
            res/logo_info_27x27.png
            res/logo_5_27x27.png
            res/logo_mais_34x34.png
            res/logo_info_34x34.png
            res/logo_audio_lr_34x34.png
            res/logo_audio_l_27x27.png
            res/logo_audio_r_27x27.png
            res/logo_mover_cima_27x27.png
            res/logo_mover_baixo_27x27.png
            res/logo_cc_34x34.png
            res/logo_arrow_left_20x20.png
            res/logo_arrow_left_sel_20x20.png
            res/logo_arrow_rigth_20x20.png
            res/logo_arrow_rigth_sel_20x20.png
            res/logo_arrow_down_27x27.png
            res/logo_link_25x25.png
            res/logo_volume_on_25x25.png
            res/logo_whastapp_25x25.png
            res/logo_sathd_212x82.png
            res/logo_mp_play_34x34.png
            res/logo_mp_play_sel_34x34.png
            res/logo_mp_pause_34x34.png
            res/logo_mp_pause_sel_34x34.png
            res/logo_mp_next_34x34.png
            res/logo_mp_next_sel_34x34.png
            res/logo_mp_previous_34x34.png
            res/logo_mp_previous_sel_34x34.png
            res/logo_mp_stop_34x34.png
            res/logo_mp_stop_sel_34x34.png
            res/logo_mp_forward_34x34.png
            res/logo_mp_forward_sel_34x34.png
            res/logo_mp_rewind_34x34.png
            res/logo_mp_rewind_sel_34x34.png
            res/logo_mp_speed_2x_34x34.png
            res/logo_mp_speed_4x_34x34.png
            res/logo_mp_speed_8x_34x34.png
            res/logo_mp_speed_16x_34x34.png
            res/logo_mp_speed_24x_34x34.png
            res/logo_file_usb_68x80.png
            res/logo_file_video_68x80.png
            res/logo_file_video_selected_68x80.png
            res/logo_file_music_69x80.png
            res/logo_file_music_selected_69x80.png
            res/logo_file_photo_68x80.png
            res/logo_file_photo_selected_68x80.png
            res/logo_file_recording_67x80.png
            res/logo_file_recording_selected_67x80.png
            res/helio.png
            res/fran.png
            res/sky_brasil_logo_125x50.png
            res/anim/alarm_bell.json
            res/anim/blocked.json
            res/anim/cc.json
            res/anim/channel_list.json
            res/anim/music_icon.json
            res/anim/ok.json
            res/anim/satellite_signal.json
            res/anim/sleep.json
            res/anim/soundwave.json
            res/anim/support.json
            res/anim/usb.json
            res/anim/video_rec.json
        DESTINATION "res")

if(MBGUI_SAT_MONITOR)
    install(TARGETS sat_monitor DESTINATION "bin")
endif()

if(MBGUI_MBTEST)
    add_executable(mbtest mb_test.cpp)
    install(TARGETS mbtest)
    target_link_libraries(mbtest PRIVATE
        pthread
    )
endif()

function(gdb_for_all_targets DIR)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ip_board.txt)
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/ip_board.txt "0.0.0.0")
    endif()
    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/ip_board.txt IP_BOARD)
    get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TARGET IN LISTS TGTS)
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        if("${TARGET_TYPE}" STREQUAL "EXECUTABLE")
            message(STATUS "Generate GDB Files for ${TARGET}: ${CMAKE_CURRENT_BINARY_DIR}/gdb-local-${TARGET}.sh ${CMAKE_CURRENT_BINARY_DIR}/gdb-remote-${TARGET}.txt")
            get_target_property(TARGET_FULL_PATH ${TARGET} BINARY_DIR)
            string(APPEND TARGET_FULL_PATH "/${TARGET}")
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_shell_script_telnet_seed.sh ${CMAKE_CURRENT_BINARY_DIR}/run_shell_script_telnet_${TARGET}.sh)
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_shell_script_serial_seed.sh ${CMAKE_CURRENT_BINARY_DIR}/run_shell_script_serial_${TARGET}.sh)
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_gdb_script_seed.txt ${CMAKE_CURRENT_BINARY_DIR}/run_gdb_script_${TARGET}.txt)

            # Do not remove frame-pointers in debug - Allows GDB to have full stack
            target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Debug>:-fno-omit-frame-pointer -fno-common>)

            # ASAN - https://wiki.gentoo.org/wiki/AddressSanitizer
            if(MBGUI_USE_ASAN)
                if (${TARGET} STREQUAL mbcas)
                    message(STATUS "Skip ASAN for mbcas")
                else()
                    message(STATUS "Add ASAN for ${TARGET}")
                    target_compile_options(${TARGET} PRIVATE -fsanitize=address -fno-omit-frame-pointer -fno-common)
                    target_link_options(${TARGET} PRIVATE -fsanitize=address)
                    target_compile_definitions(${TARGET} PRIVATE MBGUI_USE_ASAN=1)
                endif()
            endif()

            # UBASN - https://wiki.gentoo.org/wiki/UndefinedBehaviorSanitizer
            if(MBGUI_USE_UBSAN)
                message(STATUS "Add UBSAN for ${TARGET}")
                target_compile_options(${TARGET} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
                target_link_options(${TARGET} PRIVATE -fsanitize=undefined)
                target_compile_definitions(${TARGET} PRIVATE MBGUI_USE_UBSN=1)
            endif()
        endif()
    endforeach()

    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
    foreach(SUBDIR IN LISTS SUBDIRS)
        gdb_for_all_targets("${SUBDIR}")
    endforeach()
endfunction()

gdb_for_all_targets(.)

set(MBGUI_TARGETS mbgui mbcas sqlite3 mbgui_common mbgui_dvb mbgui_hal mbgui_lvgl mbdlgui lvgl lv_fonts fw_env)
if (TARGET mbtpm)
    list(APPEND MBGUI_TARGETS mbtpm)
endif()
if (TARGET mbmigrate)
    list(APPEND MBGUI_TARGETS mbmigrate)
endif()
if (TARGET rlottie)
    list(APPEND MBGUI_TARGETS rlottie)
endif()
if(TARGET sat_monitor)
    list(APPEND MBGUI_TARGETS sat_monitor)
endif()

foreach(TARGET ${MBGUI_TARGETS})
    message(STATUS "Set target: ${TARGET}")
    target_compile_options(${TARGET} PRIVATE $<$<NOT:$<CONFIG:Debug>>:-fstack-protector-strong -ffunction-sections -fdata-sections -flto=auto> $<$<CONFIG:Debug>:-ggdb3>)
    target_link_options(${TARGET} PRIVATE -Wl,--as-needed $<$<NOT:$<CONFIG:Debug>>:-fstack-protector-strong -Wl,-z,relro -Wl,-z,now -Wl,--gc-sections -flto=auto>)
    set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 20)
    target_compile_definitions(${TARGET} PUBLIC
        $<$<BOOL:${MBGUI_USE_CHANNEL_RANKING}>:MBGUI_USE_CHANNEL_RANKING>
        $<$<BOOL:${MBGUI_USE_EXTRA_DEBUGGING}>:MBGUI_USE_EXTRA_DEBUGGING>
        $<$<BOOL:${MBGUI_ANIMATION}>:MBGUI_ANIMATION>
        $<$<BOOL:${MBGUI_FORCED_UPDATE}>:MBGUI_FORCED_UPDATE>
        $<$<BOOL:${MBCAS_USE_DIR_EXT}>:MBCAS_USE_DIR_EXT>
        $<$<BOOL:${MBGUI_USE_JPEG}>:MBGUI_USE_JPEG>
        $<$<BOOL:${MBGUI_USE_RLOTTIE}>:MBGUI_USE_RLOTTIE>
        $<$<BOOL:${MBGUI_PERIODIC_DUMP}>:MBGUI_PERIODIC_DUMP>
        MBD_DEFAULT_LOG_LEVEL=${MBD_DEFAULT_LOG_LEVEL}
    )

    if(MBGUI_USE_PGO_GEN)
        target_compile_options(${TARGET} PUBLIC --coverage -fprofile-generate=/var/pgo -fprofile-correction -fearly-inlining)
        target_link_options(${TARGET} PUBLIC --coverage -fprofile-generate=/var/pgo -fprofile-correction -fearly-inlining)
    endif()

    if(MBGUI_USE_PGO_USE)
        target_compile_options(${TARGET} PUBLIC -fprofile-use=/opt/ali/buildroot-10.2.1.29/output/target/var/pgo)
        target_link_options(${TARGET} PUBLIC -fprofile-use=/opt/ali/buildroot-10.2.1.29/output/target/var/pgo)
    endif()
endforeach()
