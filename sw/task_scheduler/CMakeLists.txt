add_library(task_scheduler STATIC
        source/task_scheduler.c
)

target_include_directories(task_scheduler PUBLIC
        include
)

target_link_libraries(task_scheduler PUBLIC
        STM_HEADERS
        time_counter
)

##############################################################


if (UNIT_TESTS)
    add_subdirectory(test)
endif()
