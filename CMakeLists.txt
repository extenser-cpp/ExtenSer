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
endif ()

project(
        "ExtenSer"
        VERSION 0.1.0
        DESCRIPTION "Extensible, generic serialization library for C++"
        LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTING "Build the test tree" ON)
option(CODE_COVERAGE "Build code w/ coverage enabled" OFF)
option(USE_MAGIC_ENUM "Build w/ magic_enum static reflection support for enum serialization" OFF)
option(WERROR "Treat all warnings as errors" ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if (CODE_COVERAGE)
  include(CodeCoverage)
  append_coverage_compiler_flags()
endif ()

# ==== Compiler/Machine Info ====

if(WIN32)
  set(TARGET_WINDOWS TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_MSVC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_MINGW TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU")
      set(CXX_CLANG TRUE)
    else()
      set(CXX_CLANG_CL TRUE)
      add_compile_options(/EHsc)
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CXX_INTEL TRUE)
  else()
    message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for Windows are: [MSVC, GNU, Clang, Intel]")
  endif()
elseif(UNIX AND NOT APPLE)
  set(TARGET_LINUX TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CXX_INTEL TRUE)
  else()
    message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for UNIX are: [GNU, Clang, Intel]")
  endif()
elseif(APPLE)
  set(TARGET_APPLE TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CXX_INTEL TRUE)
  else()
    message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for macOS are: [GNU, Clang, Intel]")
  endif()
endif()

# ==== Compiler/Linker Flags ====

if(CXX_MSVC)
  set(FULL_WARNING
      /W4
      /experimental:external
      /external:W0
      /external:anglebrackets
      /permissive-
      /wd4619
      /w14242
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289
      /w14296
      /w14311
      /w14545
      /w14546
      /w14547
      /w14549
      /w14555
      /w14640
      /w14826
      /w14905
      /w14906
      /w14928
      /we4834
      /Zc:__cplusplus
      /bigobj
      -D_WIN32_WINNT=0x0A00)
  set(MIN_WARNING
      /W1
      /experimental:external
      /external:W0
      /external:anglebrackets
      /wd4619
      /Zc:__cplusplus
      /bigobj
      -D_WIN32_WINNT=0x0A00)
  if(WERROR)
    list(APPEND FULL_WARNING /WX)
    list(APPEND MIN_WARNING /WX)
  endif()
elseif(CXX_GCC OR CXX_MINGW OR CXX_INTEL)
  set(FULL_WARNING
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Weffc++
      -Wuseless-cast
      -Wlogical-op
      -Wduplicated-branches
      -Wduplicated-cond
      -Wmisleading-indentation
      -Wmissing-field-initializers)
  set(MIN_WARNING -Wall)
  if(CXX_MINGW)
    list(APPEND FULL_WARNING -Wa,-mbig-obj)
    list(APPEND MIN_WARNING -Wa,-mbig-obj)
    set(USE_PCH OFF)
  endif()
  if(WERROR)
    list(APPEND FULL_WARNING -Werror)
    list(APPEND MIN_WARNING -Werror)
  endif()
elseif(CXX_CLANG OR CXX_CLANG_CL)
  set(FULL_WARNING
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-c++98-c++11-compat
      -Wno-c++98-c++11-compat-binary-literal
      -Wno-c++98-c++11-c++14-compat
      -Wno-padded
      -Wno-exit-time-destructors
      -Wno-missing-prototypes
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-weak-vtables
      -Wno-reserved-id-macro
      -Wno-missing-braces
      -Wno-covered-switch-default
      -Wno-unused-template
      -Wno-assume)
  set(MIN_WARNING -Wall)
  if(CXX_CLANG_CL)
    list(APPEND FULL_WARNING
        /bigobj
        -D_WIN32_WINNT=0x0A00)
    list(APPEND MIN_WARNING
        /bigobj
        -D_WIN32_WINNT=0x0A00)
  endif()
  if(WERROR)
    list(APPEND FULL_WARNING -Werror)
    list(APPEND MIN_WARNING -Werror)
  endif()
endif()

# ==== Target(s) ====

add_library(extenser INTERFACE include/extenser.hpp)
add_library(ExtenSer::extenser ALIAS extenser)

if (CXX_MSVC AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(extenser INTERFACE "/Zc:__cplusplus")
endif ()

include(GNUInstallDirs)
target_include_directories(
        extenser INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

include(FetchContent)
include(system_link)

FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d
        )
FetchContent_MakeAvailable(nlohmann_json)

add_library(extenser-json INTERFACE adapters/json/extenser_json.hpp)
target_link_libraries_system(extenser-json INTERFACE extenser nlohmann_json::nlohmann_json)

if (USE_MAGIC_ENUM OR BUILD_TESTING)
    FetchContent_Declare(magic_enum
            GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
            GIT_TAG e1a68e9dd3d2e9180b04c8aeacd4975db745e6b8
            )
    FetchContent_MakeAvailable(magic_enum)

    if (USE_MAGIC_ENUM)
        target_link_libraries_system(extenser-json INTERFACE magic_enum::magic_enum)
        target_compile_definitions(extenser-json INTERFACE EXTENSER_USE_MAGIC_ENUM)
    endif ()
endif ()

target_include_directories(extenser-json INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/adapters>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

FetchContent_Declare(bitsery
        GIT_REPOSITORY https://github.com/fraillt/bitsery.git
        GIT_TAG d1a47e06e2104b195a19c73b61f1d5c1dceaa228
        )
FetchContent_MakeAvailable(bitsery)

add_library(extenser-bitsery INTERFACE adapters/bitsery/extenser_bitsery.hpp)
target_link_libraries_system(extenser-bitsery INTERFACE extenser Bitsery::bitsery)
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
