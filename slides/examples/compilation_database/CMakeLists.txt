cmake_minimum_required(VERSION 3.14)

project(compilation_database LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

option(USE_FMTLIB "Use fmtlib for std::format support." false)
if(USE_FMTLIB)
	find_package(Fmt REQUIRED)
	link_libraries(Fmt::Fmt)
endif()

llvm_map_components_to_libnames(llvm_libs all)

# If Clang was built with RTTI disabled, then disable RTTI (to avoid linker
# errors caused by undefined references related to typeinfo).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_executable(arg_adjuster adjuster.cpp utility.cpp)
list(APPEND all_targets arg_adjuster)
#target_sources(arg_adjuster PRIVATE adjuster.cpp utility.cpp)
target_link_libraries(arg_adjuster PRIVATE ${llvm_libs} clangTooling libclang)
target_include_directories(arg_adjuster PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(arg_adjuster PRIVATE ${LLVM_DEFINITIONS})

add_executable(fixed_database fixed.cpp utility.cpp)
list(APPEND all_targets fixed_database)
target_link_libraries(fixed_database PRIVATE ${llvm_libs} clangTooling libclang)
target_include_directories(fixed_database PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(fixed_database PRIVATE ${LLVM_DEFINITIONS})

add_executable(json_database json.cpp utility.cpp)
list(APPEND all_targets json_database)
target_link_libraries(json_database PRIVATE ${llvm_libs} clangTooling libclang)
target_include_directories(json_database PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(json_database PRIVATE ${LLVM_DEFINITIONS})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
