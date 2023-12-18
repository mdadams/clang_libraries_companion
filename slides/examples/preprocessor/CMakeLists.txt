cmake_minimum_required(VERSION 3.14)

project(preprocessor LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)
include(ClangSetup)
include(ParseVersion)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(LLVM REQUIRED CONFIG)
parse_version_string("${LLVM_VERSION}" LLVM_MAJOR_VERSION LLVM_MINOR_VERSION
  LLVM_PATCH_VERSION)
find_package(Clang REQUIRED CONFIG)
find_package(Fmt)
if(TARGET Fmt::Fmt)
	link_libraries(Fmt::Fmt)
endif()

llvm_map_components_to_libnames(llvm_libs core)
cl_get_clang_libs(clang_libs)
#list(APPEND clang_libs clang-cpp)
#list(PREPEND clang_libs ${llvm_libs})
#list(APPEND clang_libs clang-cpp)

# If LLVM/Clang was built with RTTI disabled, then disable RTTI.
if(NOT LLVM_ENABLE_RTTI)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

if(LLVM_MAJOR_VERSION GREATER_EQUAL 16)
	set(sources clang16/main.cpp)
else()
	set(sources clang15/main.cpp)
endif()
add_executable(preproc ${sources})
list(APPEND all_targets preproc)
target_link_libraries(preproc PRIVATE ${llvm_libs} ${clang_libs})
target_include_directories(preproc PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(preproc PRIVATE ${LLVM_DEFINITIONS})

set(test_sources
  data/test_1.cpp
  data/test_2.cpp
)
add_library(dummy EXCLUDE_FROM_ALL ${test_sources})

configure_file("${CMAKE_SOURCE_DIR}/demo"
  "${CMAKE_BINARY_DIR}/demo" @ONLY)
add_custom_target(demo DEPENDS ${all_targets}
  COMMAND "${CMAKE_BINARY_DIR}/demo")
