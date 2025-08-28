cmake_minimum_required(VERSION 3.14)

project(cast_1 LANGUAGES CXX C)

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

add_executable(app)
if(LLVM_MAJOR_VERSION GREATER_EQUAL 21)
	set(app_sources main.cpp)
else()
	set(app_sources clang-20/main.cpp)
endif()
target_sources(app PRIVATE ${app_sources})

target_link_libraries(app PRIVATE ClangFoo::llvm ClangFoo::clangcpp)
list(APPEND all_targets app)

set(test_sources
  data/example_1.cpp
  )
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})
set_source_files_properties(${test_sources}
  APPEND PROPERTIES COMPILE_FLAGS "-frtti")

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
