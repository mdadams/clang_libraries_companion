option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ENABLE_ASAN_RECOVER "Enable Address Sanitizer Error Recovery" OFF)
option(ENABLE_UBSAN "Enable Undefined-Behavior Sanitizer" OFF)
option(ENABLE_LSAN "Enable Leak Sanitizer" OFF)
option(ENABLE_MSAN "Enable Memory Sanitizer" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL GNU)

	#check_cxx_compiler_flag("-fsanitize=address" HAVE_FSANITIZE_ADDRESS)
	set(HAVE_FSANITIZE_ADDRESS 1)

	check_cxx_compiler_flag("-fsanitize=leak" HAVE_FSANITIZE_LEAK)

	#check_cxx_compiler_flag("-fsanitize=memory" HAVE_FSANITIZE_MEMORY)
	set(HAVE_FSANITIZE_MEMORY 1)

	#check_cxx_compiler_flag("-fsanitize=thread" HAVE_FSANITIZE_THREAD)
	set(HAVE_FSANITIZE_THREAD 1)

	check_cxx_compiler_flag("-fsanitize=undefined" HAVE_FSANITIZE_UNDEFINED)

endif()

if(ENABLE_MSAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
	  CMAKE_C_COMPILER_ID STREQUAL GNU)
		if(HAVE_FSANITIZE_MEMORY)
			message("Enabling Memory Sanitizer")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
			set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fsanitize=memory")
		endif()
	endif()
endif()

if(ENABLE_ASAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
	  CMAKE_C_COMPILER_ID STREQUAL GNU)
		if(HAVE_FSANITIZE_ADDRESS)
			message("Enabling Address Sanitizer")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
			set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fsanitize=address")
			if(ENABLE_ASAN_RECOVER)
				set(CMAKE_C_FLAGS
				  "${CMAKE_C_FLAGS} -fsanitize-recover=address")
				set(CMAKE_CXX_FLAGS
				  "${CMAKE_CXX_FLAGS} -fsanitize-recover=address")
				set(CMAKE_LD_FLAGS
				  "${CMAKE_LD_FLAGS} -fsanitize-recover=address")
			endif()
		endif()
	endif()
endif()

if(ENABLE_LSAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
	  CMAKE_C_COMPILER_ID STREQUAL GNU)
		if(HAVE_FSANITIZE_LEAK)
			message("Enabling Leak Sanitizer")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
			set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fsanitize=leak")
		endif()
	endif()
endif()

if(ENABLE_UBSAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
	  CMAKE_C_COMPILER_ID STREQUAL GNU)
		if(HAVE_FSANITIZE_UNDEFINED)
			message("Enabling Undefined-Behavior Sanitizer")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
			set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fsanitize=undefined")
		endif()
	endif()
endif()

if(ENABLE_TSAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
	  CMAKE_C_COMPILER_ID STREQUAL GNU)
		if(HAVE_FSANITIZE_THREAD)
			message("Enabling Thread Sanitizer")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
			set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fsanitize=thread")
		endif()
	endif()
endif()
