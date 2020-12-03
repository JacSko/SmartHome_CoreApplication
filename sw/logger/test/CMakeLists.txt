add_executable(logger_tests
            unit/logger_tests.cpp
)

target_include_directories(logger_tests PUBLIC
        ../include
)
target_link_libraries(logger_tests PUBLIC
        gtest_main
        gmock_main
        time_counterMock
        STM_HEADERS
        string_formatter
)

add_test(NAME logger_tests COMMAND logger_tests)

###########################################################
