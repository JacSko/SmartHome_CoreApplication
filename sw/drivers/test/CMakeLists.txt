add_executable(uart_engine_tests
            unit/uart_engine_tests.cpp
)

target_include_directories(uart_engine_tests PUBLIC
        ../include
)
target_link_libraries(uart_engine_tests PUBLIC
        gtest_main
        gmock_main
        gpio_configMocks
        STM_HEADERS
)

add_test(NAME uart_engine_tests COMMAND uart_engine_tests)

###########################################################

add_executable(bt_engine_tests
            unit/bt_engine_tests.cpp
)

target_include_directories(bt_engine_tests PUBLIC
        ../include
)
target_link_libraries(bt_engine_tests PUBLIC
        gtest_main
        gmock_main
        gpio_configMocks
        STM_HEADERS
)

add_test(NAME bt_engine_tests COMMAND bt_engine_tests)

###########################################################

add_executable(wifi_driver_tests
            unit/wifi_driver_tests.cpp
)

target_include_directories(wifi_driver_tests PUBLIC
        ../include
)
target_link_libraries(wifi_driver_tests PUBLIC
        gtest_main
        gmock_main
        uartengine_mock
        time_counterMock
        loggerMock
)

add_test(NAME wifi_driver_tests COMMAND wifi_driver_tests)

###########################################################
add_executable(wifi_manager_tests
            unit/wifi_manager_tests.cpp
)

target_include_directories(wifi_manager_tests PUBLIC
        ../include
)
target_link_libraries(wifi_manager_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        wifidriver_mock
        STM_HEADERS
        time_counterMock
)

add_test(NAME wifi_manager_tests COMMAND wifi_manager_tests)

###########################################################
add_executable(dht_driver_tests
            unit/dht_driver_tests.cpp
)

target_include_directories(dht_driver_tests PUBLIC
        ../include
)
target_link_libraries(dht_driver_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        gpio_configMocks
        STM_HEADERS
        task_scheduler_mock
)

add_test(NAME dht_driver_tests COMMAND dht_driver_tests)

###########################################################
add_executable(i2c_driver_tests
            unit/i2c_driver_tests.cpp
)

target_include_directories(i2c_driver_tests PUBLIC
        ../include
)
target_link_libraries(i2c_driver_tests PUBLIC
        gtest_main
        gmock_main
        loggerMock
        gpio_configMocks
        STM_HEADERS
        task_scheduler_mock
)

add_test(NAME i2c_driver_tests COMMAND i2c_driver_tests)

###########################################################







###############################
#	  Driver mocks
###############################
add_library(gpio_configMocks INTERFACE)
target_include_directories(gpio_configMocks INTERFACE
          mocks
          ../include
)

add_library(btengine_mock STATIC
			mocks/btengine_mock.h
)
set_target_properties(btengine_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(btengine_mock PUBLIC
          ../include
          mocks/
)

add_library(uartengine_mock STATIC
			mocks/uartengine_mock.h
)
set_target_properties(uartengine_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(uartengine_mock PUBLIC
          ../include
          mocks/
)

add_library(wifidriver_mock STATIC
			mocks/wifidriver_mock.h
)
set_target_properties(wifidriver_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(wifidriver_mock PUBLIC
          ../include
          mocks/
)
add_library(wifimanager_mock STATIC
			mocks/wifimanager_mock.h
)
set_target_properties(wifimanager_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(wifimanager_mock PUBLIC
          ../include
          mocks/
)
add_library(dhtdriver_mock STATIC
			mocks/dht_driver_mock.h
)
set_target_properties(dhtdriver_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(dhtdriver_mock PUBLIC
          ../include
          mocks/
)
add_library(i2cdriver_mock STATIC
			mocks/i2c_driver_mock.h
)
set_target_properties(i2cdriver_mock PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(i2cdriver_mock PUBLIC
          ../include
          mocks/
)