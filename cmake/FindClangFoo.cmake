#[=[
################################################################################

After much pain and suffering, I have concluded that one or more of the
following assertions must be true:

  1) The LLVM/Clang CMake config files do not work very well for building
  code outside the LLVM source tree.

  2) Some LLVM/Clang packages for some Unix/Linux variants are broken.
  It is possible that LLVM/Clang might be at fault in some cases.

  3) I am having some basic misunderstandings about CMake and/or how
  one can correctly build LLVM/Clang code outside the LLVM source tree.

I suspect that there may be an element of truth to all three of the
preceding items.  This CMake find module is intended to help in dealing
with some of the issues that have arisen as a consequence of the items
listed above.

This CMake find module is an attempt to allow the LLVM and Clang packages
to be found more easily and used with less boilerplate.  This module seems
to work reasonably well in the environments in which it has been tested.
These environments tend to use a single shared library for each of the
LLVM and Clang C++ (i.e., clang-cpp) library code.  How well this works
for other configurations is somewhat of an open question.  There are just
so many ways in which the LLVM/Clang libraries can be built and installed.
It is doubtful that anyone could ever handle all of the possibilities
correctly.  Hopefully, this covers the more common cases reasonably well.

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

The following imported targets are provided:

  ClangFoo::llvm
  ClangFoo::clangcpp

Some other targets are also provided, but their use is discouraged.

Michael Adams
2024-01-08

################################################################################
#]=]

################################################################################

IF(ClangFoo_FOUND)
	MESSAGE(WARNING "ClangFoo package find operation performed multiple times")
ENDIF()

################################################################################

SET(ClangFoo_VERBOSE FALSE)

FIND_PACKAGE(LLVM CONFIG)
FIND_PACKAGE(Clang CONFIG)

# The following is only for testing:
#SET(LLVM_FOUND FALSE)
#SET(Clang_FOUND FALSE)

# The following is only for testing:
#SET(ClangFoo_USE_LLVM_COMPONENTS TRUE)
#SET(ClangFoo_USE_CLANGCPP_COMPONENTS FALSE)

MESSAGE(STATUS "LLVM_VERSION: ${LLVM_VERSION}")

SET(clangfoo_found_llvm_lib FALSE)
IF(NOT DEFINED LLVM_LINK_LLVM_DYLIB)
	SET(clangfoo_llvm_link_llvm_dylib_status "undefined")
ELSEIF(LLVM_LINK_LLVM_DYLIB)
	SET(clangfoo_llvm_link_llvm_dylib_status "1")
	IF(TARGET LLVM)
		SET(clangfoo_found_llvm_lib TRUE)
	ENDIF()
ELSE()
	SET(clangfoo_llvm_link_llvm_dylib_status "0")
ENDIF()
MESSAGE(STATUS
  "LLVM_LINK_LLVM_DYLIB: ${clangfoo_llvm_link_llvm_dylib_status}")

SET(clangfoo_found_clangcpp_lib FALSE)
IF(NOT DEFINED CLANG_LINK_CLANG_DYLIB)
	SET(clangfoo_clang_link_clang_dylib_status "undefined")
ELSEIF(CLANG_LINK_CLANG_DYLIB)
	SET(clangfoo_clang_link_clang_dylib_status "1")
	IF(TARGET clang-cpp)
		SET(clangfoo_found_clangcpp_lib TRUE)
	ENDIF()
ELSE()
	SET(clangfoo_clang_link_clang_dylib_status "0")
ENDIF()
MESSAGE(STATUS
  "CLANG_LINK_CLANG_DYLIB: ${clangfoo_clang_link_clang_dylib_status}")
MESSAGE(STATUS "found LLVM: ${clangfoo_found_llvm_lib}")
MESSAGE(STATUS "found clang-cpp: ${clangfoo_found_clangcpp_lib}")

SET(clangfoo_found_clangcpp_target FALSE)
IF(TARGET clang-cpp)
	SET(clangfoo_found_clangcpp_target TRUE)
ENDIF()
SET(clangfoo_found_llvm_target FALSE)
IF(TARGET LLVM)
	SET(clangfoo_found_llvm_target TRUE)
ENDIF()
MESSAGE(STATUS "TARGET clang-cpp: ${clangfoo_found_clangcpp_target}")
MESSAGE(STATUS "TARGET LLVM: ${clangfoo_found_llvm_target}")

IF(NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS)
	IF(DEFINED ENV{ClangFoo_USE_LLVM_COMPONENTS})
		SET(ClangFoo_USE_LLVM_COMPONENTS $ENV{ClangFoo_USE_LLVM_COMPONENTS})
		MESSAGE(STATUS "ClangFoo_USE_LLVM_COMPONENTS set from environment")
	ENDIF()
ENDIF()
IF(NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS)
	IF(DEFINED ENV{ClangFoo_USE_CLANGCPP_COMPONENTS})
		SET(ClangFoo_USE_CLANGCPP_COMPONENTS
		  $ENV{ClangFoo_USE_CLANGCPP_COMPONENTS})
		MESSAGE(STATUS "ClangFoo_USE_CLANGCPP_COMPONENTS set from environment")
	ENDIF()
ENDIF()

IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_RTTI)
	IF(DEFINED ENV{ClangFoo_PROPAGATE_ENABLE_RTTI})
		SET(ClangFoo_PROPAGATE_ENABLE_RTTI $ENV{ClangFoo_PROPAGATE_ENABLE_RTTI})
		MESSAGE(STATUS "ClangFoo_PROPAGATE_ENABLE_RTTI set from environment")
	ENDIF()
