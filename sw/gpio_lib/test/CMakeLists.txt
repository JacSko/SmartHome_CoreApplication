

add_library(gpio_configMocks INTERFACE)
target_include_directories(gpio_configMocks INTERFACE
          mocks
          ../include
)