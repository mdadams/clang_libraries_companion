cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(examples LANGUAGES CXX)

option(ENABLE_ASAN "Enable ASan" TRUE)
option(ENABLE_UBSAN "Enable UBSan" TRUE)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
#set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(ExternalProject)
#include(FetchContent)

################################################################################

set(cmake_args "")

list(APPEND cmake_args "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
if(DEFINED CMAKE_VERBOSE_MAKEFILE)
	list(APPEND cmake_args
	  "-DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}")
endif()
#if(DEFINED CMAKE_EXPORT_COMPILE_COMMANDS)
	list(APPEND cmake_args
	  "-DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}")
#endif()
if(CMAKE_CXX_COMPILER)
	list(APPEND cmake_args "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
endif()
if(CMAKE_C_COMPILER)
	list(APPEND cmake_args "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
endif()

list(APPEND other_cmake_args "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
list(APPEND other_cmake_args "-DENABLE_ASAN=${ENABLE_ASAN}")
list(APPEND other_cmake_args "-DENABLE_UBSAN=${ENABLE_UBSAN}")

################################################################################

# The top-level directories of the projects to be processed (i.e., built).
list(APPEND project_dirs
  ast_consumer_1
  ast_matcher_0
  ast_matcher_1
  ast_matcher_2
  ast_visitor_1
  ast_visitor_2
  ast_visitor_3
  ast_visitor_matcher_1
  cfg_1
  #clang_ast
  #clang_query
  clang_utilities
  command_line
  compilation_database
  cpp
  cyclomatic_complexity
  diagnostic_consumer
  dump_cfg
  frontend_action
  frontend_action_2
  liveness_analysis
  #llvm_clang_usage
  preprocessor
  template_1
  attribute_1
  #ast_matcher_3
  cast_1
  ast_matcher_4
  ast_matcher_5
)

message("cmake_args ${cmake_args}")

add_custom_target(configure)
add_custom_target(build)
add_custom_target(demo)

# Add each project as an external project.
foreach(dir IN LISTS project_dirs)

	# Skip the processing of the directory if it does not exist.
	if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${dir}")
		message(WARNING "Skipping directory ${dir}")
		continue()
	endif()

	# Set target name to directory name with any
	# spaces changed to underscores.
	set(target "${dir}")
	string(REPLACE " " "_" target "${target}")
	string(REPLACE "/" "__" target "${target}")

	# Add external project.
	ExternalProject_Add("${target}"
	  SOURCE_DIR "${CMAKE_SOURCE_DIR}/${dir}"
	  BINARY_DIR "${CMAKE_BINARY_DIR}/${target}"
	  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
	  STEP_TARGETS configure build install demo
	  CMAKE_ARGS ${cmake_args} ${other_cmake_args}
	  INSTALL_COMMAND ""
	  )
	ExternalProject_Add_Step("${target}" demo
	  ALWAYS TRUE
	  DEPENDEES build
	  EXCLUDE_FROM_MAIN TRUE
	  EXCLUDE_FROM_ALL TRUE
	  WORKING_DIRECTORY <BINARY_DIR>
	  COMMENT "Running demo"
	  COMMAND echo ==================================================
	  COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config <CONFIG> --target demo
	  COMMAND echo ==================================================
	  )
	add_dependencies(configure ${target}-configure)
	add_dependencies(build ${target}-build)
	add_dependencies(demo ${target}-demo)

endforeach()
