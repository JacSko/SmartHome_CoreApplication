add_executable(uart_engine_tests
            unit/uart_engine_tests.cpp
)

target_include_directories(uart_engine_tests PUBLIC
        ../include
)
target_link_libraries(uart_engine_tests PUBLIC
        gtest_main
        gmock_main
        gpio_configIf
        gpio_configMocks
        STM_HEADERS
)

add_test(NAME uart_engine_tests COMMAND uart_engine_tests)