if (NOT UNIT_TESTS)

add_library(env_monitor STATIC
        source/env_monitor.c
)

target_include_directories(env_monitor PUBLIC
        include
)

target_link_libraries(env_monitor PUBLIC
        dht_driver
        task_scheduler
        logger
)

else ()

    add_subdirectory(test)

endif()