add_executable(command_parser_tests
            unit/command_parser_tests.cpp
)

target_include_directories(command_parser_tests PUBLIC
        ../include
)
target_link_libraries(command_parser_tests PUBLIC
        gtest_main
        gmock_main
        STM_HEADERS
        time_counterMock
		wifimanager_mock
		inputs_board_mock
		relays_board_mock
		i2cdriver_mock
		dhtdriver_mock
		bathroom_fan_mock
		env_monitor_mock
		loggerMock
		stairs_led_module_mock
		string_formatter
)

add_test(NAME command_parser_tests COMMAND command_parser_tests)

