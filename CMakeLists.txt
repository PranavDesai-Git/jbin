cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)
project(jbin VERSION 1.0.0 LANGUAGES CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)


FetchContent_Declare(
    zstd
    URL https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz
    SOURCE_SUBDIR build/cmake
)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(zstd)


# Create our main executable
add_executable(jbin main.cpp)

target_link_libraries(jbin PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(jbin PRIVATE libzstd_static)
