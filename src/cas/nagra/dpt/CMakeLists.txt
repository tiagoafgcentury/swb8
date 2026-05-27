add_library(Nagra_DPT STATIC
    ca_dpt.c
)

target_include_directories(Nagra_DPT PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(Nagra_DPT PUBLIC
    NASC30
    USE_SEC
)
