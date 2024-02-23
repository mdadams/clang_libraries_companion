SET(all_scripts
  clang_info
  find_program_variant
  gcc_info
  get_clang_include_dir
  make_vcs_version
  run_clang_tool
) 

SET(scripts "")
FOREACH(script ${all_scripts})
	IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${script}")
		LIST(APPEND scripts ${script})
	ENDIF()
ENDFOREACH()

install(
  PROGRAMS ${scripts}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
