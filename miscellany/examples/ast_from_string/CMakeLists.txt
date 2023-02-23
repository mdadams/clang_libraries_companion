cmake_minimum_required(VERSION 3.14)
project(ast_from_string LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ClangSetup)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
#set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS filesystem)
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

add_executable(ast_from_string)
list(APPEND all_targets ast_from_string)
target_sources(ast_from_string PRIVATE main.cpp)
target_link_libraries(ast_from_string PRIVATE ${llvm_libs} ${clang_libs}
  Boost::filesystem)
target_include_directories(ast_from_string PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(ast_from_string PRIVATE ${LLVM_DEFINITIONS})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
