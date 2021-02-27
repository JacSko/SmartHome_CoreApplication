if (SIMULATION)
	# TIME_COUNTER
	add_library(time_counter STATIC
	        source/time_counter_sim.c
	)
	
	target_include_directories(time_counter PUBLIC
	        include
	)
	
	target_link_libraries(time_counter PUBLIC
			timeIf
	)
	
	
	# LOGGER
	add_library(logger STATIC
	        source/logger_sim.c
	)
	target_include_directories(logger PUBLIC
	        include
	)
	
	target_link_libraries(logger PUBLIC
		loggerIf
		time_counter
		string_formatter
	)
	
	# SYSTEM_TIMESTAMP
	add_library(system_timestamp STATIC
	        source/system_timestamp_sim.c
	)
	
	target_include_directories(system_timestamp PUBLIC
	        include
	)
	
	target_link_libraries(system_timestamp PUBLIC
		timeIf
	)
	
	# SOCKET DRIVER
	add_library(socket_driver STATIC
	        source/socket_driver.c
	)
	
	target_include_directories(socket_driver PUBLIC
	        include
	)
	
	target_link_libraries(socket_driver PUBLIC
			logger
	)
	
	# BLUETOOTH
	add_library(bt_engine STATIC
	        source/bt_engine_sim.c
	)
	
	target_include_directories(bt_engine PUBLIC
	        include
	)
	
	target_link_libraries(bt_engine PUBLIC
		driversIf
		socket_driver
		logger
	)
	# HWSTUB
	add_library(hw_stub STATIC
	        source/hw_stub.c
	)
	
	target_include_directories(hw_stub PUBLIC
	        include
	)
	
	target_link_libraries(hw_stub PUBLIC
		driversIf
		socket_driver
		inputs_board
		logger
	)
	# I2C DRIVER
	add_library(i2c_driver STATIC
	        source/i2c_driver_sim.c
	)
	
	target_include_directories(i2c_driver PUBLIC
	        include
	)
	
	target_link_libraries(i2c_driver PUBLIC
		driversIf
		socket_driver
		logger
		hw_stub
	)
	# DHT DRIVER
	add_library(dht_driver STATIC
	        source/dht_driver_sim.c
	)
	
	target_include_directories(dht_driver PUBLIC
	        include
	)
	
	target_link_libraries(dht_driver PUBLIC
		driversIf
		socket_driver
		logger
		hw_stub
	)
	
add_library(wifi_driver STATIC
        source/wifi_driver_sim.c
)

target_include_directories(wifi_driver PUBLIC
        include
)

target_link_libraries(wifi_driver PUBLIC
		driversIf
		socket_driver
		logger
		hw_stub
)

endif()

if (UNIT_TESTS)

	add_subdirectory(tests)
  
  	add_library(hw_stub STATIC
	        source/hw_stub.c
	)
	
	target_include_directories(hw_stub PUBLIC
	        include
	)
	
	target_link_libraries(hw_stub PUBLIC
		driversIf
		socket_driver_mock
		loggerMock
		inputs_board_mock
	)
endif()
