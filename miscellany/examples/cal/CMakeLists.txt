cmake_minimum_required(VERSION 3.14)

set(CAL_VERSION 0.0.1)

project(cal LANGUAGES CXX C VERSION ${CAL_VERSION})

option(CAL_ENABLE_TEST "Enable test code" TRUE)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CheckCXXCompilerFlag)
include(Sanitizers)

# Adhere to GNU filesystem layout conventions.
include(GNUInstallDirs)

include(CMakePackageConfigHelpers)

set(CAL_CONFIG_DIR
  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/CAL")

#set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(ClangFoo REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS filesystem)
include(CheckStdFormat)
import_std_format()

add_subdirectory(src/lib)
if(CAL_ENABLE_TEST)
	add_subdirectory(src/app)
endif()

configure_package_config_file(
  build/cmake/CALConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfig.cmake
  INSTALL_DESTINATION "${CAL_CONFIG_DIR}"
  )

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfigVersion.cmake
  VERSION ${CAL_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfigVersion.cmake
  DESTINATION "${CAL_CONFIG_DIR}")

add_custom_target(demo)
