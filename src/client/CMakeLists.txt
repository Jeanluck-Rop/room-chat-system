cmake_minimum_required(VERSION 3.10)

# Divide client from server for a better handle of each one
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

# Add the headers in the include/ directory for the compiltation/executation
include_directories(include)

# GTK/libadwaita flags
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Include directories and libraries for gtk use
include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER})

# Path to XML resources
set(RESOURCE_XML ${CMAKE_CURRENT_SOURCE_DIR}/gui/resources.gresource.xml)
set(RESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/resources.c)

# Generate the resources.c file
add_custom_command(
  OUTPUT ${RESOURCE_C}
  COMMAND glib-compile-resources
  --generate-source
  --target=${RESOURCE_C}
  --sourcedir ${CMAKE_CURRENT_SOURCE_DIR}/gui/resources
  ${RESOURCE_XML}
  DEPENDS ${RESOURCE_XML}
)

# Auxiliar target to ensure the resources.c generate before
add_custom_target(resources_target DEPENDS ${RESOURCE_C})

# Source code files in C
set(C_SOURCES
  gui/view.c
  ${RESOURCE_C}
)

# Source code files in C++
set(CPP_SOURCES
  src/main.cpp
  src/client.cpp
  src/message.cpp
  src/controller.cpp
  src/wrapper_controller.cpp
  src/idle.c
)

# Create static library to compile the view in C
add_library(gui_c STATIC ${C_SOURCES})

# Create static library to compile the client in C++
add_library(client_library STATIC ${CPP_SOURCES})

# Ensure gui generates after get the resources.c
add_dependencies(gui_c resources_target)

# Create main executable
add_executable(client src/main.cpp ${RESOURCE_C})

# Ensure the executable get the resources.c
add_dependencies(client resources_target)

# Link static library with executable
target_link_libraries(client OpenSSL::SSL OpenSSL::Crypto gui_c client_library ${GTK4_LIBRARIES})
