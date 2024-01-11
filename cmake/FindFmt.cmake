if(NOT DEFINED FMT_FOUND OR NOT FMT_FOUND)

find_path(
  FMT_INCLUDE_DIR
  stdfmt/core.h
  PATHS $ENV{FMTLIB_ROOT_DIR}/include
  )
mark_as_advanced(FMT_INCLUDE_DIR)

find_library(
  FMT_LIBRARY
  NAMES stdfmt
  PATHS
  $ENV{FMTLIB_ROOT_DIR}/lib
  $ENV{FMTLIB_ROOT_DIR}/lib64
  )
mark_as_advanced(FMT_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Fmt
  DEFAULT_MSG
  FMT_LIBRARY FMT_INCLUDE_DIR
  )

if(FMT_FOUND)
	set(FMT_LIBRARIES ${FMT_LIBRARY})
	set(FMT_INCLUDE_DIRS ${FMT_INCLUDE_DIR})
	message(STATUS "Found fmt library: ${FMT_LIBRARY}")
	#if(NOT TARGET Fmt::Fmt)
		add_library(Fmt::Fmt UNKNOWN IMPORTED)
	#endif()
	set_target_properties(
	  Fmt::Fmt PROPERTIES
	  INTERFACE_INCLUDE_DIRECTORIES "${FMT_INCLUDE_DIR}"
	)
	set_target_properties(
	  Fmt::Fmt PROPERTIES
	  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
	  IMPORTED_LOCATION "${FMT_LIBRARY}"
	)
endif()

endif()
