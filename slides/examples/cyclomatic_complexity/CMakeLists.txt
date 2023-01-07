cmake_minimum_required(VERSION 3.14)

project(cyclomatic_complexity LANGUAGES CXX C)

option(ENABLE_EXTRAS "Enable extras." FALSE)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ClangSetup)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Fmt)
if(TARGET Fmt::Fmt)
	link_libraries(Fmt::Fmt)
endif()

if(ENABLE_EXTRAS)
	add_library(dummy EXCLUDE_FROM_ALL test_1.cpp test_2.cpp test_3.cpp)
endif()

llvm_map_components_to_libnames(llvm_libs all)
cl_get_clang_libs(clang_libs)

# If Clang was built with RTTI disabled, then disable RTTI (to avoid linker
# errors caused by undefined references related to typeinfo).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_executable(cyclomatic_complexity_matcher matcher.cpp)
list(APPEND all_targets cyclomatic_complexity_matcher)
target_link_libraries(cyclomatic_complexity_matcher
  PRIVATE ${llvm_libs} ${clang_libs})
target_include_directories(cyclomatic_complexity_matcher
  PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(cyclomatic_complexity_matcher
  PRIVATE ${LLVM_DEFINITIONS})

add_executable(cyclomatic_complexity_visitor visitor.cpp)
list(APPEND all_targets cyclomatic_complexity_visitor)
target_link_libraries(cyclomatic_complexity_visitor
  PRIVATE ${llvm_libs} ${clang_libs})
target_include_directories(cyclomatic_complexity_visitor
  PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(cyclomatic_complexity_visitor
  PRIVATE ${LLVM_DEFINITIONS})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
