cmake_minimum_required(VERSION 3.30)

project(lox-language)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS true)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(SANITIZER "" CACHE STRING "Sanitizer to use. Valid values are 'address', 'undefined' or ''.")

add_subdirectory(src)
