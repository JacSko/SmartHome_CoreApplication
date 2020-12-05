if (NOT UNIT_TESTS)

add_library(time_counter STATIC
        source/time_counter.c
)

target_include_directories(time_counter PUBLIC
        include
)

target_link_libraries(time_counter PUBLIC
        STM_HEADERS
        logger
)


else ()

    add_library(time_counterMock INTERFACE)
    target_include_directories(time_counterMock INTERFACE
                test/mocks
                include
    )
	target_link_libraries(time_counterMock INTERFACE
	        STM_HEADERS
	)
	
    add_subdirectory(test)
    
endif()