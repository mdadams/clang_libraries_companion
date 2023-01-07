cmake_minimum_required(VERSION 3.14)
project(ast_matcher_10 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ClangSetup)

option("ENABLE_EXPERIMENTAL" "Enable experimental code" FALSE)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
#set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(CAL REQUIRED)
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

add_executable(matcher)
list(APPEND all_targets matcher)
target_sources(matcher PRIVATE main.cpp clang_utility.cpp)
if(ENABLE_EXPERIMENTAL)
	target_sources(matcher PRIVATE clang_experimental.cpp)
	target_compile_definitions(matcher PRIVATE ENABLE_EXPERIMENTAL)
endif()

target_link_libraries(matcher PRIVATE ${llvm_libs} ${clang_libs}
  Boost::filesystem CAL::CAL)
target_include_directories(matcher PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(matcher PRIVATE ${LLVM_DEFINITIONS})

set(test_sources
  data/empty.cpp
  data/standard_headers.cpp
  data/example_1.cpp
  data/example_2.cpp
  data/example_3.cpp
  data/example_4.cpp
  data/example_5.cpp
  data/example_6.cpp
  data/example_7.cpp
  data/example_10.cpp
  data/example_11.cpp
  data/example_12.cpp
  data/example_13.cpp
  data/example_15.cpp
  data/example_16.cpp
  data/example_17.cpp
  data/example_18.cpp
  data/example_19.cpp
)
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
