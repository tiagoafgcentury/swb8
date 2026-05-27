project(extra)

add_executable(crc16 crc16.cpp)

add_executable(hex_transform hex_transform.cpp)

set_target_properties(hex_transform PROPERTIES CXX_STANDARD 20)

add_executable(fnvsum fnvsum.cpp ../src/common/mb_hash.cpp)

target_include_directories(fnvsum PRIVATE ../src)

set_target_properties(fnvsum PROPERTIES CXX_STANDARD 20)
