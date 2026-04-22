add_library(flight STATIC
    pid.c
    mixer.c
    attitude.c
)

target_include_directories(flight PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(flight PUBLIC sensors)
target_compile_options(flight PRIVATE -Wall -Wextra -fno-math-errno)
target_link_libraries(flight PUBLIC m)
