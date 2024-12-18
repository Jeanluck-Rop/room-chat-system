add_library(client)

target_include_directories(client PUBLIC include)

find_package(nlohmann_json REQUIRED)

target_link_libraries(client PUBLIC nlohmann_json::nlohmann_json)
