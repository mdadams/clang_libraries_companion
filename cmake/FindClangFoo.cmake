#[=[
################################################################################

The build process for the LLVM/Clang software is extremely flexible,
with many configuration parameters.  For this reason, the number of very
different ways in which an installation of this software can be configured
is practically limitless.  This makes using this software somewhat
painful, as it may exhibit many different modes of behavior, depending
on how it was configured at the time that it was built.  This leads to
a significant amount of boilerplate being needed in CMakeLists.txt files
for software using the LLVM/Clang libraries.

This CMake find module is an attempt to allow the LLVM and Clang packages
to be found more easily and used with less boilerplate.  This module
is primarily intended to be used with LLVM/Clang configurations that
employ a single shared library for each of the LLVM and Clang C++ (i.e.,
clang-cpp) library code.  This type of configuration is quite commonly
used by Linux distributions.  This CMake module may not work so well for
other kinds of configurations of LLVM/Clang.  The LLVM/Clang libraries
can be built and installed in so many ways that correctly handling all of
the possibilities in a clean and simple manner is likely not achievable.
Hopefully, this covers the more common cases reasonably well.

The inputs to this find module are as follows:

ClangFoo_PROPAGATE_ENABLE_RTTI
This is a boolean indicating if the enable-RTTI setting used to build
the LLVM/Clang libraries should be propagated to users of these libraries.

ClangFoo_PROPAGATE_ENABLE_EH
This is a boolean indicating if the enable-EH setting used to build the
LLVM/Clang libraries should be propagated to users of these libraries.

ClangFoo_USE_LLVM_COMPONENTS
This is a boolean indicating if the LLVM component libraries should
be used (instead of the single combined LLVM library).

ClangFoo_USE_CLANGCPP_COMPONENTS
This is a boolean indicating if the Clang C++ component libraries should
be used (instead of the single combined clang-cpp library).

Generally, it is recommended that the default values for the
above variables be used, unless this causes problems.

The following imported targets are provided:

  ClangFoo::llvm
  ClangFoo::clangcpp

Some other targets are also provided (which are deliberately
undocumented), but their use is discouraged.

Michael Adams
2024-01-21

################################################################################
#]=]

################################################################################
# Helper Macros and Functions
################################################################################

MACRO(ClangFoo_ASSIGN_TO_BOOL var)
	IF(${ARGN})
		SET(${var} TRUE)
	else()
		SET(${var} FALSE)
	ENDIF()
ENDMACRO()

FUNCTION(ClangFoo_MESSAGE)
	LIST(APPEND CMAKE_MESSAGE_INDENT "[ClangFoo] ")
	MESSAGE(${ARGN})
	LIST(POP_BACK CMAKE_MESSAGE_INDENT)
ENDFUNCTION()

################################################################################

##########
IF((NOT DEFINED ClangFoo_FOUND) OR (NOT ClangFoo_FOUND)) # GUARD
##########

################################################################################

IF(ClangFoo_FOUND)
	ClangFoo_MESSAGE(WARNING "ClangFoo package find operation performed multiple times")
ENDIF()

################################################################################

# The following is for debugging/testing:
#SET(ClangFoo_VERBOSE TRUE)
SET(ClangFoo_VERBOSE FALSE)

FIND_PACKAGE(LLVM CONFIG)
FIND_PACKAGE(Clang CONFIG)

# The following is only for testing:
#SET(LLVM_FOUND FALSE)
#SET(Clang_FOUND FALSE)

# The following is only for testing:
#SET(ClangFoo_USE_LLVM_COMPONENTS TRUE)
#SET(ClangFoo_USE_CLANGCPP_COMPONENTS FALSE)

################################################################################
# Print various information for debugging purposes.
################################################################################

ClangFoo_MESSAGE(STATUS "LLVM_VERSION: ${LLVM_VERSION}")

# Print the value of LLVM_LINK_LLVM_DYLIB for debugging.
IF(NOT DEFINED LLVM_LINK_LLVM_DYLIB)
	SET(ClangFoo_status "undefined")
