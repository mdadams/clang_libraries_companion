cmake_minimum_required(VERSION 3.14)
project(dump_cfg LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

option(USE_FMTLIB "Use fmtlib for std::format support." false)
if(USE_FMTLIB)
	find_package(Fmt REQUIRED)
	link_libraries(Fmt::Fmt)
endif()

set(LLVM_LINK_COMPONENTS support)

llvm_map_components_to_libnames(llvm_libs support)

# If Clang was built with RTTI disabled, then disable RTTI (to avoid linker
# errors caused by undefined references related to typeinfo).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_executable(dump_cfg)
list(APPEND all_targets dump_cfg)
target_sources(dump_cfg PRIVATE main.cpp)
target_link_libraries(dump_cfg PRIVATE ${llvm_libs} clangTooling
  libclang Boost::filesystem)
target_include_directories(dump_cfg PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(dump_cfg PRIVATE ${LLVM_DEFINITIONS})

set(test_sources
  data/example_1.cpp
  data/example_2.cpp
  data/example_3.cpp
  data/example_4.cpp
  data/example_5.cpp
)
add_executable(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
