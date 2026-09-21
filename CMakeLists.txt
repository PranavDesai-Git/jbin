cmake_minimum_required(VERSION 3.20)
project(jbin VERSION 1.0.0 LANGUAGES CXX)

# Enforce Modern C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Useful for IDEs and LSP

# We'll add our dependencies (nlohmann_json and zstd) here soon!

# Create our main executable
add_executable(jbin main.cpp)

# We will link our libraries down here
