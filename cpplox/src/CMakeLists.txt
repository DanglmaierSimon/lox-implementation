cmake_minimum_required(VERSION 3.30)

set(LOX_LIB_HEADERS
    common.h
    chunk.h
    memory.h
    value.h
    debug.h
    vm.h
    compiler.h
    scanner.h
    table.h
    object.h
    token.h
    parser.h
)

set(LOX_LIB_SOURCES
    chunk.cpp
    memory.cpp
    debug.cpp
    value.cpp
    vm.cpp
    compiler.cpp
    scanner.cpp
    table.cpp
    object.cpp
    parser.cpp
)

add_library(lox SHARED ${LOX_LIB_HEADERS} ${LOX_LIB_SOURCES})
set_target_properties(lox PROPERTIES PUBLIC_HEADER "${LOX_LIB_HEADERS}")

target_compile_options(lox PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic
    -g
    -Wundef
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wwrite-strings
    -Wcast-qual
    -Wunreachable-code
    -Winit-self
    -Wimplicit-fallthrough
    -Wempty-body
    -Wno-c99-extensions
)

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
    -Wshadow
    -Wuninitialized
    -Wno-c++98-compat
    -Wno-unused
    -Wno-c99-designator
    -Wimplicit-fallthrough
    -Wempty-body
    -Wno-c99-extensions
    -g
)

target_link_libraries(cpplox PUBLIC lox)