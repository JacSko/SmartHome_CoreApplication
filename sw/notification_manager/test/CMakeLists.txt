add_executable(notification_manager_tests
            unit/notification_manager_tests.cpp
)

target_include_directories(notification_manager_tests PUBLIC
        ../include
)
target_link_libraries(notification_manager_tests PUBLIC
        gtest_main
        gmock_main
        wifimanager_mock
		time_counterMock
)

add_test(NAME notification_manager_tests COMMAND notification_manager_tests)
