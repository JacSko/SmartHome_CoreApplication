add_executable(i2c_driver_sim_tests
            unit/i2c_driver_sim_tests.cpp
)

target_include_directories(i2c_driver_sim_tests PUBLIC
        ../include
)
target_link_libraries(i2c_driver_sim_tests PUBLIC
        gtest_main
        gmock_main
        driversIf
        loggerMock
        socket_driver_mock
        hw_stub
)

add_test(NAME i2c_driver_sim_tests COMMAND i2c_driver_sim_tests)

add_executable(dht_driver_sim_tests
            unit/dht_driver_sim_tests.cpp
)

target_include_directories(dht_driver_sim_tests PUBLIC
        ../include
)
target_link_libraries(dht_driver_sim_tests PUBLIC
        gtest_main
        gmock_main
        driversIf
        loggerMock
        socket_driver_mock
        hw_stub
)

add_test(NAME dht_driver_sim_tests COMMAND dht_driver_sim_tests)



# MOCKS
add_library(socket_driver_mock STATIC
			mocks/socket_driver_mock.h
)
set_target_properties(socket_driver_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(socket_driver_mock PUBLIC
          ../include
          mocks/
)