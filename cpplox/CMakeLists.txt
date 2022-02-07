cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    cpplox
    VERSION 0.1.0
    DESCRIPTION "Cpp implementation of jlox tree walking interpreter"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/coverage.cmake)

# ---- Declare executable ----

add_executable(cpplox 
  source/main.cpp
  source/common.h
  source/chunk.h
  source/chunk.cpp
  source/memory.h
  source/memory.cpp
  source/debug.h
  source/debug.cpp
  source/value.h
  source/value.cpp
  source/vm.h
  source/vm.cpp
  source/compiler.h
  source/compiler.cpp
  source/scanner.h
  source/scanner.cpp
  source/object.h
  source/object.cpp
  source/table.h
  source/table.cpp
)

set_target_properties(
    cpplox PROPERTIES
    OUTPUT_NAME cpplox
    EXPORT_NAME exe
)

target_compile_features(cpplox PRIVATE cxx_std_17)
target_compile_options(cpplox PRIVATE -Wall -Wextra -Werror -pedantic -std=c++17 -fsanitize=address -Wshadow -Wuninitialized -fno-omit-frame-pointer -Wno-c++98-compat -Wno-unused -Wno-c99-designator -Wimplicit-fallthrough -Wcovered-switch-default -Wsometimes-uninitialized -Wempty-body -Wno-c99-extensions)
target_link_options(cpplox PRIVATE -fno-omit-frame-pointer -fsanitize=address)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT cpplox_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of cpplox"
  )
endif()

include(cmake/dev-mode.cmake)

