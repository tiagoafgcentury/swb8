add_library(mbgui_lvgl STATIC)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/translate_field_1.inc
    COMMAND awk -F\t "/^[^#]/ { print $1 \",\" }" < ${CMAKE_CURRENT_SOURCE_DIR}/mb_osd_translate.tsv > ${CMAKE_CURRENT_BINARY_DIR}/translate_field_1.inc VERBATIM
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mb_osd_translate.tsv
    COMMENT "Gerando indice de traduções: Field 1"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/translate_field_2.inc
    COMMAND awk -F\t "/^[^#]/ { print $2 \",\" }" < ${CMAKE_CURRENT_SOURCE_DIR}/mb_osd_translate.tsv > ${CMAKE_CURRENT_BINARY_DIR}/translate_field_2.inc VERBATIM
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mb_osd_translate.tsv
    COMMENT "Gerando traduções em português: Field 2"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/translate_field_3.inc
    COMMAND awk -F\t "/^[^#]/ { print $3 \",\" }" < ${CMAKE_CURRENT_SOURCE_DIR}/mb_osd_translate.tsv > ${CMAKE_CURRENT_BINARY_DIR}/translate_field_3.inc VERBATIM
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mb_osd_translate.tsv
    COMMENT "Gerando traduções em inglês: Field 3"
)

