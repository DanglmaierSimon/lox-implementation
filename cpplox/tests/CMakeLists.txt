cmake_minimum_required(VERSION 3.14)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)

set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)

FetchContent_MakeAvailable(googletest)

add_subdirectory(helpers)

function(register_tests TEST_NAME)
    message(STATUS "Adding test ${TEST_NAME}...")
    set(SOURCE autogen/${TEST_NAME}.cpp)
    add_executable(${TEST_NAME} ${SOURCE})

    target_compile_options(${TEST_NAME} PRIVATE     
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
    -Wno-c99-extensions)

    target_link_libraries(${TEST_NAME} PUBLIC lox testlib gtest_main gmock)

    if (${USE_ADDRESS_SANITIZER})
        target_compile_options(${TEST_NAME}  PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_libraries(${TEST_NAME}  PRIVATE -fsanitize=address)  
    endif()

    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endfunction()

set(TESTS
test_block
test_call 
test_closure 
test_constructor
test_field
test_function
test_inheritance
test_logical_operator
test_misc
test_number  
test_print     
test_return  
test_string
test_this    
test_while
test_assignment
test_bool 
test_class
test_comments
# test_expressions # Only useful for early chapters
test_for  
test_if      
test_limit      
test_method          
test_nil 
test_operator
test_regression
# test_scanning # only useful for early chapters
test_super 
test_variable
)

foreach(test ${TESTS})
    register_tests(${test})
endforeach()

include(GoogleTest)

add_executable(test_scanner test_scanner.cpp)
target_link_libraries(test_scanner PUBLIC lox testlib gtest_main gmock)

if (${USE_ADDRESS_SANITIZER})
    target_compile_options(test_scanner  PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(test_scanner  PRIVATE -fsanitize=address)  
endif()

gtest_discover_tests(test_scanner)

