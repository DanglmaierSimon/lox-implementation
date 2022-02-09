cmake_minimum_required(VERSION 3.14)

project(lox-language)

if ("${SANITIZER}" STREQUAL "undefined")
    message(STATUS "Using undefined behaviour sanitizer!")
    set(SANITIZER_LINK_FLAGS -fsanitize=undefined)
    set(SANITIZER_COMPILE_FLAGS -fsanitize=undefined -fno-omit-frame-pointer)
elseif("${SANITIZER}" STREQUAL "address")
    message(STATUS "Using address sanitizer!")
    set(SANITIZER_LINK_FLAGS -fsanitize=address)
    set(SANITIZER_COMPILE_FLAGS -fsanitize=address -fno-omit-frame-pointer)
elseif ("${SANITIZER}" STREQUAL "")
    message(STATUS "Not using sanitizers!")
    set(SANITIZER_LINK_FLAGS "")
    set(SANITIZER_COMPILE_FLAGS "")
else()
    message(SEND_ERROR "Unrecognised sanitizer: ${SANITIZER}")
endif()

add_subdirectory(cpplox)
add_subdirectory(source)

# add_subdirectory(benchmark)

enable_testing()

add_subdirectory(tests)
