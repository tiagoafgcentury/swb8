add_subdirectory(font_converter)

add_library(lv_fonts)
add_library(lv_fonts_min)

generate_font(seguisb.ttf 40 lv_font_segoeui_semi_bold_40_4bpp.c)
generate_font(seguisb.ttf 30 lv_font_segoeui_semi_bold_30_4bpp.c)
generate_font(seguisb.ttf 25 lv_font_segoeui_semi_bold_25_4bpp.c)
generate_font(seguisb.ttf 20 lv_font_segoeui_semi_bold_20_4bpp.c)
generate_font(segoeuib.ttf 40 lv_font_segoeui_bold_40_4bpp.c)
generate_font(segoeuib.ttf 30 lv_font_segoeui_bold_30_4bpp.c)
generate_font(segoeuib.ttf 25 lv_font_segoeui_bold_25_4bpp.c)
generate_font(segoeuib.ttf 20 lv_font_segoeui_bold_20_4bpp.c)
generate_font(segoeui.ttf 40 lv_font_segoeui_40_4bpp.c)
generate_font(segoeui.ttf 30 lv_font_segoeui_30_4bpp.c)
generate_font(segoeui.ttf 25 lv_font_segoeui_25_4bpp.c)
generate_font(segoeui.ttf 20 lv_font_segoeui_20_4bpp.c)
generate_font_extra(LiberationMono-Regular.ttf 34 lv_font_mono_34_4bpp.c)

target_sources(lv_fonts PRIVATE
    lv_font_segoeui_semi_bold_40_4bpp.c
    lv_font_segoeui_semi_bold_30_4bpp.c
    lv_font_segoeui_semi_bold_25_4bpp.c
    lv_font_segoeui_semi_bold_20_4bpp.c
    lv_font_segoeui_bold_40_4bpp.c
    lv_font_segoeui_bold_30_4bpp.c
    lv_font_segoeui_bold_25_4bpp.c
    lv_font_segoeui_bold_20_4bpp.c
    lv_font_segoeui_40_4bpp.c
    lv_font_segoeui_30_4bpp.c
    lv_font_segoeui_25_4bpp.c
    lv_font_segoeui_20_4bpp.c
    lv_font_mono_34_4bpp.c
)

target_sources(lv_fonts_min PRIVATE
    lv_font_segoeui_semi_bold_40_4bpp.c
    lv_font_segoeui_bold_25_4bpp.c
)

target_link_libraries(lv_fonts PUBLIC lvgl)
target_link_libraries(lv_fonts_min PUBLIC lvgl)
