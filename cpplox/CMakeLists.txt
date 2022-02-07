cmake_minimum_required(VERSION 3.14)

project(lox-language)

option(USE_ADDRESS_SANITIZER ON)
option(USE_UNDEFINED_SANITIZER OFF)

set(USE_ADDRESS_SANITIZER ON)

add_subdirectory(cpplox)
add_subdirectory(source)

# add_subdirectory(benchmark)

enable_testing()

add_subdirectory(tests)
