# ExtenSer - An extensible, generic serialization library for C++
#
# Copyright (c) 2023 by Jackson Harmer
#
# SPDX-License-Identifier: BSD-3-Clause
# Distributed under The 3-Clause BSD License
# See accompanying file LICENSE or a copy at
# https://opensource.org/license/bsd-3-clause/

cmake_minimum_required(VERSION 3.18)

project(
        "ExtenSer Bitsery Adapter"
        VERSION 0.1.0
        DESCRIPTION "Bitsery adapter for the ExtenSer serialization library"
        LANGUAGES CXX)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif ()

FetchContent_Declare(bitsery
        GIT_REPOSITORY https://github.com/fraillt/bitsery.git
        GIT_TAG 9bebfd4911b8ef52c5a67754ccb87a8fd5223414 # 5.2.5
)
FetchContent_MakeAvailable(bitsery)

add_library(extenser-bitsery INTERFACE include/extenser_bitsery.hpp)
target_link_libraries(extenser-bitsery INTERFACE extenser)
target_link_libraries_system(extenser-bitsery INTERFACE extenser Bitsery::bitsery)
target_include_directories(extenser-bitsery INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()
