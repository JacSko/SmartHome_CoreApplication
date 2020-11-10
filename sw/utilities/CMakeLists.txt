
add_library(list_generic STATIC
        source/list_generic.c
)

target_include_directories(list_generic PUBLIC
        include
)

target_link_libraries(list_generic PUBLIC
        STM_HEADERS
)

if (UNIT_TESTS)
    add_subdirectory(test)
endif()
