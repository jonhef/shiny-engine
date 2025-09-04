cmake_minimum_required(VERSION 3.12)
project(shiny-engine)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(shiny-engine src/main.cpp)
