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
provide a single shared library for each of the LLVM and Clang C++ (i.e.,
clang-cpp) library code and/or individual static libraries for the LLVM
and Clang library code.  This type of configuration is quite commonly
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
This is useful for building statically-linked executables.

ClangFoo_USE_CLANGCPP_COMPONENTS
This is a boolean indicating if the Clang C++ component libraries should
be used (instead of the single combined clang-cpp library).
This is useful for building statically-linked executables.

Generally, it is recommended that the default values for the
above variables be used, unless this causes problems.

The following imported targets are provided:

  ClangFoo::llvm
  ClangFoo::clangcpp

Some other targets are also provided (which are deliberately
undocumented), but their use is discouraged.

Michael Adams
2025-10-26

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

	LIST(GET ARGN 0 first_arg)
	SET(message_types STATUS WARNING AUTHOR_ERROR FATAL_ERROR SEND_ERROR TEXT)
	LIST(FIND message_types "${first_arg}" message_index)
	IF(message_index GREATER_EQUAL 0)
		SET(message_type "${first_arg}")
		LIST(REMOVE_AT ARGN 0)
		IF(message_type STREQUAL "TEXT")
			SET(message_type "")
		ENDIF()
	ELSE()
		SET(message_type "")
	ENDIF()
	SET(message_text "${ARGN}")

	IF(message_type STREQUAL "STATUS")
		MESSAGE(STATUS "${message_text}")
	ELSEIF(message_type STREQUAL "WARNING")
		MESSAGE(WARNING "${message_text}")
	ELSEIF(message_type STREQUAL "AUTHOR_ERROR")
		MESSAGE(AUTHOR_ERROR "${message_text}")
	ELSEIF(message_type STREQUAL "FATAL_ERROR")
		MESSAGE(FATAL_ERROR "${message_text}")
	ELSEIF(message_type STREQUAL "SEND_ERROR")
		MESSAGE(SEND_ERROR "${message_text}")
	ELSE()
		MESSAGE("${message_text}")
	ENDIF()

	LIST(POP_BACK CMAKE_MESSAGE_INDENT)

ENDFUNCTION()

################################################################################

function(print_transitive_dependencies target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} does not exist")
    endif()

    # Create a global property to track visited targets.
    set_property(GLOBAL PROPERTY _visited_targets "")

    function(_print_deps t)
        # Read the global visited list.
        get_property(visited GLOBAL PROPERTY _visited_targets)
        list(FIND visited "${t}" _found)
        if(NOT _found EQUAL -1)
            return()
        endif()

        # Mark this target as visited.
        list(APPEND visited "${t}")
        set_property(GLOBAL PROPERTY _visited_targets "${visited}")

        # Get the target's link libraries.
        get_target_property(_libs "${t}" INTERFACE_LINK_LIBRARIES)
        if(NOT _libs)
            set(_libs "")
        endif()

        foreach(lib IN LISTS _libs)
            if(NOT lib STREQUAL "")
                message(STATUS "-- ${t} -> ${lib}")
                if(TARGET "${lib}")
                    _print_deps("${lib}")
                endif()
            endif()
        endforeach()
    endfunction()

    _print_deps(${target})

    # Clear the global visited list after done.
    set_property(GLOBAL PROPERTY _visited_targets "")
endfunction()

################################################################################

