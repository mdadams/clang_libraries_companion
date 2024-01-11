cmake_minimum_required(VERSION 3.14)

project(frontend_action_2 LANGUAGES CXX C)

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

add_executable(frontend_action main.cpp)
list(APPEND all_targets frontend_action)
target_link_libraries(frontend_action PRIVATE ClangFoo::llvm ClangFoo::clangcpp)

add_executable(example_1_cxx data/example_1.cpp)
foreach(i 98 11 14 17)
	add_executable(example_1_cxx${i} data/example_1.cpp)
	set_property(TARGET example_1_cxx${i} PROPERTY CXX_STANDARD ${i})
endforeach()

add_executable(example_2_c data/example_2.c)
foreach(i 90 99 11 17)
	add_executable(example_2_c${i} data/example_2.c)
	set_property(TARGET example_2_c${i} PROPERTY C_STANDARD ${i})
endforeach()

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
