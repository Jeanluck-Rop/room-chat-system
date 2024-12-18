cmake_minimum_required(VERSION 3.10)
project(room-chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/tests)
