cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(cpplox 
  main.cpp
)

set_target_properties(
    cpplox PROPERTIES
    OUTPUT_NAME cpplox
)

target_compile_options(cpplox PUBLIC
    -Wall
    -Wextra
    -Werror
    -pedantic
    -std=c++17
    -Wshadow
    -Wuninitialized
    -Wno-c++98-compat
    -Wno-unused
    -Wno-c99-designator
    -Wimplicit-fallthrough
    -Wempty-body
    -Wno-c99-extensions 
    -g
    ${SANITIZER_COMPILE_FLAGS}
)

target_link_libraries(cpplox PUBLIC lox)

target_link_options(cpplox PRIVATE ${SANITIZER_LINK_FLAGS})

if (${USE_ADDRESS_SANITIZER})
  target_compile_options(cpplox PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_libraries(cpplox PRIVATE -fsanitize=address)  
endif()

