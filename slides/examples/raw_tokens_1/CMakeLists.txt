cmake_minimum_required(VERSION 3.14)
project(raw_tokens_1 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(tool)
target_sources(tool PRIVATE main.cpp)

target_link_libraries(tool PRIVATE ClangFoo::llvm ClangFoo::clangcpp)
list(APPEND all_targets tool)

set(test_sources
  data/example_1.cpp
  )
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