ENDIF()
IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_EH)
	IF(DEFINED ENV{ClangFoo_PROPAGATE_ENABLE_EH})
		SET(ClangFoo_PROPAGATE_ENABLE_EH $ENV{ClangFoo_PROPAGATE_ENABLE_EH})
		MESSAGE(STATUS "ClangFoo_PROPAGATE_ENABLE_EH set from environment")
	ENDIF()
ENDIF()

SET(ClangFoo_USE_LLVM_COMPONENTS_DEFAULT FALSE)
SET(ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT FALSE)

IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_RTTI)
	SET(ClangFoo_PROPAGATE_ENABLE_RTTI TRUE)
ENDIF()
MESSAGE(STATUS
  "ClangFoo_PROPAGATE_ENABLE_RTTI: ${ClangFoo_PROPAGATE_ENABLE_RTTI}")
IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_EH)
	SET(ClangFoo_PROPAGATE_ENABLE_EH FALSE)
ENDIF()
MESSAGE(STATUS "ClangFoo_PROPAGATE_ENABLE_EH: ${ClangFoo_PROPAGATE_ENABLE_EH}")

################################################################################

IF(NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS)
	SET(ClangFoo_USE_LLVM_COMPONENTS ${ClangFoo_USE_LLVM_COMPONENTS_DEFAULT})
ENDIF()
MESSAGE(STATUS "ClangFoo_USE_LLVM_COMPONENTS: ${ClangFoo_USE_LLVM_COMPONENTS}")
IF(NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS)
	SET(ClangFoo_USE_CLANGCPP_COMPONENTS ${ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT})
ENDIF()
MESSAGE(STATUS
  "ClangFoo_USE_CLANGCPP_COMPONENTS: ${ClangFoo_USE_CLANGCPP_COMPONENTS}")

################################################################################

SET(ClangFoo_FOUND FALSE)

################################################################################

IF(ClangFoo_FIND_REQUIRED AND NOT (LLVM_FOUND AND Clang_FOUND))
	IF(NOT LLVM_FOUND)
		MESSAGE(STATUS "LLVM not found")
	ENDIF()
	IF(NOT Clang_FOUND)
		MESSAGE(STATUS "Clang not found")
	ENDIF()
	MESSAGE(FATAL_ERROR "ClangFoo not found")
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
			SET(clangfoo_options ${clangfoo_options} -fno-rtti)
			IF (Clangfoo_VERBOSE)
				MESSAGE(STATUS "disabling RTTI")
			ENDIF()
		ENDIF()
	ENDIF()
	# If LLVM/Clang was built with EH disabled, then (optionally) propagate
	# this choice.
	IF(ClangFoo_PROPAGATE_ENABLE_EH)
		IF(NOT LLVM_ENABLE_EH)
			SET(clangfoo_options ${clangfoo_options} -fno-exceptions)
			IF (Clangfoo_VERBOSE)
				MESSAGE(STATUS "disabling EH")
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
			MESSAGE(STATUS "LLVM: skipping non-target ${item}")
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

		GET_TARGET_PROPERTY(TARGET_TYPE ${item} TYPE)
		SET(clangfoo_selected TRUE)
		IF(item STREQUAL "LLVMLineEditor" AND NOT LibEdit_FOUND)
			SET(clangfoo_selected FALSE)
		ENDIF()
		IF(clangfoo_selected AND (TARGET_TYPE STREQUAL "STATIC_LIBRARY"))
			IF(ClangFoo_VERBOSE)
				MESSAGE(STATUS "LLVM: found component ${item}")
			ENDIF()
			LIST(APPEND clangfoo_llvm_comps ${item})
		ENDIF()

	ENDFOREACH()
	LIST(LENGTH clangfoo_llvm_comps clangfoo_llvm_num_comps)

	############################################################
	# Create ClangFoo::llvm_components target.
	############################################################

	IF(clangfoo_llvm_num_comps EQUAL 0)
		MESSAGE(WARNING "forcing ClangFoo_USE_LLVM_COMPONENTS to FALSE")
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
			ADD_LIBRARY(ClangFoo::llvm ALIAS ClangFoo::llvm_components)
		ELSEIF(clangfoo_found_llvm_lib)
			ADD_LIBRARY(ClangFoo::llvm ALIAS ClangFoo::llvm::LLVM)
		ELSE()
			MESSAGE(FATAL_ERROR "no LLVM library")
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

		GET_TARGET_PROPERTY(TARGET_TYPE ${item} TYPE)
		SET(clangfoo_selected FALSE)
		IF(item MATCHES "^clang[A-Z].*$")
			SET(clangfoo_selected TRUE)
		ENDIF()
		IF(clangfoo_selected AND (TARGET_TYPE STREQUAL "STATIC_LIBRARY"))
			IF(ClangFoo_VERBOSE)
				MESSAGE(STATUS "clangcpp: found component ${item}")
			ENDIF()
			LIST(APPEND clangfoo_clangcpp_comps ${item})
		ENDIF()

	ENDFOREACH()
	LIST(LENGTH clangfoo_clangcpp_comps clangfoo_clangcpp_num_comps)

	############################################################
	# Create ClangFoo::clangcpp_components target.
	############################################################

	IF(clangfoo_clangcpp_num_comps EQUAL 0)
		MESSAGE(WARNING "forcing ClangFoo_USE_CLANGCPP_COMPONENTS to FALSE")
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
			ADD_LIBRARY(ClangFoo::clangcpp ALIAS ClangFoo::clangcpp_components)
		ELSEIF(clangfoo_found_clangcpp_lib)
			ADD_LIBRARY(ClangFoo::clangcpp ALIAS ClangFoo::clang::clang-cpp)
		ELSE()
			MESSAGE(FATAL_ERROR "no clangcpp library")
		ENDIF()
	ENDIF()

	############################################################
	# Done.
	############################################################

	SET(ClangFoo_FOUND TRUE)

endif()

################################################################################
