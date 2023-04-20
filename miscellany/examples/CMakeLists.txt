cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(examples LANGUAGES CXX)

option(ENABLE_ASAN "Enable ASan" TRUE)
option(ENABLE_UBSAN "Enable UBSan" TRUE)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
#set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(ExternalProject)
include(FetchContent)

################################################################################

set(cmake_args "")
list(APPEND cmake_args "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
if(CMAKE_VERBOSE_MAKEFILE)
	list(APPEND cmake_args
	  "-DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}")
endif()
if(CMAKE_EXPORT_COMPILE_COMMANDS)
	list(APPEND cmake_args
	  "-DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}")
endif()
list(APPEND cmake_args
  "-DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}")
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

ExternalProject_Add(cal
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/cal"
  BINARY_DIR "${CMAKE_BINARY_DIR}/cal"
  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  STEP_TARGETS configure build install
  CMAKE_ARGS ${cmake_args} ${other_cmake_args}
  )

################################################################################

# The top-level directories of the projects to be processed (i.e., built).
list(APPEND project_dirs
  ast_from_string
  ast_matcher_10
  ast_visitor_10
  linter
  preproc_macros_1
  headers
  query_dependencies
  #ast_matcher_11
  #dump_tokens
  #test_ast_visitor_1
  template_10
  template_11
  attribute_2
  type_1
  test_ast_matcher_0
  dump_type
  lib_use_analyzer
  mangle_1
)

list(APPEND installable_project_dirs
  lib_use_analyzer
  query_dependencies
)

#message("cmake_args ${cmake_args}")

add_custom_target(configure)
add_custom_target(build)
add_custom_target(demo)
add_custom_target(install-subprojects)

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

	list(FIND installable_project_dirs ${dir} find_index)
	if(NOT find_index EQUAL -1)
		message("enabling install target for target ${target}")
		set(has_install_target TRUE)
	else()
		message("disabling install target for target ${target}")
		set(has_install_target FALSE)
	endif()

	if (has_install_target)
		set(install_command
		  ${CMAKE_COMMAND} --build <BINARY_DIR> --target install)
		set(install_target_flag TRUE)
	else()
		set(install_command ${CMAKE_COMMAND} -E echo)
	endif()

	# Add external project.
	ExternalProject_Add("${target}"
	  SOURCE_DIR "${CMAKE_SOURCE_DIR}/${dir}"
	  BINARY_DIR "${CMAKE_BINARY_DIR}/${target}"
	  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
	  STEP_TARGETS configure build install demo
	  CMAKE_ARGS ${cmake_args} ${other_cmake_args}
	  INSTALL_COMMAND ${install_command}
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
	if (has_install_target)
		add_dependencies(install-subprojects ${target}-install)
	endif()
	add_dependencies(${target}-configure cal-install)

endforeach()
