enable_testing()

add_executable(extenser-test main.cpp)
target_link_libraries(extenser-test PRIVATE extenser-json)
target_compile_features(extenser-test PRIVATE cxx_std_17)
add_test(tester extenser-test)
