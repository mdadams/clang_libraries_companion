cmake_minimum_required(VERSION 3.14)
project(ast_matcher_0 LANGUAGES CXX C)

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

add_library(trivial trivial.cpp)
target_include_directories(trivial PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(trivial PRIVATE ${LLVM_DEFINITIONS})

add_custom_target(demo)
