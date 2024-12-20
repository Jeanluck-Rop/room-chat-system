cmake_minimum_required(VERSION 3.10)
project(client_tests)

include_directories(${CMAKE_SOURCE_DIR}/client/include)

set(TEST_SOURCES
    client_test.cpp
    room_test.cpp
)

add_executable(client_tests ${TEST_SOURCES})

target_link_libraries(client_tests client_lib)
