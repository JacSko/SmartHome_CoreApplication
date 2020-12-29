cmake_minimum_required(VERSION 3.1)

project(smarthome)
###
#	Global settings
###

add_definitions(
		-DSTM32
		-DSTM32F4
		-DSTM32F411CEUx
		-DDEBUG
		-DSTM32F411xE
		-DUSE_STDPERIPH_DRIVER
	        )

if(NOT UNIT_TESTS)
	enable_language(C ASM)
	
	set(CMAKE_SYSTEM_NAME Generic)
	set(CMAKE_SYSTEM_PROCESSOR arm)
	set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
	set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
	set(CMAKE_AR "arm-none-eabi-ar" CACHE PATH "" FORCE)
	set(CMAKE_RANLIB "arm-none-eabi-ranlib" CACHE PATH "" FORCE)
	set(CMAKE_LINKER "arm-none-eabi-ld" CACHE PATH "" FORCE)
	set(CMAKE_SIZE "arm-none-eabi-size")
	set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
	
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
	set(CMAKE_SHARED_LIBRA
	RY_LINK_CXX_FLAGS "")
	add_compile_options(
	        -mcpu=cortex-m4
	        -mthumb
	        -mfpu=fpv4-sp-d16
	        -mfloat-abi=hard
	        -ffunction-sections
	        -Wall
	        -O0
	        -g3
	        )
	add_link_options(
	        -T${CMAKE_SOURCE_DIR}/ext_lib/LinkerScript.ld
	        -mcpu=cortex-m4
	        -mthumb
	        -mfloat-abi=hard
	        -mfpu=fpv4-sp-d16
	        #-lc
	        -lm
	        -Wl,-Map=${PROJECT_NAME}.map,--cref
	        -Wl,--gc-sections
	        )	
	
	set (SOURCES 
		main.c
		ext_lib/startup_stm32.s
		ext_lib/system_stm32f4xx.c
		ext_lib/syscalls.c
	)
	
	add_executable(${PROJECT_NAME}.elf ${SOURCES})
	
	target_link_libraries(${PROJECT_NAME}.elf PUBLIC
		STM_HEADERS
		uart_engine
		time_counter
		wifi_manager
		bt_engine
		logger
		task_scheduler
		command_parser
		dht_driver
		i2c_driver
		relays_board
		inputs_board
	)
	
#	target_include_directories(${PROJECT_NAME}.elf PUBLIC ext_lib/CMSIS/device
#														  ext_lib/CMSIS/core
#														  ext_lib/StdPeriph_Driver/inc)
	###
	#	CREATING BIN AND HEX
	###
	
	add_custom_command(TARGET ${PROJECT_NAME}.elf
	        POST_BUILD
	        COMMAND arm-none-eabi-objcopy -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
	        COMMAND arm-none-eabi-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
	        )
	        
	###
	#	PRINTING SIZE
	###
	
	add_custom_command(TARGET ${PROJECT_NAME}.elf
	        POST_BUILD
	        COMMAND arm-none-eabi-size ${PROJECT_NAME}.hex)
		find_program(ST_FLASH "st-flash")
	
	###
	#	FLASHING
	###
	if(NOT ST_FLASH)
		message( SEND_ERROR "Can't find ST-FLASH!!" )
	
	else()
		set(FLASH_ADDR  "0x8000000")
		add_custom_target(write 
			COMMAND ${ST_FLASH} write ${PROJECT_NAME}.bin ${FLASH_ADDR} 2>&1 | tail -2
		)
	
		add_custom_target(read
			COMMAND ${ST_FLASH} read ${PROJECT_NAME}_read.bin ${FLASH_ADDR} 2>&1 | tail -2
		)
	
		add_custom_target(erase
			COMMAND ${ST_FLASH} erase
		)
	
	endif(NOT ST_FLASH)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage -fPIC")

	enable_testing()
endif()

    include_directories(sw/utilities/include)
	add_subdirectory(sw/drivers)
	add_subdirectory(sw/logger)
	add_subdirectory(sw/time)
	add_subdirectory(sw/task_scheduler)
	add_subdirectory(ext_lib)
	add_subdirectory(sw/utilities)
	add_subdirectory(sw/command_parser)
	add_subdirectory(sw/boards)
	add_subdirectory(sw/env_monitor)
	
