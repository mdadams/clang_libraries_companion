cmake_minimum_required(VERSION 3.14)

project(cplusplus LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

option(USE_FMTLIB "Use fmtlib for std::format support." false)
if(USE_FMTLIB)
	find_package(Fmt REQUIRED)
	link_libraries(Fmt::Fmt)
endif()

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

set(lib_sources
	simple_1.cpp
	templates_1.cpp
	macros_1.cpp
	class_template_1.cpp
	class_template_2.cpp
	function_template_1.cpp
	variable_template_1.cpp
)

add_library(dummy EXCLUDE_FROM_ALL ${lib_sources})

add_executable(hello hello.cpp)
add_executable(invalid_1 EXCLUDE_FROM_ALL invalid_1.cpp)

add_custom_target(demo)
