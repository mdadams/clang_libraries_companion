find_path(
  CAL_INCLUDE_DIR
  cal/main.hpp
  PATHS $ENV{CAL_ROOT_DIR}/include
  )
mark_as_advanced(CAL_INCLUDE_DIR)

find_library(
  CAL_LIBRARY
  NAMES cal
  PATHS
  $ENV{CAL_ROOT_DIR}/lib
  $ENV{CAL_ROOT_DIR}/lib64
  )
mark_as_advanced(CAL_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  CAL
  DEFAULT_MSG
  CAL_LIBRARY CAL_INCLUDE_DIR
  )

find_package(Boost REQUIRED COMPONENTS filesystem)

if(Boost_FOUND AND CAL_FOUND)
	set(CAL_LIBRARIES ${CAL_LIBRARY})
	set(CAL_INCLUDE_DIRS ${CAL_INCLUDE_DIR})
	message(STATUS "Found fmt library: ${CAL_LIBRARY}")
	add_library(CAL::CAL UNKNOWN IMPORTED)
	set_target_properties(
	  CAL::CAL PROPERTIES
	  INTERFACE_INCLUDE_DIRECTORIES "${CAL_INCLUDE_DIR}"
	)
	set_target_properties(
	  CAL::CAL PROPERTIES
	  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
	  IMPORTED_LOCATION "${CAL_LIBRARY}"
	)
	target_link_libraries(CAL::CAL INTERFACE Boost::filesystem)
endif()
