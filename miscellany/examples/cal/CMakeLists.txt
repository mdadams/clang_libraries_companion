cmake_minimum_required(VERSION 3.14)

set(CAL_VERSION 0.0.1)

project(cal LANGUAGES CXX C VERSION ${CAL_VERSION})

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

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Boost REQUIRED COMPONENTS filesystem)

option(USE_FMTLIB "Use fmtlib for std::format support." false)
if(USE_FMTLIB)
	find_package(Fmt REQUIRED)
	link_libraries(Fmt::Fmt)
endif()

llvm_map_components_to_libnames(llvm_libs all)

# If Clang was built with RTTI disabled, then disable RTTI (to avoid linker
# errors caused by undefined references related to typeinfo).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_subdirectory(src)

#set(CAL_CONFIG_DIR
#  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/CAL")

configure_package_config_file(
  build/cmake/CALConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfig.cmake
  INSTALL_DESTINATION "${CAL_CONFIG_DIR}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfigVersion.cmake
  VERSION ${CAL_VERSION}
  COMPATIBILITY SameMajorVersion)

#install(FILES
#  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfig.cmake
#  ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/CALConfigVersion.cmake
#  ${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/FindCAL.cmake
#  DESTINATION "${CAL_CONFIG_DIR}")

add_custom_target(demo)
