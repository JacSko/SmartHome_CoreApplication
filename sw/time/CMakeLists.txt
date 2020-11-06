add_library(time STATIC
        source/time_counter.c
)

target_include_directories(time PUBLIC
        include
)

target_link_libraries(time PUBLIC
        STM_HEADERS
)


if (UNIT_TESTS)
    add_subdirectory(test)
endif()