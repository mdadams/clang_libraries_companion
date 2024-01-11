cmake_minimum_required(VERSION 3.14)
project(ast_from_string LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
#set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(ast_from_string)
list(APPEND all_targets ast_from_string)
target_sources(ast_from_string PRIVATE main.cpp)
target_link_libraries(ast_from_string PRIVATE ClangFoo::llvm ClangFoo::clangcpp
  Boost::filesystem)

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
