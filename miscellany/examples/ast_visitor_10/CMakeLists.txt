cmake_minimum_required(VERSION 3.14)

project(ast_visitor_10 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(app)
list(APPEND all_targets app)
target_sources(app PRIVATE main.cpp utilities.cpp)
target_link_libraries(app PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

set(test_sources
	data/example_1.cpp
	data/example_2.cpp
	data/example_3.cpp
	data/example_4.cpp
)
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
