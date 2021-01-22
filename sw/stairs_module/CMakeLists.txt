if (NOT UNIT_TESTS)

add_library(stairs_led_module STATIC
        source/stairs_led_module.c
)

target_include_directories(stairs_led_module PUBLIC
        include
)

target_link_libraries(stairs_led_module PUBLIC
        i2c_driver
        logger
        task_scheduler
        inputs_board
        SmartHomeTypes
)

else ()

    add_subdirectory(test)

endif()