cmake_minimum_required(VERSION 3.10)

# Divide client from server for a better handle of each one
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
  
# Add the headers in the include/ directory for the compiltation/executation
include_directories(include)

# GTK/libadwaita flags
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(ADWAITA REQUIRED libadwaita-1)

include_directories(${GTK4_INCLUDE_DIRS} ${ADWAITA_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS} ${ADWAITA_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER} ${ADWAITA_CFLAGS_OTHER})

# Source code files
set(C_SOURCES
    gui/view.c
)
  
set(CPP_SOURCES
    src/main.cpp
    src/client.cpp
    src/message.cpp
    gui/view.cpp
)

# Create static library to compile the view in C
add_library(gui_c STATIC ${C_SOURCES})

# Create static library to compile the client in C++
add_library(client_library STATIC ${CPP_SOURCES})

# Create main executable
add_executable(client src/main.cpp)

# Link static library with executable
target_link_libraries(client client_library gui_c ${GTK4_LIBRARIES} ${ADWAITA_LIBRARIES})
