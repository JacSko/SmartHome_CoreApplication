add_executable(inputs_board_tests
            unit/inputs_board_tests.cpp
)

target_include_directories(inputs_board_tests PUBLIC
        ../include
)
target_link_libraries(inputs_board_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        gpio_configMocks
        STM_HEADERS
        task_scheduler_mock
        i2cdriver_mock
)

add_test(NAME inputs_board_tests COMMAND inputs_board_tests)
###############################
#	  Driver mocks
###############################