# This function recursively removes shared-library dependencies from targets.
# This modifies the INTERFACE_LINK_LIBRARIES of each target to exclude
# shared libraries.
function(strip_shared_deps_from_targets target_list)

    # Use a global property to track visited targets to avoid scoping issues.
    set_property(GLOBAL PROPERTY _strip_visited_targets "")

    function(_strip_target_deps target)
        # Check if already processed using global property.
        get_property(visited_targets GLOBAL PROPERTY _strip_visited_targets)
        list(FIND visited_targets "${target}" found_idx)
        if(NOT found_idx EQUAL -1)
            return()
        endif()

        # Mark as visited.
        list(APPEND visited_targets "${target}")
        set_property(GLOBAL PROPERTY _strip_visited_targets "${visited_targets}")

        # Only process actual targets.
        if(NOT TARGET ${target})
            return()
        endif()

        # Get the target type.
        get_target_property(target_type ${target} TYPE)

        # Skip if it is a shared library itself.
        if(target_type STREQUAL "SHARED_LIBRARY")
            return()
        endif()

        # Get the current link libraries.
        get_target_property(link_libs ${target} INTERFACE_LINK_LIBRARIES)
        if(link_libs STREQUAL "link_libs-NOTFOUND")
            return()
        endif()

        # Filter the link libraries.
        set(filtered_libs "")
        foreach(lib IN LISTS link_libs)
            set(keep_lib TRUE)

            if(TARGET ${lib})
                get_target_property(lib_type ${lib} TYPE)

                # Filter out shared libraries.
                if(lib_type STREQUAL "SHARED_LIBRARY" OR lib_type STREQUAL "MODULE_LIBRARY")
                    set(keep_lib FALSE)
                    if(ClangFoo_VERBOSE GREATER_EQUAL 3)
                        ClangFoo_MESSAGE(STATUS "Removing shared lib ${lib} from ${target}")
                    endif()
                else()
                    # Recursively process this dependency.
                    _strip_target_deps(${lib})
                endif()
            endif()

            if(keep_lib)
                list(APPEND filtered_libs ${lib})
            endif()
        endforeach()

        # Update the target's link libraries.
        set_target_properties(${target} PROPERTIES
            INTERFACE_LINK_LIBRARIES "${filtered_libs}")

    endfunction()

    # Process each target in the list.
    foreach(target IN LISTS target_list)
        _strip_target_deps(${target})
    endforeach()

    # Clean up global property.
    set_property(GLOBAL PROPERTY _strip_visited_targets "")

endfunction()

# This simpler function filters a list.
# It does not modify targets, only the list.
function(strip_shared_deps list_var)

    set(result "")
    foreach(lib IN LISTS ${list_var})
        set(keep_lib TRUE)

        # Check if it is a target.
        if(TARGET ${lib})
            get_target_property(lib_type ${lib} TYPE)

            # Filter out shared libraries.
            if(lib_type STREQUAL "SHARED_LIBRARY")
                set(keep_lib FALSE)
                if(ClangFoo_VERBOSE GREATER_EQUAL 3)
                    ClangFoo_MESSAGE(STATUS "Filtering out shared library: ${lib}")
                endif()
            endif()

            # Optionally filter out module libraries.
            if(lib_type STREQUAL "MODULE_LIBRARY")
                set(keep_lib FALSE)
                if(ClangFoo_VERBOSE GREATER_EQUAL 3)
                    ClangFoo_MESSAGE(STATUS "Filtering out module library: ${lib}")
                endif()
            endif()
        endif()

        if(keep_lib)
            list(APPEND result ${lib})
        endif()
    endforeach()

    # Return the filtered list.
    set(${list_var} "${result}" PARENT_SCOPE)

endfunction()

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
IF((NOT DEFINED ClangFoo_VERBOSE) AND (DEFINED ENV{ClangFoo_VERBOSE}))
	SET(ClangFoo_VERBOSE $ENV{ClangFoo_VERBOSE})
	IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
		ClangFoo_MESSAGE(STATUS "ClangFoo_VERBOSE set from environment ${ClangFoo_VERBOSE}")
	ENDIF()
ENDIF()
IF(NOT DEFINED ClangFoo_VERBOSE)
	#SET(ClangFoo_VERBOSE 20)
	#SET(ClangFoo_VERBOSE 10)
	#SET(ClangFoo_VERBOSE 3)
	#SET(ClangFoo_VERBOSE 2)
	SET(ClangFoo_VERBOSE 1)
	#SET(ClangFoo_VERBOSE 0)
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS "ClangFoo_VERBOSE: ${ClangFoo_VERBOSE}")
ENDIF()

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

IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
	ClangFoo_MESSAGE(STATUS "LLVM_VERSION: ${LLVM_VERSION}")
ENDIF()

# Print the value of LLVM_LINK_LLVM_DYLIB for debugging.
IF(NOT DEFINED LLVM_LINK_LLVM_DYLIB)
	SET(ClangFoo_status "undefined")
ELSEIF(LLVM_LINK_LLVM_DYLIB)
	SET(ClangFoo_status "true")
ELSE()
	SET(ClangFoo_status "false")
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS "LLVM_LINK_LLVM_DYLIB: ${ClangFoo_status}")
ENDIF()

# Print the value of CLANG_LINK_CLANG_DYLIB for debugging.
IF(NOT DEFINED CLANG_LINK_CLANG_DYLIB)
	SET(ClangFoo_status "undefined")
ELSEIF(CLANG_LINK_CLANG_DYLIB)
	SET(ClangFoo_status "true")
