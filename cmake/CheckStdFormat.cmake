include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

function(check_std_format out)
	if(DEFINED ENV{CL_ENABLE_FMTLIB})
		if (ENV{CL_ENABLE_FMTLIB})
			set(${out} FALSE PARENT_SCOPE)
		else()
			set(${out} TRUE PARENT_SCOPE)
		endif()
		return()
	endif()
	cmake_push_check_state(RESET)
	#set(CMAKE_REQUIRED_FLAGS "-std=c++20")
	check_cxx_source_compiles("
#include <format>
int main() {auto s = std::format(\"hello\");}
"
		compile_result
	)
	if(NOT compile_result)
		set(compile_result FALSE)
	endif()
	set(${out} ${compile_result} PARENT_SCOPE)
	cmake_pop_check_state()
endfunction()

function(import_std_format)
	check_std_format(std_format)
	message(STATUS "checking if std::format available: ${std_format}")
	if(NOT std_format)
		find_package(Fmt REQUIRED)
		if(TARGET Fmt::Fmt)
			link_libraries(Fmt::Fmt)
		endif()
	endif()
endfunction()
