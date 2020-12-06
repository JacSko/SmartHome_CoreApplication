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

else()

###############################
#	  Driver mocks
###############################

add_library(driversMock STATIC
			test/mocks/btengine_mock.h
			test/mocks/uartengine_mock.h
			test/mocks/wifidriver_mock.h
)
set_target_properties(driversMock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(driversMock PUBLIC
          test/mocks
          include
)
        
    add_subdirectory(test)
endif()