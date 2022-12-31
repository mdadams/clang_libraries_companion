find_path(
  FMT_INCLUDE_DIR
  fmt/core.h
  HINTS ENV CC_INC_PATH
  )
mark_as_advanced(FMT_INCLUDE_DIR)

find_library(
  FMT_LIBRARY
  NAMES fmt fmtd
  HINTS ENV CC_LIB_PATH
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
	add_library(Fmt::Fmt UNKNOWN IMPORTED)
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
