cmake_minimum_required(VERSION 3.14)
project(ast_matcher_0 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_library(trivial trivial.cpp)
target_link_libraries(trivial PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

add_custom_target(demo)
