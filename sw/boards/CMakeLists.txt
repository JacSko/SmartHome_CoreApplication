if (STM OR SIMULATION)

###############################
# 	  STM targets here
###############################
add_library(inputs_interrupt_handler STATIC
        source/inputs_interrupt_handler.c
)

target_include_directories(inputs_interrupt_handler PUBLIC
        include
)

target_link_libraries(inputs_interrupt_handler PUBLIC
		STM_HEADERS
		gpio_config
		SmartHomeTypes
)

add_library(inputs_board STATIC
        source/inputs_board.c
)

target_include_directories(inputs_board PUBLIC
        include
)

target_link_libraries(inputs_board PUBLIC
		STM_HEADERS
		logger
		inputs_interrupt_handler
		task_scheduler
		i2c_driver
		gpio_config
		SmartHomeTypes
)
##############################
add_library(relays_board STATIC
        source/relays_board.c
)

target_include_directories(relays_board PUBLIC
        include
)

target_link_libraries(relays_board PUBLIC
		STM_HEADERS
		logger
		task_scheduler
		i2c_driver
		SmartHomeTypes
)
endif()

if (UNIT_TESTS)

	add_subdirectory(test)

endif()