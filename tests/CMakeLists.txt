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

add_executable(json-test json_deser.test.cpp json.ser.test.cpp json.test.hpp)
target_link_libraries(json-test PRIVATE doctest_runner extenser-json)
target_compile_features(json-test PRIVATE cxx_std_17)
target_compile_options(json-test PRIVATE ${FULL_WARNING})

if (CODE_COVERAGE)
  setup_target_for_coverage_gcovr_html(NAME json_gcovr EXECUTABLE json-test DEPENDENCIES json-test)
  setup_target_for_coverage_lcov(NAME json_lcov EXECUTABLE json-test DEPENDENCIES json-test)
  target_link_libraries_system(json-test PRIVATE gcov)
  target_compile_definitions(json-test PRIVATE EXTENSER_ASSERT_NONE)
endif ()

add_test(nlohmann-json json-test)
