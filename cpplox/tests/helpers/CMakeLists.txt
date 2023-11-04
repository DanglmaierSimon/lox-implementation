add_library(testlib STATIC testhelper.cpp testhelper.h)
target_link_libraries(testlib PUBLIC lox fmt gtest gmock pthread)

# target_compile_options(testlib PUBLIC -fPIC)
target_include_directories(testlib PUBLIC . ${CMAKE_SOURCE_DIR}/src)