ELSE()
	SET(ClangFoo_status "false")
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS "CLANG_LINK_CLANG_DYLIB: ${ClangFoo_status}")
ENDIF()

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_llvm_target TARGET LLVM)
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS "TARGET LLVM: ${ClangFoo_found_llvm_target}")
ENDIF()

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_clangcpp_target TARGET clang-cpp)
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS
	  "TARGET clang-cpp: ${ClangFoo_found_clangcpp_target}")
ENDIF()

################################################################################
# Select default values for some key settings.
################################################################################

SET(ClangFoo_PROPAGATE_ENABLE_RTTI_DEFAULT TRUE)

SET(ClangFoo_PROPAGATE_ENABLE_EH_DEFAULT FALSE)

SET(ClangFoo_FORCE_STATIC_DEFAULT FALSE)

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_llvm_lib
  (TARGET LLVM) AND (DEFINED LLVM_LINK_LLVM_DYLIB) AND LLVM_LINK_LLVM_DYLIB)
ClangFoo_ASSIGN_TO_BOOL(ClangFoo_USE_LLVM_COMPONENTS_DEFAULT
  NOT ${ClangFoo_found_llvm_lib})

ClangFoo_ASSIGN_TO_BOOL(ClangFoo_found_clangcpp_lib
  (TARGET clang-cpp) AND (DEFINED CLANG_LINK_CLANG_DYLIB) AND
  CLANG_LINK_CLANG_DYLIB)
ClangFoo_ASSIGN_TO_BOOL(ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT
  NOT ${ClangFoo_found_clangcpp_lib})

IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS "found libLLVM: ${ClangFoo_found_llvm_lib}")
	ClangFoo_MESSAGE(STATUS "found libclang-cpp: ${ClangFoo_found_clangcpp_lib}")
ENDIF()

IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS "ClangFoo_USE_LLVM_COMPONENTS_DEFAULT: "
	  "${ClangFoo_USE_LLVM_COMPONENTS_DEFAULT}")
	ClangFoo_MESSAGE(STATUS "ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT: "
	  "${ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT}")
ENDIF()

SET(ClangFoo_enable_static_libzstd_hack TRUE)

################################################################################
# Initialize some key variables with values taken from the environment
# if appropriate.
################################################################################

IF((NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS) AND
  (DEFINED ENV{ClangFoo_USE_LLVM_COMPONENTS}))
	SET(ClangFoo_USE_LLVM_COMPONENTS $ENV{ClangFoo_USE_LLVM_COMPONENTS})
	IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
		ClangFoo_MESSAGE(STATUS
		  "ClangFoo_USE_LLVM_COMPONENTS set from environment ${ClangFoo_USE_LLVM_COMPONENTS}")
	ENDIF()
ENDIF()

IF((NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS) AND
  (DEFINED ENV{ClangFoo_USE_CLANGCPP_COMPONENTS}))
	SET(ClangFoo_USE_CLANGCPP_COMPONENTS
	  $ENV{ClangFoo_USE_CLANGCPP_COMPONENTS})
	IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
		ClangFoo_MESSAGE(STATUS
		  "ClangFoo_USE_CLANGCPP_COMPONENTS set from environment ${ClangFoo_USE_CLANGCPP_COMPONENTS}")
	ENDIF()
ENDIF()

IF((NOT DEFINED ClangFoo_PROPAGATE_ENABLE_RTTI) AND 
  (DEFINED ENV{ClangFoo_PROPAGATE_ENABLE_RTTI}))
	SET(ClangFoo_PROPAGATE_ENABLE_RTTI $ENV{ClangFoo_PROPAGATE_ENABLE_RTTI})
	IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
		ClangFoo_MESSAGE(STATUS
		  "ClangFoo_PROPAGATE_ENABLE_RTTI set from environment ${ClangFoo_PROPAGATE_ENABLE_RTTI}")
	ENDIF()
ENDIF()

IF((NOT DEFINED ClangFoo_PROPAGATE_ENABLE_EH) AND
  (DEFINED ENV{ClangFoo_PROPAGATE_ENABLE_EH}))
	SET(ClangFoo_PROPAGATE_ENABLE_EH $ENV{ClangFoo_PROPAGATE_ENABLE_EH})
	IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
		ClangFoo_MESSAGE(STATUS
		  "ClangFoo_PROPAGATE_ENABLE_EH set from environment ${ClangFoo_PROPAGATE_ENABLE_EH}")
	ENDIF()
ENDIF()

