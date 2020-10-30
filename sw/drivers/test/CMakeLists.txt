add_executable(test_tests unit/test_tests.cpp)

target_link_libraries(test_tests
	gtest_main
	gmock_main
	math_lib
)

add_test(NAME test_tests COMMAND test_tests)