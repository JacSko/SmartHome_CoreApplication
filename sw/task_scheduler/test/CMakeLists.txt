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
        time_counterMock
        loggerMock
)

add_test(NAME task_scheduler_tests COMMAND task_scheduler_tests)

###############################################################################


add_library(task_scheduler_mock STATIC
			mocks/task_scheduler_mock.h
)
    target_include_directories(task_scheduler_mock PUBLIC
                mocks
                ../include
    )

set_target_properties(task_scheduler_mock PROPERTIES LINKER_LANGUAGE CXX)