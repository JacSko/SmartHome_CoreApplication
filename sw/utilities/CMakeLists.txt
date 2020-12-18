add_library(string_formatter STATIC
        source/string_formatter.c
)

target_include_directories(string_formatter PUBLIC
        include/
)

if (UNIT_TESTS)
    add_subdirectory(test)
endif()
