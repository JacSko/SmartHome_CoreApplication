add_executable(stairs_led_module_tests
            unit/stairs_led_module_tests.cpp
)

target_include_directories(stairs_led_module_tests PUBLIC
        ../include
)
target_link_libraries(stairs_led_module_tests PUBLIC
        gtest_main
        gmock_main
        task_scheduler_mock
        i2cdriver_mock
        inputs_board_mock
        loggerMock
)

add_test(NAME stairs_led_module_tests COMMAND stairs_led_module_tests)

###########################################################
