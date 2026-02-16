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
        "ExtenSer"
        VERSION 0.1.1
        DESCRIPTION "Extensible, generic serialization library for C++"
        LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_BITSERY "Build the optional bitsery serial adapter" OFF)
option(USE_MAGIC_ENUM "Build w/ magic_enum static reflection support for enum serialization" OFF)
option(WERROR "Treat all warnings as errors" ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# ==== Compiler/Machine Info ====

if (WIN32)
    set(TARGET_WINDOWS TRUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CXX_MSVC TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CXX_MINGW TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU")
            set(CXX_CLANG TRUE)
        else ()
            set(CXX_CLANG_CL TRUE)
            add_compile_options(/EHsc)
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CXX_INTEL TRUE)
    else ()
        message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for Windows are: [MSVC, GNU, Clang, Intel]")
    endif ()
elseif (UNIX AND NOT APPLE)
    set(TARGET_LINUX TRUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CXX_GCC TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CXX_CLANG TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CXX_INTEL TRUE)
    else ()
        message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for UNIX are: [GNU, Clang, Intel]")
    endif ()
elseif (APPLE)
    set(TARGET_APPLE TRUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CXX_GCC TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CXX_CLANG TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CXX_INTEL TRUE)
    else ()
        message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for macOS are: [GNU, Clang, Intel]")
    endif ()
endif ()

# ==== Build Types ====

include(CheckCXXCompilerFlag)

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
set(ALLOWED_BUILD_TYPES Debug Release MinSizeRel RelWithDebInfo)

if (CXX_MSVC OR CXX_CLANG_CL)
    check_cxx_compiler_flag("/fsanitize=address" SUPPORTS_ASAN)
    check_cxx_compiler_flag("/fsanitize=thread" SUPPORTS_TSAN)
    if (CXX_CLANG_CL)
        list(APPEND ALLOWED_BUILD_TYPES Coverage)
    endif ()
else ()
    list(APPEND ALLOWED_BUILD_TYPES Coverage)
    set(TMP_REQ_LIB ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address;asan")
    check_cxx_compiler_flag("-fsanitize=address" SUPPORTS_ASAN)
    set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=thread;tsan")
    check_cxx_compiler_flag("-fsanitize=thread" SUPPORTS_TSAN)
    set(CMAKE_REQUIRED_LIBRARIES ${TMP_REQ_LIB})
endif ()

if (SUPPORTS_ASAN)
    list(APPEND ALLOWED_BUILD_TYPES "ASAN")
endif ()

if (SUPPORTS_TSAN)
    list(APPEND ALLOWED_BUILD_TYPES "TSAN")
endif ()

if (IS_MULTI_CONFIG)
    foreach (BTYPE IN LISTS ALLOWED_BUILD_TYPES)
        if (NOT ${BTYPE} IN_LIST CMAKE_CONFIGURATION_TYPES)
            list(APPEND CMAKE_CONFIGURATION_TYPES ${BTYPE})
        endif ()
    endforeach ()
else ()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${ALLOWED_BUILD_TYPES}")
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    elseif (NOT CMAKE_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
        message(FATAL_ERROR "Unknown BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
    endif ()
endif ()

if (CXX_CLANG OR CXX_GCC OR CXX_INTEL)
    set(GCC_COVERAGE_FLAGS "--coverage")
    set(GCC_ASAN_FLAGS "-fsanitize=address,leak,undefined -fno-omit-frame-pointer")
    set(GCC_TSAN_FLAGS "-fsanitize=thread,undefined")

    set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")
    set(CMAKE_STATIC_LINKER_FLAGS_COVERAGE "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}" CACHE STRING "")
    set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "")

    set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")
    set(CMAKE_EXE_LINKER_FLAGS_ASAN "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")
    set(CMAKE_SHARED_LINKER_FLAGS_ASAN "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")
    set(CMAKE_STATIC_LINKER_FLAGS_ASAN "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "")
    set(CMAKE_MODULE_LINKER_FLAGS_ASAN "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_ASAN_FLAGS}" CACHE STRING "")

    set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
    set(CMAKE_EXE_LINKER_FLAGS_TSAN "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
    set(CMAKE_SHARED_LINKER_FLAGS_TSAN "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
    set(CMAKE_STATIC_LINKER_FLAGS_TSAN "${CMAKE_STATIC_LINKER_FLAGS}" CACHE STRING "")
    set(CMAKE_MODULE_LINKER_FLAGS_TSAN "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${GCC_TSAN_FLAGS}" CACHE STRING "")
elseif (CXX_MSVC OR CXX_CLANG_CL)
    if (CXX_CLANG_CL)
        set(CLANG_COVERAGE_FLAGS "-fprofile-instr-generate -fcoverage-mapping")

        set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} ${CLANG_COVERAGE_FLAGS}" CACHE STRING "")
        set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${CLANG_COVERAGE_FLAGS}" CACHE STRING "")
        set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${CLANG_COVERAGE_FLAGS}" CACHE STRING "")
        set(CMAKE_STATIC_LINKER_FLAGS_COVERAGE "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}" CACHE STRING "")
        set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${CLANG_COVERAGE_FLAGS}" CACHE STRING "")
    endif ()

    set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /fsanitize=address /Oy-" CACHE STRING "")
    set(CMAKE_EXE_LINKER_FLAGS_ASAN ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
    set(CMAKE_SHARED_LINKER_FLAGS_ASAN ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
    set(CMAKE_STATIC_LINKER_FLAGS_ASAN ${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
    set(CMAKE_MODULE_LINKER_FLAGS_ASAN ${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "")
endif ()

# ==== Compiler/Linker Flags ====

if (CXX_MSVC)
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
    if (WERROR)
        list(APPEND FULL_WARNING /WX)
        list(APPEND MIN_WARNING /WX)
    endif ()
elseif (CXX_GCC OR CXX_MINGW OR CXX_INTEL)
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
    if (CXX_MINGW)
        list(APPEND FULL_WARNING -Wa,-mbig-obj)
        list(APPEND MIN_WARNING -Wa,-mbig-obj)
        set(USE_PCH OFF)
    endif ()
    if (WERROR)
        list(APPEND FULL_WARNING -Werror)
        list(APPEND MIN_WARNING -Werror)
    endif ()
elseif (CXX_CLANG OR CXX_CLANG_CL)
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
            -Wno-assume
            -Wno-poison-system-directories)
    set(MIN_WARNING -Wall)
    if (CXX_CLANG_CL)
        list(APPEND FULL_WARNING
                /bigobj
                -D_WIN32_WINNT=0x0A00)
        list(APPEND MIN_WARNING
                /bigobj
                -D_WIN32_WINNT=0x0A00)
    endif ()
    if (WERROR)
        list(APPEND FULL_WARNING -Werror)
        list(APPEND MIN_WARNING -Werror)
    endif ()
endif ()

# ==== Target(s) ====

add_library(extenser INTERFACE include/extenser/extenser.hpp)
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
        GIT_TAG 55f93686c01528224f448c19128836e7df245f72 # 3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(extenser-json INTERFACE include/extenser/json_adapter/extenser_json.hpp)
target_link_libraries(extenser-json INTERFACE extenser)
target_link_libraries_system(extenser-json INTERFACE nlohmann_json::nlohmann_json)

if (USE_MAGIC_ENUM OR BUILD_TESTING)
    FetchContent_Declare(magic_enum
            GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
            GIT_TAG e046b69a3736d314fad813e159b1c192eaef92cd # 0.9.7
    )
    FetchContent_MakeAvailable(magic_enum)

    if (USE_MAGIC_ENUM)
        target_link_libraries_system(extenser-json INTERFACE magic_enum::magic_enum)
        target_compile_definitions(extenser-json INTERFACE EXTENSER_USE_MAGIC_ENUM)
    endif ()
endif ()

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

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif ()

if (BUILD_BITSERY)
    add_subdirectory(extras/bitsery_adapter)
endif ()
