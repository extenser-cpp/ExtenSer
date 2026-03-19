# ExtenSer - An extensible, generic serialization library for C++
#
# Copyright (c) 2023 by Jackson Harmer
#
# SPDX-License-Identifier: BSD-3-Clause
# Distributed under The 3-Clause BSD License
# See accompanying file LICENSE or a copy at
# https://opensource.org/license/bsd-3-clause/

enable_testing()

include(doctest)

FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG 1da23a3e8119ec5cce4f9388e91b065e20bf06f5 # 2.4.12
)
FetchContent_MakeAvailable(doctest)

add_library(doctest_runner OBJECT test_runner.cpp)
target_include_directories(doctest_runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries_system(doctest_runner PUBLIC doctest::doctest)

add_executable(extenser_test test_main.cpp)
target_link_libraries(extenser_test PRIVATE doctest_runner extenser_json)
target_compile_features(extenser_test PRIVATE cxx_std_17)
target_compile_options(extenser_test PRIVATE ${FULL_WARNING})
doctest_discover_tests(extenser_test ADD_LABELS 1)

add_executable(json_test json_adapter/json.deser.test.cpp json_adapter/json.ser.test.cpp test_helpers.hpp)
target_link_libraries(json_test PRIVATE doctest_runner extenser_json)
target_compile_features(json_test PRIVATE cxx_std_17)
target_compile_options(json_test PRIVATE ${FULL_WARNING})

get_target_property(json_test_defs json_test COMPILE_DEFINITIONS)
if (NOT json_test_defs STREQUAL "json_test_defs-NOTFOUND")
    list(FILTER json_test_defs EXCLUDE REGEX [[^EXTENSER_USE_MAGIC_ENUM$]])
    set_property(TARGET json_test PROPERTY COMPILE_DEFINITIONS ${json_test_defs})
endif ()

if (USE_MAGIC_ENUM)
    add_executable(json_magic_test json_adapter/json.deser.test.cpp json_adapter/json.ser.test.cpp test_helpers.hpp)
    target_link_libraries(json_magic_test PRIVATE doctest_runner extenser_json)
    target_link_libraries_system(json_magic_test PRIVATE magic_enum::magic_enum)
    target_compile_definitions(json_magic_test PRIVATE EXTENSER_USE_MAGIC_ENUM)
    target_compile_features(json_magic_test PRIVATE cxx_std_17)
    target_compile_options(json_magic_test PRIVATE ${FULL_WARNING})
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CodeCoverage)

    set(GCOVR_ADDITIONAL_ARGS --merge-mode-functions=separate)
    set(COVERAGE_EXCLUDES "${PROJECT_BINARY_DIR}/*" "${PROJECT_SOURCE_DIR}/tests/*")

    setup_target_for_coverage_gcovr_html(NAME json_gcovr EXECUTABLE json_test DEPENDENCIES json_test)
    setup_target_for_coverage_lcov(NAME json_lcov EXECUTABLE json_test DEPENDENCIES json_test)
    target_compile_definitions(json_test PRIVATE EXTENSER_ASSERT_NONE)

    if (USE_MAGIC_ENUM)
        setup_target_for_coverage_gcovr_html(NAME json_magic_gcovr EXECUTABLE json_magic_test DEPENDENCIES json_magic_test)
        setup_target_for_coverage_lcov(NAME json_magic_lcov EXECUTABLE json_magic_test DEPENDENCIES json_magic_test)
        target_compile_definitions(json_magic_test PRIVATE EXTENSER_ASSERT_NONE)
    endif ()
endif ()

doctest_discover_tests(json_test ADD_LABELS 1)

if (USE_MAGIC_ENUM)
    doctest_discover_tests(json_magic_test ADD_LABELS 1)
endif ()
