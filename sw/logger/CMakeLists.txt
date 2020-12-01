if (NOT UNIT_TESTS)

add_library(logger STATIC
        source/Logger.c
)

target_include_directories(logger PUBLIC
        include
)

target_link_libraries(logger PUBLIC
        STM_HEADERS
)

else ()

#    add_subdirectory(test)

endif()