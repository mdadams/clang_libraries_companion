cmake_minimum_required(VERSION 3.14)

project(dump_ast_3 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(dump_ast main.cpp clang_utility_1.cpp)
list(APPEND all_targets dump_ast)
target_link_libraries(dump_ast PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

add_executable(tree_formatter TreeFormatter.cpp)
list(APPEND all_targets tree_formatter)
target_link_libraries(tree_formatter PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
