if (NOT UNIT_TESTS)

add_library(notification_manager STATIC
        source/notification_manager.c
)

target_include_directories(notification_manager PUBLIC
        include
)

target_link_libraries(notification_manager PUBLIC
		wifi_manager
		
)

else ()

    add_subdirectory(test)

endif()