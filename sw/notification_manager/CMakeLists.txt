if (NOT UNIT_TESTS)

add_library(notification_manager STATIC
        source/notification_manager.c
)

target_include_directories(notification_manager PUBLIC
        include
)

target_link_libraries(notification_manager PUBLIC
		wifi_manager
		env_monitor
		inputs_board
		logger
		stairs_led_module
		time_counter
)

else ()

    add_subdirectory(test)

endif()