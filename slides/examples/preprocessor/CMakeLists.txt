cmake_minimum_required(VERSION 3.14)

project(preprocessor LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ParseVersion)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#find_package(LLVM REQUIRED CONFIG)
find_package(ClangFoo REQUIRED)
parse_version_string("${LLVM_VERSION}" LLVM_MAJOR_VERSION LLVM_MINOR_VERSION
  LLVM_PATCH_VERSION)
include(CheckStdFormat)
import_std_format()

if(LLVM_MAJOR_VERSION GREATER_EQUAL 20)
	set(sources clang-20/main.cpp)
elseif(LLVM_MAJOR_VERSION GREATER_EQUAL 16)
	set(sources clang-16/main.cpp)
else()
	set(sources clang-15/main.cpp)
endif()
add_executable(preproc ${sources})
list(APPEND all_targets preproc)
target_link_libraries(preproc PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

set(test_sources
  data/test_1.cpp
  data/test_2.cpp
)
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
