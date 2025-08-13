cmake_minimum_required(VERSION 3.10)

# Divide server from client for a better handle of each one
project(server)

set(CMAKE_C_STANDARD 99)

find_package(OpenSSL REQUIRED)

# Add the headers in the include/ directory for the compiltation/executation
include_directories(include)

# Code files
set(SOURCES
  src/main.c
  src/message.c
  src/server.c
  src/room.c
  src/cJSON.c
)

# Create static library 
add_library(server_library ${SOURCES})

target_link_libraries(server_library PRIVATE OpenSSL::SSL OpenSSL::Crypto pthread)

# Create main executable
add_executable(server src/main.c)

# Link static library with executable
target_link_libraries(server server_library OpenSSL::SSL OpenSSL::Crypto pthread)
