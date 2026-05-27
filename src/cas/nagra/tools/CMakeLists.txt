cmake_minimum_required(VERSION 3.22)

project(NagraTools
    VERSION 1.0
    DESCRIPTION "Nagra Tools"
    HOMEPAGE_URL "https://centurybr.com.br"
    LANGUAGES C CXX)

add_executable(pk_split pk_split.cpp)

set_target_properties(pk_split PROPERTIES CXX_STANDARD 23)

add_executable(pk_casn_read pk_casn_read.cpp)

set_target_properties(pk_casn_read PROPERTIES CXX_STANDARD 23)

install(TARGETS pk_split pk_casn_read)
