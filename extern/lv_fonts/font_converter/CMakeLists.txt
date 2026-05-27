execute_process(COMMAND docker build --tag font_converter .
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

#docker run -v $PWD:/data -w /data -it font_converter lv_font_conv --size 25 -o lv_font_segoeui_semi_bold_25_4bpp.c --bpp 4 --format lvgl --font seguisb.ttf -r 0x20-0x7f,0xa0-0xff
function(generate_font file_name size result_name)

    add_custom_command(OUTPUT ${result_name}
                        COMMAND docker run --rm -v "${CMAKE_CURRENT_SOURCE_DIR}:/src" -v "${CMAKE_CURRENT_BINARY_DIR}:/dst" -w /src font_converter lv_font_conv --size ${size} -o "/dst/${result_name}" --bpp 4 --format lvgl --no-compress --font "${file_name}" -r 0x20-0x7f,0xa0-0xff
                        MAIN_DEPENDENCY ${file_name}
                        COMMENT "Gerando fonte: ${file_name} ${size}"
                        )
endfunction()

function(generate_font_extra file_name size result_name)

    add_custom_command(OUTPUT ${result_name}
                        COMMAND docker run --rm -v "${CMAKE_CURRENT_SOURCE_DIR}:/src" -v "${CMAKE_CURRENT_BINARY_DIR}:/dst" -w /src font_converter lv_font_conv --size ${size} -o "/dst/${result_name}" --bpp 4 --format lvgl --no-compress --font "${file_name}" -r 0x20-0x7f,0xa0-0xff --symbols ®°½¿™¢£♪©¦~ß¥¤Åå
                        MAIN_DEPENDENCY ${file_name}
                        COMMENT "Gerando fonte: ${file_name} ${size}"
                        )
endfunction()

