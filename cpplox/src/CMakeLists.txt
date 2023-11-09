cmake_minimum_required(VERSION 3.14)

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
    parser.h
    token.h
    objboundmethod.h
    objclass.h
    objclosure.h
    obj.h
    objfunction.h
    objinstance.h
    objnative.h
    objstring.h
    objupvalue.h
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
    parser.cpp
    token.cpp
    objboundmethod.cpp
    objclass.cpp
    objclosure.cpp
    obj.cpp
    objfunction.cpp
    objinstance.cpp
    objnative.cpp
    objstring.cpp
    objupvalue.cpp
)

add_library(lox SHARED ${LOX_LIB_HEADERS} ${LOX_LIB_SOURCES})

target_compile_options(lox PUBLIC
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wfloat-equal
    -Wundef
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wwrite-strings
    -Wcast-qual
    -Wunreachable-code
    -Wformat=2
    -Winit-self
    -Wimplicit-fallthrough
    -Wempty-body
)

target_link_libraries(lox PRIVATE fmt::fmt-header-only)

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
    -Wimplicit-fallthrough
    -Wempty-body
)

target_link_libraries(cpplox PUBLIC lox)
