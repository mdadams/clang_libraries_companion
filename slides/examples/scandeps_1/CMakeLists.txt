cmake_minimum_required(VERSION 3.14)
project(scandeps_1 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ParseVersion)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(ClangFoo REQUIRED)
parse_version_string("${LLVM_VERSION}" LLVM_MAJOR_VERSION LLVM_MINOR_VERSION
  LLVM_PATCH_VERSION)
include(CheckStdFormat)
import_std_format()

if(LLVM_MAJOR_VERSION GREATER_EQUAL 22)
	add_executable(scandeps)
	target_sources(scandeps PRIVATE main.cpp)
	target_link_libraries(scandeps PRIVATE ClangFoo::llvm ClangFoo::clangcpp)
	list(APPEND all_targets scandeps)

	configure_file("${CMAKE_SOURCE_DIR}/demo"
	  "${CMAKE_BINARY_DIR}/demo" @ONLY)
	add_custom_target(demo DEPENDS ${all_targets}
	  COMMAND "${CMAKE_BINARY_DIR}/demo")
else()
	add_custom_target(demo)
endif()