ELSEIF(LLVM_LINK_LLVM_DYLIB)
	SET(ClangFoo_status "true")
ELSE()
	SET(ClangFoo_status "false")
ENDIF()
ClangFoo_MESSAGE(STATUS "LLVM_LINK_LLVM_DYLIB: ${ClangFoo_status}")

# Print the value of CLANG_LINK_CLANG_DYLIB for debugging.
IF(NOT DEFINED CLANG_LINK_CLANG_DYLIB)
	SET(ClangFoo_status "undefined")
ELSEIF(CLANG_LINK_CLANG_DYLIB)
	SET(ClangFoo_status "true")
ELSE()
	SET(ClangFoo_status "false")
ENDIF()
ClangFoo_MESSAGE(STATUS "CLANG_LINK_CLANG_DYLIB: ${ClangFoo_status}")

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_llvm_target TARGET LLVM)
ClangFoo_MESSAGE(STATUS "TARGET LLVM: ${ClangFoo_found_llvm_target}")

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_clangcpp_target TARGET clang-cpp)
ClangFoo_MESSAGE(STATUS "TARGET clang-cpp: ${ClangFoo_found_clangcpp_target}")

################################################################################
# Select default values for some key settings.
################################################################################

SET(ClangFoo_PROPAGATE_ENABLE_RTTI_DEFAULT TRUE)

SET(ClangFoo_PROPAGATE_ENABLE_EH_DEFAULT FALSE)

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_llvm_lib
  (TARGET LLVM) AND (DEFINED LLVM_LINK_LLVM_DYLIB) AND LLVM_LINK_LLVM_DYLIB)
ClangFoo_ASSIGN_TO_BOOL(ClangFoo_USE_LLVM_COMPONENTS_DEFAULT
  NOT ${ClangFoo_found_llvm_lib})

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_clangcpp_lib
  (TARGET clang-cpp) AND (DEFINED CLANG_LINK_CLANG_DYLIB) AND
  CLANG_LINK_CLANG_DYLIB)
ClangFoo_ASSIGN_TO_BOOL(ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT
  NOT ${ClangFoo_found_clangcpp_lib})

ClangFoo_MESSAGE(STATUS "found libLLVM: ${ClangFoo_found_llvm_lib}")
ClangFoo_MESSAGE(STATUS "found libclang-cpp: ${ClangFoo_found_clangcpp_lib}")

ClangFoo_MESSAGE(STATUS "ClangFoo_USE_LLVM_COMPONENTS_DEFAULT: "
  "${ClangFoo_USE_LLVM_COMPONENTS_DEFAULT}")
ClangFoo_MESSAGE(STATUS "ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT: "
  "${ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT}")

################################################################################
# Initialize some key variables with values taken from the environment
# if appropriate.
################################################################################

IF((NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS) AND
  (DEFINED ENV{ClangFoo_USE_LLVM_COMPONENTS}))
	SET(ClangFoo_USE_LLVM_COMPONENTS $ENV{ClangFoo_USE_LLVM_COMPONENTS})
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_USE_LLVM_COMPONENTS set from environment")
ENDIF()

IF((NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS) AND
  (DEFINED ENV{ClangFoo_USE_CLANGCPP_COMPONENTS}))
	SET(ClangFoo_USE_CLANGCPP_COMPONENTS
	  $ENV{ClangFoo_USE_CLANGCPP_COMPONENTS})
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_USE_CLANGCPP_COMPONENTS set from environment")
ENDIF()

IF((NOT DEFINED ClangFoo_PROPAGATE_ENABLE_RTTI) AND 
  (DEFINED ENV{ClangFoo_PROPAGATE_ENABLE_RTTI}))
	SET(ClangFoo_PROPAGATE_ENABLE_RTTI $ENV{ClangFoo_PROPAGATE_ENABLE_RTTI})
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_PROPAGATE_ENABLE_RTTI set from environment")
ENDIF()

