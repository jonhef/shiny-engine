cmake_minimum_required(VERSION 3.20)
project(shiny-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(shiny-engine 
    src/main.cpp
    src/position/position.cpp

    src/fen/fen.cpp

    src/evaluation/evaluation.cpp

    src/searching/zobrist.cpp 
    src/searching/tt.cpp 
    src/searching/pvs.cpp
    src/searching/searching.cpp
)

target_include_directories(shiny-engine PRIVATE src)