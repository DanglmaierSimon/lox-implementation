cmake_minimum_required(VERSION 3.14)

include(FetchContent)

project(lox-language)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS true)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(SANITIZER "" CACHE STRING "Sanitizer to use. Valid values are 'address', 'undefined' or ''.")
option(BUILD_LOX_TESTS "Build lox tests." ON)

if("${SANITIZER}" STREQUAL "undefined")
    message(STATUS "Using undefined behaviour sanitizer!")
    set(SANITIZER_LINK_FLAGS -fsanitize=undefined)
    set(SANITIZER_COMPILE_FLAGS -fsanitize=undefined -fno-omit-frame-pointer)
elseif("${SANITIZER}" STREQUAL "address")
    message(STATUS "Using address sanitizer!")
    set(SANITIZER_LINK_FLAGS -fsanitize=address)
    set(SANITIZER_COMPILE_FLAGS -fsanitize=address -fno-omit-frame-pointer)
elseif("${SANITIZER}" STREQUAL "")
    message(STATUS "Not using sanitizers!")
    set(SANITIZER_LINK_FLAGS "")
    set(SANITIZER_COMPILE_FLAGS "")
else()
    message(SEND_ERROR "Unrecognised sanitizer: ${SANITIZER}")
endif()

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

find_package(Threads REQUIRED) # for pthread

add_subdirectory(src)

if(BUILD_LOX_TESTS)
    include(CTest)
    message(STATUS "Lox tests enabled.")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Lox tests disabled.")
endif()