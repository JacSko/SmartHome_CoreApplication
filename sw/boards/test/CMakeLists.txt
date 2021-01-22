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
        SmartHomeTypes
)

add_test(NAME inputs_board_tests COMMAND inputs_board_tests)

###############################
add_executable(relays_board_tests
            unit/relays_board_tests.cpp
)

target_include_directories(relays_board_tests PUBLIC
        ../include
)
target_link_libraries(relays_board_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        task_scheduler_mock
        i2cdriver_mock
        SmartHomeTypes
)

add_test(NAME relays_board_tests COMMAND relays_board_tests)

###############################
#	  Boards mocks
###############################
add_library(relays_board_mock STATIC
			mocks/relays_board_mock.h
)
set_target_properties(relays_board_mock PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(relays_board_mock PUBLIC
			i2cdriver_mock
)
target_include_directories(relays_board_mock PUBLIC
          ../include
          mocks/
)


add_library(inputs_board_mock STATIC
			mocks/relays_board_mock.h
)
set_target_properties(inputs_board_mock PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(inputs_board_mock PUBLIC
			i2cdriver_mock
			SmartHomeTypes
)
target_include_directories(inputs_board_mock PUBLIC
          ../include
          mocks/
)
