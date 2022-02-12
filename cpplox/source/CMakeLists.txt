cmake_minimum_required(VERSION 3.14)

find_package(fmt 8 REQUIRED)

set(HEADERS
    ${CMAKE_SOURCE_DIR}/include/lox/common.h
    ${CMAKE_SOURCE_DIR}/include/lox/chunk.h
    ${CMAKE_SOURCE_DIR}/include/lox/memory.h
    ${CMAKE_SOURCE_DIR}/include/lox/value.h
    ${CMAKE_SOURCE_DIR}/include/lox/debug.h
    ${CMAKE_SOURCE_DIR}/include/lox/vm.h
    ${CMAKE_SOURCE_DIR}/include/lox/compiler.h
    ${CMAKE_SOURCE_DIR}/include/lox/scanner.h
    ${CMAKE_SOURCE_DIR}/include/lox/table.h
    ${CMAKE_SOURCE_DIR}/include/lox/parser.h    
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objboundmethod.h 
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objclass.h 
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objclosure.h 
    ${CMAKE_SOURCE_DIR}/include/lox/objects/obj.h  
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objfunction.h  
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objinstance.h 
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objnative.h  
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objstring.h  
    ${CMAKE_SOURCE_DIR}/include/lox/objects/objupvalue.h
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
    objects/objboundmethod.cpp
    objects/objclass.cpp
    objects/objclosure.cpp
    objects/obj.cpp
    objects/objfunction.cpp
    objects/objinstance.cpp
    objects/objnative.cpp
    objects/objstring.cpp
    objects/objupvalue.cpp
)


add_library(lox STATIC ${HEADERS} ${SOURCES})
set_target_properties(lox PROPERTIES PUBLIC_HEADER "${HEADERS}")
target_include_directories(lox PUBLIC ../include)

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