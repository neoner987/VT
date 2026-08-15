cmake_minimum_required(VERSION 4.1)
project(VT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(VT main.cpp
        Tokenization.hpp
        Parser.hpp
        Generation.hpp
        arena.hpp)