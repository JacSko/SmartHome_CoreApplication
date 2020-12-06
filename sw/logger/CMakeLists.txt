if (NOT UNIT_TESTS)

add_library(logger STATIC
        source/Logger.c
)

target_include_directories(logger PUBLIC
        include
)

target_link_libraries(logger PUBLIC
        STM_HEADERS
        bt_engine
        time_counter
)

else ()
add_library(loggerMock STATIC
			test/mocks/logger_mock.h
)
    target_include_directories(loggerMock PUBLIC
                test/mocks
                include
    )
target_link_libraries(loggerMock PUBLIC
	string_formatter
)

set_target_properties(loggerMock PROPERTIES LINKER_LANGUAGE CXX)
    add_subdirectory(test)

endif()