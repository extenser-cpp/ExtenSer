# ExtenSer - An extensible, generic serialization library for C++
#
# Copyright (c) 2023 by Jackson Harmer
#
# SPDX-License-Identifier: BSD-3-Clause
# Distributed under The 3-Clause BSD License
# See accompanying file LICENSE or a copy at
# https://opensource.org/license/bsd-3-clause/

FetchContent_Declare(bitsery
        GIT_REPOSITORY https://github.com/fraillt/bitsery.git
        GIT_TAG d1a47e06e2104b195a19c73b61f1d5c1dceaa228
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
