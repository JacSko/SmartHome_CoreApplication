add_executable(uart_engine_tests
            unit/uart_engine_tests.cpp
)

target_include_directories(uart_engine_tests PUBLIC
        ../include
)
target_link_libraries(uart_engine_tests PUBLIC
        gtest_main
        gmock_main
        gpio_configMocks
        STM_HEADERS
)

add_test(NAME uart_engine_tests COMMAND uart_engine_tests)

###########################################################

add_executable(bt_engine_tests
            unit/bt_engine_tests.cpp
)

target_include_directories(bt_engine_tests PUBLIC
        ../include
)
target_link_libraries(bt_engine_tests PUBLIC
        gtest_main
        gmock_main
        gpio_configMocks
        STM_HEADERS
)

add_test(NAME bt_engine_tests COMMAND bt_engine_tests)

###########################################################

add_executable(wifi_driver_tests
            unit/wifi_driver_tests.cpp
)

target_include_directories(wifi_driver_tests PUBLIC
        ../include
)
target_link_libraries(wifi_driver_tests PUBLIC
        gtest_main
        gmock_main
        driversMock
        time_counterMock
        string_formatter
        loggerMock
)

add_test(NAME wifi_driver_tests COMMAND wifi_driver_tests)

###########################################################
add_executable(wifi_manager_tests
            unit/wifi_manager_tests.cpp
)

target_include_directories(wifi_manager_tests PUBLIC
        ../include
)
target_link_libraries(wifi_manager_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        driversMock
        STM_HEADERS
        time_counterMock
)

add_test(NAME wifi_manager_tests COMMAND wifi_manager_tests)

###########################################################