IF((NOT DEFINED ClangFoo_FORCE_STATIC) AND
  (DEFINED ENV{ClangFoo_FORCE_STATIC}))
	SET(ClangFoo_FORCE_STATIC $ENV{ClangFoo_FORCE_STATIC})
	ClangFoo_ASSIGN_TO_BOOL(ClangFoo_FORCE_STATIC
	  ${ClangFoo_FORCE_STATIC})
	IF(ClangFoo_VERBOSE GREATER_EQUAL 1)
		ClangFoo_MESSAGE(STATUS
		  "ClangFoo_FORCE_STATIC set from environment ${ClangFoo_FORCE_STATIC}")
	ENDIF()
ENDIF()

################################################################################
# Initialize some key variables to default values if appropriate.
################################################################################

IF(NOT DEFINED ClangFoo_USE_LLVM_COMPONENTS)
	SET(ClangFoo_USE_LLVM_COMPONENTS ${ClangFoo_USE_LLVM_COMPONENTS_DEFAULT})
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_USE_LLVM_COMPONENTS: ${ClangFoo_USE_LLVM_COMPONENTS}")
ENDIF()

IF(NOT DEFINED ClangFoo_USE_CLANGCPP_COMPONENTS)
	SET(ClangFoo_USE_CLANGCPP_COMPONENTS
	  ${ClangFoo_USE_CLANGCPP_COMPONENTS_DEFAULT})
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_USE_CLANGCPP_COMPONENTS: ${ClangFoo_USE_CLANGCPP_COMPONENTS}")
ENDIF()

IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_RTTI)
	SET(ClangFoo_PROPAGATE_ENABLE_RTTI
	  ${ClangFoo_PROPAGATE_ENABLE_RTTI_DEFAULT})
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_PROPAGATE_ENABLE_RTTI: ${ClangFoo_PROPAGATE_ENABLE_RTTI}")
ENDIF()

IF(NOT DEFINED ClangFoo_PROPAGATE_ENABLE_EH)
	SET(ClangFoo_PROPAGATE_ENABLE_EH ${ClangFoo_PROPAGATE_ENABLE_EH_DEFAULT})
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_PROPAGATE_ENABLE_EH: ${ClangFoo_PROPAGATE_ENABLE_EH}")
ENDIF()

IF(NOT DEFINED ClangFoo_FORCE_STATIC)
	SET(ClangFoo_FORCE_STATIC ${ClangFoo_FORCE_STATIC_DEFAULT})
ENDIF()
IF(ClangFoo_VERBOSE GREATER_EQUAL 2)
	ClangFoo_MESSAGE(STATUS
	  "ClangFoo_FORCE_STATIC: ${ClangFoo_FORCE_STATIC}")
ENDIF()

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

