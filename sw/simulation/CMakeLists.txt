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

# BLUETOOTH
add_library(bt_engine STATIC
        source/bt_engine_sim.c
)

target_include_directories(bt_engine PUBLIC
        include
)

target_link_libraries(bt_engine PUBLIC
	driversIf
)