target_sources(mbgui_lvgl PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/translate_field_1.inc
    ${CMAKE_CURRENT_BINARY_DIR}/translate_field_2.inc
    ${CMAKE_CURRENT_BINARY_DIR}/translate_field_3.inc
    mb_dfb_terminal_task.cpp
    mb_dfb_terminal_task.h
    mb_lnbf_detection.cpp
    mb_lnbf_detection.h
    mb_osd.cpp
    mb_osd.h
    mb_osd_activate.cpp
    mb_osd_activate.h
    mb_osd_audio_lr.cpp
    mb_osd_audio_lr.h
    mb_osd_auto_search_channel_list.cpp
    mb_osd_auto_search_channel_list.h
    mb_osd_auto_search_choose_satellite.cpp
    mb_osd_auto_search_choose_satellite.h
    mb_osd_box_info.cpp
    mb_osd_box_info.h
    mb_osd_breadcrumb.cpp
    mb_osd_breadcrumb.h
    mb_osd_ca_info.cpp
    mb_osd_ca_info.h
    mb_osd_ca_info.h
    mb_osd_change_channel.cpp
    mb_osd_change_channel.h
    mb_osd_channel_detail.cpp
    mb_osd_channel_detail.h
    mb_osd_channel_info.cpp
    mb_osd_channel_info.h
    mb_osd_channel_list.cpp
    mb_osd_channel_list.h
    mb_osd_channel_list_update.cpp
    mb_osd_channel_list_update.h
    mb_osd_choose_home_satellite.cpp
    mb_osd_choose_home_satellite.h
    mb_osd_clock.cpp
    mb_osd_clock.h
    mb_osd_closed_caption.cpp
    mb_osd_closed_caption.h
    mb_osd_confirm_default.cpp
    mb_osd_confirm_default.h
    mb_osd_confirm_delete_satellite.cpp
    mb_osd_confirm_delete_satellite.h
    mb_osd_confirm_save.cpp
    mb_osd_confirm_save.h
    mb_osd_delete_satellite.cpp
    mb_osd_delete_satellite.h
    mb_osd_diagnosis.cpp
    mb_osd_draw_closed_caption.cpp
    mb_osd_draw_closed_caption.h
    mb_osd_draw_subtitle.cpp
    mb_osd_draw_subtitle.h
    mb_osd_edit_channel.cpp
    mb_osd_edit_channel.h
    mb_osd_edit_delete_satellite.cpp
    mb_osd_edit_delete_satellite.h
    mb_osd_edit_satellite.cpp
    mb_osd_edit_satellite.h
    mb_osd_fade_canvas.cpp
    mb_osd_fade_canvas.h
    mb_osd_fast_install.cpp
    mb_osd_fast_install.h
    mb_osd_footer.cpp
    mb_osd_footer.h
    mb_osd_guide_channel.cpp
    mb_osd_guide_channel.h
    mb_osd_guide_channel_info.cpp
    mb_osd_guide_channel_info.h
    mb_osd_instala_facil.cpp
    mb_osd_instala_facil.h
    mb_osd_keyboard.cpp
    mb_osd_keyboard.h
    mb_osd_keys.h
    mb_osd_enhanced_keys.h
    mb_osd_lnbf_detection.cpp
    mb_osd_lnbf_detection.h
    mb_osd_main_menu.cpp
    mb_osd_main_menu.h
    mb_osd_media_player.cpp
    mb_osd_media_player.h
    mb_osd_menu_ajustes.cpp
    mb_osd_menu_ajustes.h
    mb_osd_menu_channel_list.cpp
    mb_osd_menu_channel_list.h
    mb_osd_menu_config.cpp
    mb_osd_menu_config.h
    mb_osd_menu_home.cpp
    mb_osd_menu_home.h
    mb_osd_menu_info.cpp
    mb_osd_menu_info.cpp
    mb_osd_menu_info.h
    mb_osd_menu_info.h
    mb_osd_menu_multimidia.cpp
    mb_osd_menu_multimidia.h
    mb_osd_menu_multimidia_video.cpp
    mb_osd_menu_multimidia_video.h
    mb_osd_menu_plus.cpp
    mb_osd_menu_plus.h
    mb_osd_menu_plus_record.cpp
    mb_osd_menu_plus_record.h
    mb_osd_menu_plus_sleep.cpp
    mb_osd_menu_plus_sleep.h
    mb_osd_menu_plus_timeshift.cpp
    mb_osd_menu_plus_timeshift.h
    mb_osd_menu_satellite.cpp
    mb_osd_menu_satellite.h
    mb_osd_manual_search.cpp
    mb_osd_manual_search.h
    mb_osd_menu_suporte.cpp
    mb_osd_menu_suporte.h
    mb_osd_menu_update.cpp
    mb_osd_menu_update.h
    mb_osd_menu_update_usb_satellite.cpp
    mb_osd_menu_update_usb_satellite.h
    mb_osd_message_box.cpp
    mb_osd_message_box.h
    mb_osd_password.cpp
    mb_osd_password.h
    mb_osd_photo_configuration.cpp
    mb_osd_photo_configuration.h
    mb_osd_photo_player.cpp
    mb_osd_photo_player.h
    mb_osd_popup_message.cpp
    mb_osd_popup_message.h
    mb_osd_production_info.cpp
    mb_osd_production_info.h
    mb_osd_sat_detect_lnbf.cpp
    mb_osd_sat_detect_lnbf.h
    mb_osd_scheduled_edit.cpp
    mb_osd_scheduled_edit.h
    mb_osd_scheduled_list.cpp
    mb_osd_scheduled_list.h
    mb_osd_select_satellite.cpp
    mb_osd_select_satellite.h
    mb_osd_select_switch.cpp
    mb_osd_select_switch.h
    mb_osd_service_table_progress.h
    mb_osd_set_clock.cpp
    mb_osd_set_clock.h
    mb_osd_set_timezone.cpp
    mb_osd_set_timezone.h
    mb_osd_sleep_timer.cpp
    mb_osd_sleep_timer.h
    mb_osd_software_update.cpp
    mb_osd_software_update.h
    mb_osd_software_update_finish.cpp
    mb_osd_software_update_finish.h
    mb_osd_software_update_forced.cpp
    mb_osd_software_update_forced.h
    mb_osd_terms_of_use.cpp
    mb_osd_terms_of_use.h
    mb_osd_translate.cpp
    mb_osd_translate.h
    mb_osd_volume.cpp
    mb_osd_volume.h
    mb_osd_waiting_signal.cpp
    mb_osd_waiting_signal.h
    mb_osd_welcome_banner.cpp
    mb_osd_welcome_banner.h
    mb_terms_of_use.cpp
    mb_terms_of_use.h
    mb_osd_radio_icon.cpp
    mb_osd_radio_icon.h
    mb_osd_change_password.cpp
    mb_osd_change_password.h
    mb_osd_parental_control.cpp
    mb_osd_parental_control.h
    mb_osd_parental_block_screen.cpp
    mb_osd_parental_block_screen.h
    mb_osd_usb_disk.cpp
    mb_osd_usb_disk.h
    mb_osd_subtitle_converter.h
    mb_osd_subtitle_configuration.cpp
    mb_osd_subtitle_configuration.h
    mb_osd_program_reminder.cpp
    mb_osd_program_reminder.h
    mb_osd_message_system_restart.h
    mb_osd_message_system_restart.cpp
    mb_osd_lnbf_snr.h
    mb_osd_lnbf_snr.cpp
)

target_link_libraries(mbgui_lvgl PUBLIC mbgui_common lvgl aui_mp fw_env)

target_include_directories(mbgui_lvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(mbgui_lvgl PUBLIC
    MBGUI_USE_LVGL
)

target_compile_options(mbgui_lvgl PRIVATE -ffast-math)
