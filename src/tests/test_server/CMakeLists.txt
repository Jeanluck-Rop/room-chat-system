cmake_minimum_required(VERSION 3.10)
project(server_tests)

# Agregamos el directorio de encabezados
include_directories(${CMAKE_SOURCE_DIR}/server/include)

# Archivos fuente de las pruebas unitarias:
set(TEST_SOURCES
    server_test.c
    room_test.c
)

# Creamos el ejecutable de las pruebas unitarias para el servidor.
add_executable(server_tests ${TEST_SOURCES})

# Vinculamos el ejecutable de las pruebas unitarias con el código del servidor.
target_link_libraries(server_tests server_lib)
