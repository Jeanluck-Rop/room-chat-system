cmake_minimun_requeried(VERSION 3.10)

project(server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
  src/main.c
)

add_library(server_library ${SOURCES})

add_executable(server src/main.c)

target_link_libraries(server server_library)

target_include_directories(server_lib PRIVATE ${CMAKE_SOURSE_DIR}/src/server/include)

target_compile_definitions(server PRIVATE ${CMAKE_SOURCE_DIR}/src/server/include)