# The following is somewhat of a hack.
# Some builds of LLVM are linked against the static libzstd library.
# For such builds, the CMake package config file for LLVM requires the target
# for the static library (zstd::libzstd_static) to be defined.  As a
# convenience, if the shared library is installed but not the static library,
# we allow the shared library to be used in place of the static library.
IF(ClangFoo_enable_static_libzstd_hack)
	GET_TARGET_PROPERTY(ClangFoo_llvm_support_libs LLVMSupport
	  INTERFACE_LINK_LIBRARIES)
	IF("zstd::libzstd_static" IN_LIST ClangFoo_llvm_support_libs)
		SET(ClangFoo_uses_static_libzstd TRUE)
	ELSE()
		SET(ClangFoo_uses_static_libzstd FALSE)
	ENDIF()
	ClangFoo_MESSAGE(STATUS
	  "LLVM uses static libzstd: ${ClangFoo_uses_static_libzstd}")
	IF(ClangFoo_uses_static_libzstd AND TARGET zstd::libzstd_shared AND
	  NOT TARGET zstd::libzstd_static)
		ClangFoo_MESSAGE(WARNING
		  "Using the shared libzstd library in place of the static one.")
		ClangFoo_MESSAGE(WARNING
		  "It is advisable to install the static libzstd library if possible.")
		add_library(zstd::libzstd_static ALIAS zstd::libzstd_shared)
	ENDIF()
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
			IF (Clangfoo_VERBOSE GREATER_EQUAL 2)
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
			IF (Clangfoo_VERBOSE GREATER_EQUAL 2)
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

		IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
			ClangFoo_MESSAGE(STATUS "LLVM: considering item ${item}")
		ENDIF()
		IF(NOT TARGET ${item})
			IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
				ClangFoo_MESSAGE(STATUS "LLVM: skipping non-target ${item}")
			ENDIF()
			CONTINUE()
		ENDIF()

		IF(NOT TARGET ClangFoo::llvm::${item})
			ADD_LIBRARY(ClangFoo::llvm::${item} INTERFACE IMPORTED)
		ENDIF()
		SET_TARGET_PROPERTIES(ClangFoo::llvm::${item} PROPERTIES
		  INTERFACE_LINK_LIBRARIES ${item})
		#TARGET_COMPILE_OPTIONS(ClangFoo::llvm::${item} INTERFACE
		#  ${clangfoo_options})
		TARGET_COMPILE_OPTIONS(ClangFoo::llvm::${item} INTERFACE
		  $<$<COMPILE_LANGUAGE:CXX>:${clangfoo_options}>)
		TARGET_COMPILE_DEFINITIONS(ClangFoo::llvm::${item} INTERFACE
		  ${LLVM_DEFINITIONS})
		TARGET_INCLUDE_DIRECTORIES(ClangFoo::llvm::${item} INTERFACE
		  ${LLVM_INCLUDE_DIRS})
		TARGET_LINK_DIRECTORIES(ClangFoo::llvm::${item} INTERFACE
		  ${LLVM_LIBRARY_DIRS})
		SET_TARGET_PROPERTIES(ClangFoo::llvm::${item} PROPERTIES
		  INTERFACE_LINK_LIBRARIES ${item})

		GET_TARGET_PROPERTY(ClangFoo_target_type ${item} TYPE)
		IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
			ClangFoo_MESSAGE(STATUS "LLVM component ${item} ${ClangFoo_target_type}")
		ENDIF()
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
			IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
				ClangFoo_MESSAGE(STATUS "LLVM: found component ${item}")
			ENDIF()
			LIST(APPEND clangfoo_llvm_comps "${item}")
		ELSE()
			IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
				ClangFoo_MESSAGE(STATUS "LLVM: skipping component ${item}")
			ENDIF()
		ENDIF()

	ENDFOREACH()

	IF(ClangFoo_VERBOSE GREATER_EQUAL 3)
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
		IF(ClangFoo_FORCE_STATIC)
			strip_shared_deps(clangfoo_llvm_comps)
			strip_shared_deps_from_targets("${clangfoo_llvm_comps}")
		ENDIF()
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
		IF(ClangFoo_VERBOSE GREATER_EQUAL 20)
			print_transitive_dependencies(ClangFoo::llvm_components)
		ENDIF()
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

		IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
			ClangFoo_MESSAGE(STATUS "Clang: considering item ${item}")
		ENDIF()
		IF(NOT TARGET ${item})
			IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
				ClangFoo_MESSAGE(STATUS "Clang: skipping non-target ${item}")
			ENDIF()
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
		IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
			ClangFoo_MESSAGE(STATUS "clangcpp component ${item} ${ClangFoo_target_type}")
		ENDIF()
		SET(clangfoo_selected FALSE)
		IF(item MATCHES "^clang[A-Z].*$")
			SET(clangfoo_selected TRUE)
		ENDIF()
		IF(clangfoo_selected AND
		  (ClangFoo_target_type STREQUAL "STATIC_LIBRARY"))
			IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
				ClangFoo_MESSAGE(STATUS "clangcpp: found component ${item}")
			ENDIF()
			LIST(APPEND clangfoo_clangcpp_comps ${item})
		ELSE()
			IF(ClangFoo_VERBOSE GREATER_EQUAL 10)
				ClangFoo_MESSAGE(STATUS "clangcpp: skipping component ${item}")
			ENDIF()
		ENDIF()

	ENDFOREACH()

	IF(ClangFoo_VERBOSE GREATER_EQUAL 3)
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
		IF(ClangFoo_FORCE_STATIC)
			strip_shared_deps(clangfoo_clangcpp_comps)
			strip_shared_deps_from_targets("${clangfoo_clangcpp_comps}")
		ENDIF()
		IF(NOT TARGET ClangFoo::clangcpp_components)
			ADD_LIBRARY(ClangFoo::clangcpp_components INTERFACE IMPORTED)
		ENDIF()
		SET_TARGET_PROPERTIES(ClangFoo::clangcpp_components PROPERTIES
		  INTERFACE_LINK_LIBRARIES "${clangfoo_clangcpp_comps}")
		IF(ClangFoo_VERBOSE GREATER_EQUAL 20)
			print_transitive_dependencies(ClangFoo::clangcpp_components)
		ENDIF()
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
