add_library(STM_HEADERS INTERFACE)

target_include_directories(STM_HEADERS INTERFACE
							CMSIS/device
							CMSIS/core
							StdPeriph_Driver/inc
)

if (UNIT_TESTS)
	add_subdirectory(googletest)
endif()