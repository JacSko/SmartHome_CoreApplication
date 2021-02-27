if (STM OR SIMULATION)

add_library(command_parser STATIC
		source/command_parser.c
)

target_include_directories(command_parser PUBLIC
		include/
)

target_link_libraries(command_parser PUBLIC
		wifi_manager
		stairs_led_module
		bathroom_fan
)
endif()

if (UNIT_TESTS)
  
add_subdirectory(test)
  
endif()