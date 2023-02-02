enable_testing()

include(FetchContent)
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG b7c21ec5ceeadb4951b00396fc1e4642dd347e5f
    )
FetchContent_MakeAvailable(doctest)

add_executable(extenser-test test_main.cpp)
target_link_libraries(extenser-test PRIVATE extenser-bitsery extenser-json doctest::doctest)
target_compile_features(extenser-test PRIVATE cxx_std_17)
add_test(tester extenser-test)