IF((NOT DEFINED ClangFoo_PROPAGATE_ENABLE_EH) AND
  (DEFINED ENV{ClangFoo_PROPAGATE_ENABLE_EH}))
	SET(ClangFoo_PROPAGATE_ENABLE_EH $ENV{ClangFoo_PROPAGATE_ENABLE_EH})
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_PROPAGATE_ENABLE_EH set from environment")
ENDIF()

################################################################################
# Initialize some key variables to default values if appropriate.
################################################################################

IF(NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS)
	SET(ClangFoo_USE_LLVM_COMPONENTS ${ClangFoo_USE_LLVM_COMPONENTS_DEFAULT})
ENDIF()
ClangFoo_MESSAGE(STATUS
  "ClangFoo_USE_LLVM_COMPONENTS: ${ClangFoo_USE_LLVM_COMPONENTS}")

IF(NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS)
	SET(ClangFoo_USE_CLANGCPP_COMPONENTS
	  ${ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT})
ENDIF()
ClangFoo_MESSAGE(STATUS
  "ClangFoo_USE_CLANGCPP_COMPONENTS: ${ClangFoo_USE_CLANGCPP_COMPONENTS}")

IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_RTTI)
	SET(ClangFoo_PROPAGATE_ENABLE_RTTI
	  ${ClangFoo_PROPAGATE_ENABLE_RTTI_DEFAULT})
ENDIF()
ClangFoo_MESSAGE(STATUS
  "ClangFoo_PROPAGATE_ENABLE_RTTI: ${ClangFoo_PROPAGATE_ENABLE_RTTI}")

IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_EH)
	SET(ClangFoo_PROPAGATE_ENABLE_EH ${ClangFoo_PROPAGATE_ENABLE_EH_DEFAULT})
ENDIF()
ClangFoo_MESSAGE(STATUS
  "ClangFoo_PROPAGATE_ENABLE_EH: ${ClangFoo_PROPAGATE_ENABLE_EH}")

################################################################################

SET(ClangFoo_FOUND FALSE)

################################################################################

IF(ClangFoo_FIND_REQUIRED AND NOT (LLVM_FOUND AND Clang_FOUND))
	IF(NOT LLVM_FOUND)
		ClangFoo_MESSAGE(STATUS "LLVM package not found")
	ENDIF()
	IF(NOT Clang_FOUND)
		ClangFoo_MESSAGE(STATUS "Clang package not found")
	ENDIF()
	ClangFoo_MESSAGE(FATAL_ERROR "ClangFoo find failed")
ENDIF()

FIND_LIBRARY(LibEdit_LIBRARY NAMES edit)
IF (LibEdit_LIBRARY)
	SET(LibEdit_FOUND TRUE)
	# TODO/NOTE: Do we need to set the include directories?
	SET(LibEdit_INCLUDE_DIRS "")
	IF (NOT TARGET LibEdit::LibEdit)
		ADD_LIBRARY(LibEdit::LibEdit UNKNOWN IMPORTED)
	ENDIF()
	SET_TARGET_PROPERTIES(LibEdit::LibEdit PROPERTIES
	  IMPORTED_LOCATION ${LibEdit_LIBRARY}
	  #INTERFACE_INCLUDE_DIRECTORIES ${LibEdit_INCLUDE_DIRS}
	)
ELSE()
	SET(LibEdit_FOUND FALSE)
ENDIF()

FIND_PACKAGE(CURL)
IF(NOT CURL_FOUND)
	IF (NOT TARGET CURL::libcurl)
		ADD_LIBRARY(CURL::libcurl UNKNOWN IMPORTED)
	ENDIF()
	SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
	  IMPORTED_LOCATION ${CURL_LIBRARY}
	  #INTERFACE_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIRS}
	)
ENDIF()

