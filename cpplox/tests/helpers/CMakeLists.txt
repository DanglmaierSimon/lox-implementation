add_library(testlib SHARED testhelper.cpp testhelper.h)
target_link_libraries(testlib PRIVATE lox fmt::fmt-header-only)
target_include_directories(testlib PUBLIC .)