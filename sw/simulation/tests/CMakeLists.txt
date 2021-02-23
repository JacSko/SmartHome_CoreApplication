add_executable(hw_stub_tests
            unit/hw_stub_tests.cpp
)

target_include_directories(hw_stub_tests PUBLIC
        ../include
)
target_link_libraries(hw_stub_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        socket_driver_mock
        driversIf
        STM_HEADERS
)

add_test(NAME hw_stub_tests COMMAND hw_stub_tests)



# MOCKS
add_library(socket_driver_mock STATIC
			mocks/socket_driver_mock.h
)
set_target_properties(socket_driver_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(socket_driver_mock PUBLIC
          ../include
          mocks/
)