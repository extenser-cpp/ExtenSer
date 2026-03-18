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

add_executable(bitsery_test bitsery.test.cpp)
target_link_libraries(bitsery_test PRIVATE doctest_runner extenser_bitsery)
target_compile_features(bitsery_test PRIVATE cxx_std_17)
target_compile_options(bitsery_test PRIVATE ${FULL_WARNING})

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CodeCoverage)

    set(GCOVR_ADDITIONAL_ARGS --merge-mode-functions=separate)
    set(COVERAGE_EXCLUDES "${PROJECT_BINARY_DIR}/*" "${PROJECT_SOURCE_DIR}/tests/*")

    setup_target_for_coverage_gcovr_html(NAME bitsery_gcovr EXECUTABLE bitsery_test DEPENDENCIES bitsery_test)
    setup_target_for_coverage_lcov(NAME bitsery_lcov EXECUTABLE bitsery_test DEPENDENCIES bitsery_test)
endif ()

doctest_discover_tests(bitsery_test ADD_LABELS 1)
