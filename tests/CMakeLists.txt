enable_testing()

include(FetchContent)
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG 8fdfd113dcb4ad1a31705ff8ddb13a21a505bad8
    )
FetchContent_MakeAvailable(doctest)

add_executable(extenser-test test_main.cpp)
target_link_libraries(extenser-test PRIVATE extenser-bitsery extenser-json doctest::doctest)
target_compile_features(extenser-test PRIVATE cxx_std_17)
add_test(tester extenser-test)

add_executable(json-test json.test.cpp json_deser.test.cpp json.ser.test.cpp json.test.hpp)
target_link_libraries(json-test PRIVATE extenser-json doctest::doctest)
target_compile_features(json-test PRIVATE cxx_std_17)
add_test(nlohmann-json json-test)
