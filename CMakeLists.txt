cmake_minimum_required(VERSION 3.10)
project(ToyC_Parser)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(compiler parser.cpp)
