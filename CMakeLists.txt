cmake_minimum_required(VERSION 3.20)
project(shiny-engine LANGUAGES CXX)

# Указываем только для C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Явно задаём компиляторы из Homebrew LLVM
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

# Настройки сборки
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Цель
add_executable(shiny-engine 
    src/main.cpp
    src/position/position.cpp
    src/fen/fen.cpp
    src/evaluation/evaluation.cpp
)

target_include_directories(shiny-engine PRIVATE src)
