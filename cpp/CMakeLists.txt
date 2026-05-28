cmake_minimum_required(VERSION 3.16)
project(rtshield_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rtshield_cpp STATIC src/rtshield.cpp)
target_include_directories(rtshield_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(rtshield_cpp PRIVATE -Wall -Wextra)
