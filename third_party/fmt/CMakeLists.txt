cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(examples LANGUAGES NONE)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
#set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(ExternalProject)
include(FetchContent)

################################################################################

ExternalProject_Add(fmt
  SOURCE_DIR ${CMAKE_BINARY_DIR}/fmt-source
  BINARY_DIR ${CMAKE_BINARY_DIR}/fmt-build
  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
  GIT_REPOSITORY https://github.com/mdadams/sde.git
  #GIT_TAG        v5.0.1
  GIT_TAG        5d564f0eb372b66ecacd909e0e483ec82b8c1e6f
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
