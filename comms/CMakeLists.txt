add_library(comms STATIC
    mavlink_tx.c
    mavlink_rx.c
    hil.c
)

target_include_directories(comms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(comms PUBLIC
    MAVLINK
    flight
    sensors
)
