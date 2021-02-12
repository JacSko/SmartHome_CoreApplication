if (STM)

add_library(time_counter STATIC
        source/time_counter.c
)

target_include_directories(time_counter PUBLIC
        include
)

target_link_libraries(time_counter PUBLIC
        STM_HEADERS
        logger
        string_formatter
)
###################################################
add_library(system_timestamp STATIC
        source/system_timestamp.c
)

target_include_directories(system_timestamp PUBLIC
        include
)

target_link_libraries(system_timestamp PUBLIC
        STM_HEADERS
        gpio_config
)
endif()

if (UNIT_TESTS)

    add_subdirectory(test)
    
endif()