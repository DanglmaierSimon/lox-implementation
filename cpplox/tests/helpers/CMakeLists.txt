add_library(testlib STATIC testhelper.cpp testhelper.h)
target_link_libraries(testlib PRIVATE lox fmt::fmt-header-only gtest gmock)
target_include_directories(testlib PUBLIC .)