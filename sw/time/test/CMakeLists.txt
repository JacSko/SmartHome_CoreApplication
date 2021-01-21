add_executable(time_tests
            unit/time_counter_tests.cpp
)

target_include_directories(time_tests PUBLIC
        ../include
)
target_link_libraries(time_tests PUBLIC
        gtest_main
        gmock_main
        STM_HEADERS
        loggerMock
)

add_test(NAME time_tests COMMAND time_tests)

###########################################################
add_executable(system_timestamp_tests
            unit/system_timestamp_tests.cpp
)

target_include_directories(system_timestamp_tests PUBLIC
        ../include
)
target_link_libraries(system_timestamp_tests PUBLIC
        gtest_main
        gmock_main
        STM_HEADERS
)
add_test(NAME system_timestamp_tests COMMAND system_timestamp_tests)

# MOCKS

add_library(time_counterMock INTERFACE)
target_include_directories(time_counterMock INTERFACE
            mocks
            ../include
)
target_link_libraries(time_counterMock INTERFACE
        STM_HEADERS
)

add_library(system_timestamp_mock INTERFACE)
target_include_directories(system_timestamp_mock INTERFACE
            mocks
            ../include
)
target_link_libraries(system_timestamp_mock INTERFACE
        STM_HEADERS
)