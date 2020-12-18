if (NOT UNIT_TESTS)

###############################
# 	  STM targets here
###############################

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

############################

add_library(bt_engine STATIC
        source/bt_engine.c
)

target_include_directories(bt_engine PUBLIC
        include
)

target_link_libraries(bt_engine PUBLIC
        gpio_config
        STM_HEADERS
)

############################

add_library(wifi_driver STATIC
        source/wifi_driver.c
)

target_include_directories(wifi_driver PUBLIC
        include
)

target_link_libraries(wifi_driver PUBLIC
        time_counter
        uart_engine
        string_formatter
        logger
)

############################

add_library(wifi_manager STATIC
        source/wifi_manager.c
)

target_include_directories(wifi_manager PUBLIC
        include
)

target_link_libraries(wifi_manager PUBLIC
        time_counter
        wifi_driver
        logger
)

############################

add_library(dht_driver STATIC
        source/dht_driver.c
)

target_include_directories(dht_driver PUBLIC
        include
)

target_link_libraries(dht_driver PUBLIC
		STM_HEADERS
		logger
		task_scheduler
)

############################

add_library(i2c_driver STATIC
        source/i2c_driver.c
)

target_include_directories(i2c_driver PUBLIC
        include
)

target_link_libraries(i2c_driver PUBLIC
		STM_HEADERS
		logger
		task_scheduler
)



else()

	add_subdirectory(test)

endif()