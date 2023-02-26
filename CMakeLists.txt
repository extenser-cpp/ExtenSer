# BSD 3-Clause License
#
# Copyright (c) 2020-2023, Jackson Harmer All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# 1. Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# 1. Neither the name of the copyright holder nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.12)

if (CMAKE_GENERATOR MATCHES "Ninja")
    file(
        WRITE "${CMAKE_BINARY_DIR}/GNUMakeRulesOverwrite.cmake" 
            "STRING(REPLACE \"-MD\" \"-MMD\" CMAKE_DEPFILE_FLAGS_C \"\${CMAKE_DEPFILE_FLAGS_C}\")\n"
            "STRING(REPLACE \"-MD\" \"-MMD\" CMAKE_DEPFILE_FLAGS_CXX \"\${CMAKE_DEPFILE_FLAGS_CXX}\")\n"
    )
    set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_BINARY_DIR}/GNUMakeRulesOverwrite.cmake" CACHE INTERNAL "")
endif()

project(
    "ExtenSer"
    VERSION 0.1.0
    DESCRIPTION "Extensible, generic serialization library for C++"
    LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTING "Build the test tree" ON)

# ==== Target(s) ====

add_library(extenser INTERFACE include/extenser.hpp)
add_library(ExtenSer::extenser ALIAS extenser)

include(GNUInstallDirs)
target_include_directories(
    extenser INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d
    )
FetchContent_MakeAvailable(nlohmann_json)

add_library(extenser-json INTERFACE adapters/json/extenser_json.hpp)
target_link_libraries(extenser-json INTERFACE extenser nlohmann_json::nlohmann_json)
target_include_directories(extenser-json INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/adapters>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

FetchContent_Declare(bitsery
    GIT_REPOSITORY https://github.com/fraillt/bitsery.git
    GIT_TAG d1a47e06e2104b195a19c73b61f1d5c1dceaa228
    )
FetchContent_MakeAvailable(bitsery)

add_library(extenser-bitsery INTERFACE adapters/bitsery/extenser_bitsery.hpp)
target_link_libraries(extenser-bitsery INTERFACE extenser Bitsery::bitsery)
target_include_directories(extenser-bitsery INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/adapters>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ==== Target Installation ====

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ExtenSerConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(
    TARGETS extenser
    EXPORT extenser-targets
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
    EXPORT extenser-targets
    FILE "ExtenSerConfig.cmake"
    NAMESPACE ExtenSer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/extenser)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ExtenSerConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/extenser)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()
