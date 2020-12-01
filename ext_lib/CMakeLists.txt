if (NOT UNIT_TESTS)

add_library(STM_HEADERS INTERFACE)
target_include_directories(STM_HEADERS INTERFACE
							CMSIS/device
							CMSIS/core
							StdPeriph_Driver/inc
)

else()

add_library(STM_HEADERS STATIC
            CMSIS/stubs/device/stm32f4xx.c
)
target_include_directories(STM_HEADERS INTERFACE
							CMSIS/stubs/device
							CMSIS/stubs/core
							StdPeriph_Driver/inc
)

add_subdirectory(googletest)

endif()
