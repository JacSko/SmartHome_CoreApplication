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
        SmartHomeTypes
)

add_test(NAME stairs_led_module_tests COMMAND stairs_led_module_tests)

###########################################################
add_library(stairs_led_module_mock STATIC
			mocks/stairs_led_module_mock.h
)
set_target_properties(stairs_led_module_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(stairs_led_module_mock PUBLIC
          ../include
          mocks/
)