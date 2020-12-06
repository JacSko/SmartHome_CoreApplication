if (NOT UNIT_TESTS)

add_library(gpio_config STATIC
        source/gpio_lib.c
)

target_include_directories(gpio_config PUBLIC
    include
)

target_link_libraries(gpio_config PUBLIC
        STM_HEADERS
)

else()

	add_subdirectory(test)
    
endif()