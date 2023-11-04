cmake_minimum_required(VERSION 3.14)

find_package(fmt 8 CONFIG REQUIRED)

set(HEADERS
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

set(SOURCES
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

add_library(lox STATIC ${HEADERS} ${SOURCES})
set_target_properties(lox PROPERTIES PUBLIC_HEADER "${HEADERS}")

target_compile_options(lox PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic
    -std=c++17
    -g
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
    ${SANITIZER_COMPILE_FLAGS}
)

target_link_options(lox PRIVATE ${SANITIZER_LINK_FLAGS})

target_compile_features(lox PUBLIC cxx_std_17)

target_link_libraries(lox PUBLIC fmt)

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

if(${USE_ADDRESS_SANITIZER})
    target_compile_options(cpplox PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(cpplox PRIVATE -fsanitize=address)
endif()
