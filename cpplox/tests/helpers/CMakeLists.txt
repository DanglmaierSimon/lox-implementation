add_library(testlib SHARED testhelper.cpp testhelper.h)
target_compile_options(testlib PRIVATE -fPIC)
target_link_libraries(testlib PRIVATE lox fmt::fmt-header-only gtest gmock)
target_include_directories(testlib PUBLIC .)