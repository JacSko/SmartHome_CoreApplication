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
        SmartHomeTypes
)

###############################################
add_library(bathroom_fan STATIC
        source/bathroom_fan.c
)

target_include_directories(bathroom_fan PUBLIC
        include
)

target_link_libraries(bathroom_fan PUBLIC
        relays_board
        env_monitor
        task_scheduler
        logger
        SmartHomeTypes
)

###############################################

else ()

    add_subdirectory(test)

endif()