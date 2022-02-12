cmake_minimum_required(VERSION 3.14)
project(benchmarks)

# set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

#
# add_executable(cpploxbenchmark benchmark.cpp)

# target_link_libraries(cpploxbenchmark PUBLIC libcpplox benchmark::benchmark benchmark::benchmark_main)

# target_compile_options(cpploxbenchmark PUBLIC -Wall -Wextra -Werror -pedantic -std=c++17 -Wshadow -Wuninitialized -Wno-c++98-compat -Wno-unused -Wno-c99-designator -Wimplicit-fallthrough -Wcovered-switch-default -Wsometimes-uninitialized -Wempty-body -Wno-c99-extensions -O3 -Ofast)