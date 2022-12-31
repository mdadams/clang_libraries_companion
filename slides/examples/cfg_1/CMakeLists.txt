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

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

option(USE_FMTLIB "Use fmtlib for std::format support." false)
if(USE_FMTLIB)
	find_package(Fmt REQUIRED)
	link_libraries(Fmt::Fmt)
endif()

if(ENABLE_EXTRAS)
	add_library(dummy EXCLUDE_FROM_ALL test_1.cpp test_2.cpp)
endif()

llvm_map_components_to_libnames(llvm_libs all)

# If Clang was built with RTTI disabled, then disable RTTI (to avoid linker
# errors caused by undefined references related to typeinfo).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_executable(cfg main.cpp)
list(APPEND all_targets cfg)
target_link_libraries(cfg PRIVATE ${llvm_libs} clangTooling libclang)
target_include_directories(cfg PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(cfg PRIVATE ${LLVM_DEFINITIONS})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
