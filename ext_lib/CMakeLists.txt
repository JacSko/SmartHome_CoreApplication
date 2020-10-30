add_library(STM_HEADERS INTERFACE)

if (NOT UNIT_TESTS)
target_include_directories(STM_HEADERS INTERFACE
							CMSIS/device
							CMSIS/core
							StdPeriph_Driver/inc
)
else()
target_include_directories(STM_HEADERS INTERFACE
							CMSIS/stubs
							CMSIS/stubs/device
							CMSIS/stubs/core
							StdPeriph_Driver/inc
)
endif()

if (UNIT_TESTS)
	add_subdirectory(googletest)
endif()