cmake_minimum_required(VERSION 3.14)

# copy lox test files to build directory
file(GLOB_RECURSE
    LOX_TEST_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS

    ${CMAKE_CURRENT_SOURCE_DIR}/test/*
)

FOREACH(TEST_FILE ${LOX_TEST_FILES})
    message(STATUS ${TEST_FILE})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE}
    )
ENDFOREACH()

list(TRANSFORM LOX_TEST_FILES PREPEND ${CMAKE_CURRENT_BINARY_DIR}/)

add_executable(test-cpplox test-cpplox.cpp ${LOX_TEST_FILES})
target_link_libraries(test-cpplox PUBLIC lox GTest::gtest_main pthread fmt)
target_include_directories(test-cpplox PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(test-cpplox PUBLIC
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wshadow
    -Wuninitialized
    -Wimplicit-fallthrough
    -Wempty-body
)

include(GoogleTest)
gtest_discover_tests(test-cpplox)
