cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(examples LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
#set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(ExternalProject)
include(FetchContent)
include(CheckStdFormat)

set(CMAKE_CXX_STANDARD 20)

check_std_format(std_format)

if(NOT std_format)

################################################################################

ExternalProject_Add(fmt
  SOURCE_DIR ${CMAKE_BINARY_DIR}/fmt-source
  BINARY_DIR ${CMAKE_BINARY_DIR}/fmt-build
  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  GIT_REPOSITORY https://github.com/mdadams/sde.git
  GIT_TAG dc39be0ee01ebf7efd6d36f17cc0b4b575dc02c6
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_COMMAND
  <SOURCE_DIR>/bin/sde_install_fmtlib
  -x
  -y
  -v 9.1.0
  -d <INSTALL_DIR>
  )

else()

add_library(dummy dummy.cpp)
install(TARGETS dummy)

endif()
