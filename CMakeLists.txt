cmake_minimum_required(VERSION 3.12)
project(shiny-engine)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(shiny-engine 
    src/main.cpp

    src/evaluation/evaluation.h
    src/evaluation/evaluation.cpp

    src/searching/alpha_beta.cpp
    src/searching/alpha_beta.h

    src/utils/board.cpp 
    src/utils/board.h 
    src/utils/chess_logic.cpp 
    src/utils/chess_logic.h 
    src/utils/fen_utils.cpp 
    src/utils/fen_utils.h 
    src/utils/figures.h 
    src/utils/position.cpp 
    src/utils/position.h 
    src/utils/ratio.cpp 
    src/utils/ratio.h
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O1")

set(CMAKE_CXX_FLAGS_LINKER "${CMAKE_CXX_FLAGS_LINKER} -stack_size=268435456")

target_include_directories(shiny-engine PRIVATE src)