IF(LLVM_FOUND AND Clang_FOUND)

	SET(clangfoo_options)
	# If LLVM/Clang was built with RTTI disabled, then (optionally) propagate
	# this choice.
	IF(ClangFoo_PROPAGATE_ENABLE_RTTI)
		IF(NOT LLVM_ENABLE_RTTI)
			# NOTE/TODO: This compiler option is GCC/Clang specific.
			SET(clangfoo_options ${clangfoo_options} -fno-rtti)
			IF (Clangfoo_VERBOSE)
				ClangFoo_MESSAGE(STATUS "disabling RTTI")
			ENDIF()
		ENDIF()
	ENDIF()
	# If LLVM/Clang was built with EH disabled, then (optionally) propagate
	# this choice.
	IF(ClangFoo_PROPAGATE_ENABLE_EH)
		IF(NOT LLVM_ENABLE_EH)
			# NOTE/TODO: This compiler option is GCC/Clang specific.
			SET(clangfoo_options ${clangfoo_options} -fno-exceptions)
			IF (Clangfoo_VERBOSE)
				ClangFoo_MESSAGE(STATUS "disabling EH")
			ENDIF()
		ENDIF()
	ENDIF()

	############################################################
	# Create ClangFoo::llvm::$x targets.
	############################################################

	# Create an imported target for each target exported from LLVM.
	# Importantly, attach key properties to these targets, such as
	# include directories, etc.
	SET(clangfoo_llvm_comps "")
	FOREACH(item ${LLVM_AVAILABLE_LIBS})

		IF(NOT TARGET ${item})
			ClangFoo_MESSAGE(STATUS "LLVM: skipping non-target ${item}")
			CONTINUE()
		ENDIF()

		IF(NOT TARGET ClangFoo::llvm::${item})
			ADD_LIBRARY(ClangFoo::llvm::${item} INTERFACE IMPORTED)
		ENDIF()
		SET_TARGET_PROPERTIES(ClangFoo::llvm::${item} PROPERTIES
		  INTERFACE_LINK_LIBRARIES ${item})
		TARGET_COMPILE_OPTIONS(ClangFoo::llvm::${item} INTERFACE
		  ${clangfoo_options})
		TARGET_COMPILE_DEFINITIONS(ClangFoo::llvm::${item} INTERFACE
		  ${LLVM_DEFINITIONS})
		TARGET_INCLUDE_DIRECTORIES(ClangFoo::llvm::${item} INTERFACE
		  ${LLVM_INCLUDE_DIRS})
		TARGET_LINK_DIRECTORIES(ClangFoo::llvm::${item} INTERFACE
		  ${LLVM_LIBRARY_DIRS})
		SET_TARGET_PROPERTIES(ClangFoo::llvm::${item} PROPERTIES
		  INTERFACE_LINK_LIBRARIES ${item})

		GET_TARGET_PROPERTY(ClangFoo_target_type ${item} TYPE)
		IF(item MATCHES "^LLVM[A-Z].*$")
		#IF(TRUE)
			SET(clangfoo_selected TRUE)
			IF(item STREQUAL "LLVMLineEditor" AND NOT LibEdit_FOUND)
				SET(clangfoo_selected FALSE)
			ENDIF()
		ELSE()
			SET(clangfoo_selected FALSE)
		ENDIF()
		IF(clangfoo_selected AND
		  (ClangFoo_target_type STREQUAL "STATIC_LIBRARY"))
			IF(ClangFoo_VERBOSE)
				ClangFoo_MESSAGE(STATUS "LLVM: found component ${item}")
			ENDIF()
			LIST(APPEND clangfoo_llvm_comps "${item}")
		ELSE()
			IF(ClangFoo_VERBOSE)
				ClangFoo_MESSAGE(STATUS "LLVM: skipping component ${item}")
			ENDIF()
		ENDIF()

	ENDFOREACH()

	IF(ClangFoo_VERBOSE)
		ClangFoo_MESSAGE(STATUS "LLVM components: ${clangfoo_llvm_comps}")
	ENDIF()
	LIST(LENGTH clangfoo_llvm_comps clangfoo_llvm_num_comps)
	#ClangFoo_ASSIGN_TO_BOOL(ClangFoo_USE_LLVM_COMPONENTS_DEFAULT
	#  clangfoo_llvm_num_comps GREATER 0)
	#IF(NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS)
	#	SET(ClangFoo_USE_LLVM_COMPONENTS
	#	  ${ClangFoo_USE_LLVM_COMPONENTS_DEFAULT})
	#ENDIF()

	############################################################
	# Create ClangFoo::llvm_components target.
	############################################################

	IF(ClangFoo_USE_LLVM_COMPONENTS AND (clangfoo_llvm_num_comps EQUAL 0))
		ClangFoo_MESSAGE(WARNING
		  "forcing ClangFoo_USE_LLVM_COMPONENTS to FALSE")
		SET(ClangFoo_USE_LLVM_COMPONENTS FALSE)
	ENDIF()

	IF(ClangFoo_USE_LLVM_COMPONENTS)
		IF(NOT TARGET ClangFoo::llvm_components)
			ADD_LIBRARY(ClangFoo::llvm_components INTERFACE IMPORTED)
		ENDIF()
		TARGET_COMPILE_OPTIONS(ClangFoo::llvm_components INTERFACE
		  ${clangfoo_options})
		TARGET_COMPILE_DEFINITIONS(ClangFoo::llvm_components INTERFACE
		  ${LLVM_DEFINITIONS})
		TARGET_INCLUDE_DIRECTORIES(ClangFoo::llvm_components INTERFACE
		  ${LLVM_INCLUDE_DIRS})
		TARGET_LINK_DIRECTORIES(ClangFoo::llvm_components INTERFACE
		  ${LLVM_LIBRARY_DIRS})
		SET_TARGET_PROPERTIES(ClangFoo::llvm_components PROPERTIES
		  INTERFACE_LINK_LIBRARIES "${clangfoo_llvm_comps}"
		)
	endif()

	############################################################
	# Create ClangFoo::llvm as an alias for either ClangFoo::llvm::LLVM or
	# ClangFoo::llvm_components.
	############################################################

	IF(NOT TARGET ClangFoo::llvm)
		IF(ClangFoo_USE_LLVM_COMPONENTS)
			IF(NOT TARGET ClangFoo::llvm_components)
				ClangFoo_MESSAGE(FATAL_ERROR "yikes")
			ENDIF()
			ADD_LIBRARY(ClangFoo::llvm ALIAS ClangFoo::llvm_components)
		ELSEIF(ClangFoo_found_llvm_lib)
			ADD_LIBRARY(ClangFoo::llvm ALIAS ClangFoo::llvm::LLVM)
		ELSE()
			ClangFoo_MESSAGE(FATAL_ERROR "no LLVM library")
		ENDIF()
	ENDIF()

	############################################################
	# Create ClangFoo::clang::$x targets.
	############################################################

	# Create an imported target for each target exported from Clang.
	# Importantly, attach key properties to these targets, such as
	# include directories, etc.
	SET(clangfoo_clangcpp_comps "")
	FOREACH(item ${CLANG_EXPORTED_TARGETS})

		IF(NOT TARGET ${item})
			CONTINUE()
		ENDIF()

		IF(NOT TARGET ClangFoo::clang::${item})
			ADD_LIBRARY(ClangFoo::clang::${item} INTERFACE IMPORTED)
		ENDIF()
		SET_TARGET_PROPERTIES(ClangFoo::clang::${item} PROPERTIES
		  INTERFACE_LINK_LIBRARIES ${item})
		TARGET_COMPILE_OPTIONS(ClangFoo::clang::${item} INTERFACE
		  ${clangfoo_options})
		TARGET_COMPILE_DEFINITIONS(ClangFoo::clang::${item} INTERFACE
		  ${LLVM_DEFINITIONS})
		TARGET_INCLUDE_DIRECTORIES(ClangFoo::clang::${item} INTERFACE
		  ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
		TARGET_LINK_DIRECTORIES(ClangFoo::clang::${item} INTERFACE
		  ${LLVM_LIBRARY_DIRS})
		SET_TARGET_PROPERTIES(ClangFoo::clang::${item} PROPERTIES
		  INTERFACE_LINK_LIBRARIES ${item})

		GET_TARGET_PROPERTY(ClangFoo_target_type ${item} TYPE)
		SET(clangfoo_selected FALSE)
		IF(item MATCHES "^clang[A-Z].*$")
			SET(clangfoo_selected TRUE)
		ENDIF()
		IF(clangfoo_selected AND
		  (ClangFoo_target_type STREQUAL "STATIC_LIBRARY"))
			IF(ClangFoo_VERBOSE)
				ClangFoo_MESSAGE(STATUS "clangcpp: found component ${item}")
			ENDIF()
			LIST(APPEND clangfoo_clangcpp_comps ${item})
		ELSE()
			IF(ClangFoo_VERBOSE)
				ClangFoo_MESSAGE(STATUS "clangcpp: skipping component ${item}")
			ENDIF()
		ENDIF()

	ENDFOREACH()

	IF(ClangFoo_VERBOSE)
		ClangFoo_MESSAGE(STATUS
		  "clangcpp components: ${clangfoo_clangcpp_comps}")
	ENDIF()
	LIST(LENGTH clangfoo_clangcpp_comps clangfoo_clangcpp_num_comps)
	#ClangFoo_ASSIGN_TO_BOOL(ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT
	#  clangfoo_clangcpp_num_comps GREATER 0)
	#IF(NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS)
	#	SET(ClangFoo_USE_CLANGCPP_COMPONENTS
	#	  ${ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT})
	#ENDIF()

	############################################################
	# Create ClangFoo::clangcpp_components target.
	############################################################

	IF(ClangFoo_USE_CLANGCPP_COMPONENTS AND
	  (clangfoo_clangcpp_num_comps EQUAL 0))
		ClangFoo_MESSAGE(WARNING
		  "forcing ClangFoo_USE_CLANGCPP_COMPONENTS to FALSE")
		SET(ClangFoo_USE_CLANGCPP_COMPONENTS FALSE)
	ENDIF()

	IF(ClangFoo_USE_CLANGCPP_COMPONENTS)
		IF(NOT TARGET ClangFoo::clangcpp_components)
			ADD_LIBRARY(ClangFoo::clangcpp_components INTERFACE IMPORTED)
		ENDIF()
		SET_TARGET_PROPERTIES(ClangFoo::clangcpp_components PROPERTIES
		  INTERFACE_LINK_LIBRARIES "${clangfoo_clangcpp_comps}")
	endif()

	############################################################
	# Create ClangFoo::clangcpp as an alias for either
	# ClangFoo::clang::clang-cpp or ClangFoo::clangcpp_components.
	############################################################

	IF(NOT TARGET ClangFoo::clangcpp)
		IF(ClangFoo_USE_CLANGCPP_COMPONENTS)
			IF(NOT TARGET ClangFoo::clangcpp_components)
				ClangFoo_MESSAGE(FATAL_ERROR "yikes")
			ENDIF()
			ADD_LIBRARY(ClangFoo::clangcpp ALIAS ClangFoo::clangcpp_components)
		ELSEIF(ClangFoo_found_clangcpp_lib)
			ADD_LIBRARY(ClangFoo::clangcpp ALIAS ClangFoo::clang::clang-cpp)
		ELSE()
			ClangFoo_MESSAGE(FATAL_ERROR "no clangcpp library")
		ENDIF()
	ENDIF()

	############################################################
	# Done.
	############################################################

	SET(ClangFoo_FOUND TRUE)

endif()

################################################################################

##########
ELSE()
ClangFoo_MESSAGE(WARNING
  "ClangFoo package find operation performed multiple times")
ENDIF() # GUARD
##########
