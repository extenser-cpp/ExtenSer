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
    GIT_TAG ae7a13539fb71f270b87eb2e874fbac80bc8dda2
    )
FetchContent_MakeAvailable(doctest)

add_library(doctest_runner OBJECT test_runner.cpp)
target_include_directories(doctest_runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries_system(doctest_runner PUBLIC doctest::doctest)

add_executable(extenser-test test_main.cpp)
target_link_libraries(extenser-test PRIVATE doctest_runner extenser-json)
target_compile_features(extenser-test PRIVATE cxx_std_17)
target_compile_options(extenser-test PRIVATE ${FULL_WARNING})
doctest_discover_tests(extenser-test ADD_LABELS 1)

add_executable(json-test json_adapter/json.deser.test.cpp json_adapter/json.ser.test.cpp test_helpers.hpp)
target_link_libraries(json-test PRIVATE doctest_runner extenser-json)
target_compile_features(json-test PRIVATE cxx_std_17)
target_compile_options(json-test PRIVATE ${FULL_WARNING})
get_target_property(json-test-defs json-test COMPILE_DEFINITIONS)
list(FILTER json_test_defs EXCLUDE REGEX [[^EXTENSER_USE_MAGIC_ENUM$]])
set_property(TARGET json-test PROPERTY COMPILE_DEFINITIONS ${json_test_defs})

add_executable(json-magic-test json_adapter/json.deser.test.cpp json_adapter/json.ser.test.cpp test_helpers.hpp)
target_link_libraries(json-magic-test PRIVATE doctest_runner extenser-json)
target_link_libraries_system(json-magic-test PRIVATE magic_enum::magic_enum)
target_compile_definitions(json-magic-test PRIVATE EXTENSER_USE_MAGIC_ENUM)
target_compile_features(json-magic-test PRIVATE cxx_std_17)
target_compile_options(json-magic-test PRIVATE ${FULL_WARNING})

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
  include(CodeCoverage)
  
  set(GCOVR_ADDITIONAL_ARGS --merge-mode-functions=separate)
  set(COVERAGE_EXCLUDES "${PROJECT_BINARY_DIR}/*" "${PROJECT_SOURCE_DIR}/tests/*")

  setup_target_for_coverage_gcovr_html(NAME json_gcovr EXECUTABLE json-test DEPENDENCIES json-test)
  setup_target_for_coverage_lcov(NAME json_lcov EXECUTABLE json-test DEPENDENCIES json-test)
  target_compile_definitions(json-test PRIVATE EXTENSER_ASSERT_NONE)

  setup_target_for_coverage_gcovr_html(NAME json_magic_gcovr EXECUTABLE json-magic-test DEPENDENCIES json-magic-test)
  setup_target_for_coverage_lcov(NAME json_magic_lcov EXECUTABLE json-magic-test DEPENDENCIES json-magic-test)
  target_compile_definitions(json-magic-test PRIVATE EXTENSER_ASSERT_NONE)
endif ()

doctest_discover_tests(json-test ADD_LABELS 1)
doctest_discover_tests(json-magic-test ADD_LABELS 1)
