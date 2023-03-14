enable_testing()

include(FetchContent)
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG 8fdfd113dcb4ad1a31705ff8ddb13a21a505bad8
    )
FetchContent_MakeAvailable(doctest)

add_library(doctest_runner OBJECT test_runner.cpp)
target_link_libraries_system(doctest_runner PUBLIC doctest::doctest)

add_executable(extenser-test test_main.cpp)
target_link_libraries(extenser-test PRIVATE doctest_runner extenser-bitsery extenser-json)
target_compile_features(extenser-test PRIVATE cxx_std_17)
target_compile_options(extenser-test PRIVATE ${FULL_WARNING})
add_test(tester extenser-test)

add_executable(json-test json.deser.test.cpp json.ser.test.cpp json.test.hpp)
target_link_libraries(json-test PRIVATE doctest_runner extenser-json)
target_compile_features(json-test PRIVATE cxx_std_17)
target_compile_options(json-test PRIVATE ${FULL_WARNING})
get_target_property(json-test-defs json-test COMPILE_DEFINITIONS)
list(FILTER json-test-defs EXCLUDE REGEX [[^EXTENSER_USE_MAGIC_ENUM$]])
set_property(TARGET json-test PROPERTY COMPILE_DEFINITIONS ${json-test-defs})

add_executable(json-magic-test json.deser.test.cpp json.ser.test.cpp json.test.hpp)
target_link_libraries(json-magic-test PRIVATE doctest_runner extenser-json magic_enum::magic_enum)
target_compile_definitions(json-magic-test PRIVATE EXTENSER_USE_MAGIC_ENUM)
target_compile_features(json-magic-test PRIVATE cxx_std_17)
target_compile_options(json-magic-test PRIVATE ${FULL_WARNING})

if (CODE_COVERAGE)
  setup_target_for_coverage_gcovr_html(NAME json_gcovr EXECUTABLE json-test DEPENDENCIES json-test)
  setup_target_for_coverage_lcov(NAME json_lcov EXECUTABLE json-test DEPENDENCIES json-test)
  target_link_libraries_system(json-test PRIVATE gcov)
  target_compile_definitions(json-test PRIVATE EXTENSER_ASSERT_NONE)

  setup_target_for_coverage_gcovr_html(NAME json_gcovr EXECUTABLE json-magic-test DEPENDENCIES json-magic-test)
  setup_target_for_coverage_lcov(NAME json_lcov EXECUTABLE json-magic-test DEPENDENCIES json-magic-test)
  target_link_libraries_system(json-magic-test PRIVATE gcov)
  target_compile_definitions(json-magic-test PRIVATE EXTENSER_ASSERT_NONE)
endif ()

add_test(nlohmann-json json-test)
add_test(nlohmann-json-magic json-magic-test)
