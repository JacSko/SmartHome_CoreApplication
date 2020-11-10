add_executable(task_list_tests
            unit/list_generic_tests.cpp
)

target_include_directories(task_list_tests PUBLIC
        ../include
)
target_link_libraries(task_list_tests PUBLIC
        gtest_main
        gmock_main
)

add_test(NAME task_list_tests COMMAND task_list_tests)