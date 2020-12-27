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