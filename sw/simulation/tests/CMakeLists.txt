add_executable(socket_driver_tests
            unit/socket_driver_tests.cpp
)

target_include_directories(socket_driver_tests PUBLIC
        ../include
)
target_link_libraries(socket_driver_tests PUBLIC
        gtest_main
        gmock_main
		loggerMock
)

add_test(NAME socket_driver_tests COMMAND socket_driver_tests)
