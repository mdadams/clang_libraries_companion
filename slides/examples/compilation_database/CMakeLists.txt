cmake_minimum_required(VERSION 3.14)

project(compilation_database LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(arg_adjuster adjuster.cpp utility.cpp)
list(APPEND all_targets arg_adjuster)
target_link_libraries(arg_adjuster PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

add_executable(fixed_database fixed.cpp utility.cpp)
list(APPEND all_targets fixed_database)
target_link_libraries(fixed_database PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

add_executable(json_database json.cpp utility.cpp)
list(APPEND all_targets json_database)
target_link_libraries(json_database PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
