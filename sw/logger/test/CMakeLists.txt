add_executable(uart_engine_tests
            unit/uart_engine_tests.cpp
)

target_link_libraries(uart_engine_tests PRIVATE
        gtest_main
        gmock_main
        uart_engine
        STM_HEADERS
)

add_test(NAME uart_engine_tests COMMAND uart_engine_tests)