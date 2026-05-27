add_library(sqlite3 sqlite3.c)

target_link_libraries(sqlite3 INTERFACE pthread dl)

set(SQLITE_CLI OFF CACHE BOOL "Generate CLI for sqlite")
if (SQLITE_CLI)
    add_executable(sqlite shell.c)
    target_link_libraries(sqlite PRIVATE sqlite3)
endif()

target_include_directories(sqlite3
                        PUBLIC
                        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/.."
                        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>/.."
)
