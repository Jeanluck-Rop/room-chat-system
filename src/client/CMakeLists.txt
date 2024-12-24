cmake_minimum_required(VERSION 3.10)

# Divide client from server for a better handle of each one
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
  
# Add the headers in the include/ directory for the compiltation/executation
include_directories(include)

# Code files
set(SOURCES
    src/main.cpp
    src/client.cpp
)

# Create static library 
add_library(client_library ${SOURCES})

# Create main executable
add_executable(client src/main.cpp)

# Link static library with executable
target_link_libraries(client client_library)

# Paths to headers, we use PRIVATE to handl header access precisely and prevent propagation of include paths.
# target_include_directories(client_library PRIVATE ${CMAKE_SOURCE_DIR}/src/client/include)
# target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/src/client/include)

