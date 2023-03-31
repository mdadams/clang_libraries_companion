cmake_minimum_required(VERSION 3.14)
project(mangle_1 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ClangSetup)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Fmt)
if(TARGET Fmt::Fmt)
	link_libraries(Fmt::Fmt)
endif()

set(LLVM_LINK_COMPONENTS support)

llvm_map_components_to_libnames(llvm_libs support)
cl_get_clang_libs(clang_libs)

# If Clang was built with RTTI disabled, then disable RTTI (to avoid linker
# errors caused by undefined references related to typeinfo).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

list(APPEND all_targets tool)
add_executable(tool)
target_sources(tool PRIVATE main.cpp)
target_link_libraries(tool PRIVATE ${llvm_libs} ${clang_libs}
  clangAST
  clangASTMatchers
  clangBasic
  clangDynamicASTMatchers
  clangFrontend
  clangTooling
  clangSerialization
)
target_include_directories(tool PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(tool PRIVATE ${LLVM_DEFINITIONS})

set(test_sources
	data/example_1.cpp
	data/example_2.cpp
)
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
