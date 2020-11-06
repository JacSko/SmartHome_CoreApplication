add_executable(time_tests
            unit/time_counter_tests.cpp
)

target_include_directories(time_tests PUBLIC
        ../include
)
target_link_libraries(time_tests PUBLIC
        gtest_main
        gmock_main
        STM_HEADERS
)

add_test(NAME time_tests COMMAND time_tests)