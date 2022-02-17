cmake_minimum_required(VERSION 3.16.3)
project(gps-system)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(gps_reader src/gps_reader.c src/gpslib.c src/Primary.c)
target_link_libraries(gps_reader m)

add_executable(can_listener src/can_listener.c src/Primary.c)

add_custom_target(test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gps_reader /dev/ttyACM0
    DEPENDS gps_reader
    COMMENT "Test GPS reader")

add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/can_listener
    DEPENDS can_listener gps_reader
    COMMENT "Run all!")
