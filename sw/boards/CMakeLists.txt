if (NOT UNIT_TESTS)

###############################
# 	  STM targets here
###############################

add_library(inputs_board STATIC
        source/inputs_board.c
)

target_include_directories(inputs_board PUBLIC
        include
)

target_link_libraries(inputs_board PUBLIC
		STM_HEADERS
		logger
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


else()

	add_subdirectory(test)

endif()