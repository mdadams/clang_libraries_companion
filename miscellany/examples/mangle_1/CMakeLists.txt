cmake_minimum_required(VERSION 3.14)
project(mangle_1 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ParseVersion)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(ClangFoo REQUIRED)
parse_version_string("${LLVM_VERSION}" LLVM_MAJOR_VERSION LLVM_MINOR_VERSION
  LLVM_PATCH_VERSION)
include(CheckStdFormat)
import_std_format()

list(APPEND all_targets tool)
add_executable(tool)
target_sources(tool PRIVATE main.cpp)
target_link_libraries(tool PRIVATE ClangFoo::llvm ClangFoo::clangcpp)
target_compile_definitions(tool PRIVATE LLVM_MAJOR_VERSION=${LLVM_MAJOR_VERSION})

set(test_sources
	data/example_1.cpp
	data/example_2.cpp
	data/example_3.cpp
	data/example_3.c
)
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
