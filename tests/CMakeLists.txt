cmake_minimum_required(VERSION 3.21)
project(drone_tests C)

include(FetchContent)
include(CTest)

# Unity test framework
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(unity)

# flight/ compiled as a static lib for host.
# It only depends on sensors/ for type definitions — no platform headers.
add_subdirectory(../sensors ${CMAKE_BINARY_DIR}/sensors)
add_subdirectory(../flight  ${CMAKE_BINARY_DIR}/flight)

function(add_drone_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE unity flight)
    target_compile_options(${name} PRIVATE -Wall -Wextra)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_drone_test(test_pid   test_pid.c)
add_drone_test(test_mixer test_mixer.c)
