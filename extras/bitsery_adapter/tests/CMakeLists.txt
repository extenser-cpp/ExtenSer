# ExtenSer - An extensible, generic serialization library for C++
#
# Copyright (c) 2023 by Jackson Harmer
#
# SPDX-License-Identifier: BSD-3-Clause
# Distributed under The 3-Clause BSD License
# See accompanying file LICENSE or a copy at
# https://opensource.org/license/bsd-3-clause/

enable_testing()

include(CTest)
include(doctest)

# FetchContent_Declare(doctest
#     GIT_REPOSITORY https://github.com/doctest/doctest.git
#     GIT_TAG ae7a13539fb71f270b87eb2e874fbac80bc8dda2
#     )
# FetchContent_MakeAvailable(doctest)

# add_library(doctest_runner OBJECT test_runner.cpp)
# target_link_libraries_system(doctest_runner PUBLIC doctest::doctest)

add_executable(bitsery-test bitsery.test.cpp)
target_link_libraries(bitsery-test PRIVATE doctest_runner extenser-bitsery)
target_compile_features(bitsery-test PRIVATE cxx_std_17)
target_compile_options(bitsery-test PRIVATE ${FULL_WARNING})

doctest_discover_tests(bitsery-test ADD_LABELS 1)
