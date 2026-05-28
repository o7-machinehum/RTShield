cmake_minimum_required(VERSION 3.16)
project(rtshield_python CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

add_subdirectory(../cpp cpp_build)
pybind11_add_module(rtshield bindings.cpp)
target_link_libraries(rtshield PRIVATE rtshield_cpp)
target_compile_options(rtshield PRIVATE -Wall -Wextra)
