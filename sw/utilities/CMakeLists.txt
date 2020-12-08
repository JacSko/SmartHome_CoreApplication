add_library(string_formatter STATIC
        source/string_formatter.c
)

target_include_directories(string_formatter PUBLIC
        include
)

add_library(system_settings INTERFACE)

target_include_directories(system_settings INTERFACE
        include
)

if (UNIT_TESTS)
    add_subdirectory(test)
endif()
