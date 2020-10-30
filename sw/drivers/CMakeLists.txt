add_library(math_lib SHARED source/test.c)

target_include_directories(math_lib PUBLIC
							include
)

target_link_libraries(math_lib STM_HEADERS)

if (UNIT_TESTS)
	add_subdirectory(test)
endif()