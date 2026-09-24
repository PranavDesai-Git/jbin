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
    cxxopts
    URL https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.1.1.tar.gz
)

FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.tar.gz
)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(cxxopts)
FetchContent_MakeAvailable(Catch2)

enable_testing()


# 1. Automatically find all .cpp files in the src/ directory
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# 2. Create the core library
add_library(jbin_core STATIC ${LIB_SOURCES})
target_include_directories(jbin_core PUBLIC include)
target_link_libraries(jbin_core PUBLIC nlohmann_json::nlohmann_json)

# 3. Create the main executable
add_executable(jbin src/main.cpp)
target_link_libraries(jbin PRIVATE jbin_core cxxopts::cxxopts)

# 4. Create the test executable
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")
if (TEST_SOURCES)
    add_executable(jbin_tests ${TEST_SOURCES})
    target_link_libraries(jbin_tests PRIVATE jbin_core Catch2::Catch2WithMain)
    
    include(Catch)
    catch_discover_tests(jbin_tests)
endif()
