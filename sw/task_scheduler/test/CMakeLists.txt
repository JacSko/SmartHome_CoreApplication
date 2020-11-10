add_executable(task_scheduler_tests
            unit/task_scheduler_tests.cpp
)

target_include_directories(task_scheduler_tests PUBLIC
        ../include
)
target_link_libraries(task_scheduler_tests PUBLIC
        gtest_main
        gmock_main
        STM_HEADERS
)

add_test(NAME task_scheduler_tests COMMAND task_scheduler_tests)

###############################################################################