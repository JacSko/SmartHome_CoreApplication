add_library(time_counter STATIC
        source/time_counter.c
)

target_include_directories(time_counter PUBLIC
        include
)

target_link_libraries(time_counter PUBLIC
        STM_HEADERS
)




if (UNIT_TESTS)
    add_library(time_counterIf INTERFACE)
    target_include_directories(time_counterIf INTERFACE
                include
    )

    add_library(time_counterMock INTERFACE)
    target_include_directories(time_counterMock INTERFACE
                test/mocks
    )


    add_subdirectory(test)
endif()