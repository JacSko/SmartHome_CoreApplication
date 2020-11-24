add_library(uart_engine STATIC
        source/uart_engine.c
)

target_include_directories(uart_engine PUBLIC
        include
)

target_link_libraries(uart_engine PUBLIC
        gpio_config
        STM_HEADERS
)

##############################################################

add_library(wifi_driver STATIC
        source/wifi_driver.c
)

target_include_directories(wifi_driver PUBLIC
        include
)

target_link_libraries(wifi_driver PUBLIC
        time_counter
        uart_engine
)

##############################################################

if (UNIT_TESTS)
    add_library(driversMock INTERFACE)
    target_include_directories(driversMock INTERFACE
                test/mocks
    )
    add_subdirectory(test)
endif()