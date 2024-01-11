FUNCTION(parse_version_string version_string major_version minor_version
  patch_version)
	STRING(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" dummy
	  "${version_string}")
	SET(${major_version} "${CMAKE_MATCH_1}" PARENT_SCOPE)
	SET(${minor_version} "${CMAKE_MATCH_2}" PARENT_SCOPE)
	SET(${patch_version} "${CMAKE_MATCH_3}" PARENT_SCOPE)
ENDFUNCTION()
