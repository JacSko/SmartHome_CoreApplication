add_executable(string_formatter_tests
            unit/string_formatter_tests.cpp
)
target_include_directories(string_formatter_tests PUBLIC
        ../include
)
target_link_libraries(string_formatter_tests PUBLIC
        gtest_main
        gmock_main
)
add_test(NAME string_formatter_tests COMMAND string_formatter_tests)

###########################################################