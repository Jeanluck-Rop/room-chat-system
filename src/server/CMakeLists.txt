add_library(server)

target_sources(server PRIVATE src/cJSON.c)

target_include_directories(server PUBLIC include)

target_compile_definitions(server PRIVATE -DCJSON_MEM_REDUCED=1)
