add_executable(env_monitor_tests
            unit/env_monitor_tests.cpp
)

target_include_directories(env_monitor_tests PUBLIC
        ../include
)
target_link_libraries(env_monitor_tests PUBLIC
        gtest_main
        gmock_main
        dhtdriver_mock
        task_scheduler_mock
        loggerMock
)

add_test(NAME env_monitor_tests COMMAND env_monitor_tests)

##########################################################

add_executable(bathroom_fan_tests
            unit/bathroom_fan_tests.cpp
)

target_include_directories(bathroom_fan_tests PUBLIC
        ../include
)
target_link_libraries(bathroom_fan_tests PUBLIC
        gtest_main
        gmock_main
        relays_board_mock
        env_monitor_mock
        task_scheduler_mock
        loggerMock
)

add_test(NAME bathroom_fan_tests COMMAND bathroom_fan_tests)
###############################
#	  env mocks
###############################
add_library(env_monitor_mock STATIC
			mocks/env_monitor_mock.h
)
set_target_properties(env_monitor_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(env_monitor_mock PUBLIC
          ../include
          mocks/
)
add_library(bathroom_fan_mock STATIC
			mocks/bathroom_fan_mock.h
)
set_target_properties(bathroom_fan_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(bathroom_fan_mock PUBLIC
          ../include
          mocks/
)