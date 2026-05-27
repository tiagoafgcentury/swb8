add_library(fw_env STATIC
    crc32.c
    fw_env.c
    fw_env_ubi.c
)

target_include_directories(fw_env PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(fw_env PUBLIC ALI_AS_ENABLE SUPPORT_ALI_HMAC CONFIG_ENV_AES)
