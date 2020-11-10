add_library(task_scheduler STATIC
        source/task_scheduler.c
)

target_include_directories(task_scheduler PUBLIC
        include
)

target_link_libraries(task_scheduler PUBLIC
        STM_HEADERS
        time_counter
        list_generic
)

##############################################################


if (UNIT_TESTS)
    add_subdirectory(test)
endif()
