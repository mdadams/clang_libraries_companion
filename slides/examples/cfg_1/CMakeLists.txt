cmake_minimum_required(VERSION 3.14)

project(cfg_1 LANGUAGES CXX C)

option(ENABLE_EXTRAS "Enable extras." FALSE)

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

if(ENABLE_EXTRAS)
	add_library(dummy EXCLUDE_FROM_ALL test_1.cpp test_2.cpp)
endif()

add_executable(cfg main.cpp)
list(APPEND all_targets cfg)
target_link_libraries(cfg PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
