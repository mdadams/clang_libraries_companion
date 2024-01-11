cmake_minimum_required(VERSION 3.14)
project(dump_cfg LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(ClangFoo REQUIRED)
include(CheckStdFormat)
import_std_format()

add_executable(dump_cfg)
list(APPEND all_targets dump_cfg)
target_sources(dump_cfg PRIVATE main.cpp analyze.cpp)
target_link_libraries(dump_cfg PRIVATE ClangFoo::llvm ClangFoo::clangcpp
  Boost::filesystem)

add_library(dummy EXCLUDE_FROM_ALL
  data/example_1.cpp
  data/example_2.cpp
  #data/example_3.cpp
  data/example_4.cpp
  data/example_5.cpp
  )

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
