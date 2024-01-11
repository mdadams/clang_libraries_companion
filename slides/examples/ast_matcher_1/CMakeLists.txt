cmake_minimum_required(VERSION 3.14)
project(ast_matcher_1 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(matcher)
list(APPEND all_targets matcher)
target_sources(matcher PRIVATE main.cpp utilities.cpp)

target_link_libraries(matcher PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

add_executable(example_1 data/example_1.cpp)
add_executable(example_2 data/example_2.cpp)

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
