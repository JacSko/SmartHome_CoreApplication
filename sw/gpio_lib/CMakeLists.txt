add_library(gpio_config STATIC
        gpio_lib.c
)

target_include_directories(gpio_config PUBLIC
    ./
)

target_link_libraries(gpio_config PUBLIC
        STM_HEADERS
)
