################################################################################
# Copyright (c) 2019-2021 Vladislav Trifochkin
#
# This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
#
# Changelog:
#      2021.09.10 Initial version.
################################################################################
cmake_minimum_required (VERSION 3.5)
project(io-lib CXX C)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(pfs::io ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${PROJECT_NAME} INTERFACE pfs::common)
target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    target_compile_definitions(${PROJECT_NAME} INTERFACE "-D_POSIX_C_SOURCE=1")
endif()
