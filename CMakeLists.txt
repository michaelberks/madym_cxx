PROJECT( manchester_qbi_madym )

cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0003 NEW)

#Some annoying warnings/bugs here on Apple to fix via policy settings
if (APPLE)
  cmake_policy(SET CMP0025 NEW)
  cmake_policy(SET CMP0042 NEW)
  set(CMAKE_MACOSX_RPATH 1)
  add_compile_options()
endif()

####################################################
# Require C++17
set(CMAKE_CXX_STANDARD 17 CACHE STRING "Defines standard required for C++ compiler")

####################################################
#Get version of the git repository
####################################################
find_package(Git)

if (Git_FOUND)

  message(STATUS "git found at ${GIT_EXECUTABLE}")
  # Get the current working branch
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Get the latest abbreviated commit hash of the working branch
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Get the current tag
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  message(WARNING "git not found, and so could not be used to generate Madym's version
    from the latest repository tag. Version will be set to v0.0.0")
  set(GIT_TAG "v0.0.0")
endif()

#-----------------------------------------------------------------------------
# Check if user supplied install prefix
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR DEFAULT_INSTALL_PREFIX)
  set(DEFAULT_INSTALL_PREFIX TRUE CACHE BOOL "..." FORCE)
else()
  set(DEFAULT_INSTALL_PREFIX FALSE CACHE BOOL "..." FORCE)
endif()

#-----------------------------------------------------------------------------
if(NOT COMMAND SETIFEMPTY)
	macro(SETIFEMPTY)
		set(KEY ${ARGV0})
		set(VALUE ${ARGV1})
		if(NOT ${KEY})
			set(${ARGV})
		endif()
	endmacro()
endif()
#-----------------------------------------------------------------------------

#Set output locations in build dir
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(MADYM_VERSION ${GIT_TAG})
string(REPLACE "." ";" VERSION_TEMP ${MADYM_VERSION})
list(GET VERSION_TEMP 0 MADYM_VERSION_MAJOR)
list(GET VERSION_TEMP 1 MADYM_VERSION_MINOR)
list(GET VERSION_TEMP 2 MADYM_VERSION_PATCH)
string(REPLACE "v" "" MADYM_VERSION_MAJOR ${MADYM_VERSION_MAJOR})

message(STATUS "Madym version ${MADYM_VERSION_MAJOR}.${MADYM_VERSION_MINOR}.${MADYM_VERSION_PATCH}")
set(QBI_WEBSITE "http://www.qbi-lab.org/" 
  CACHE STRING "Address of QBI lab website")
set(USER_WIKI 
  "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui" 
  CACHE STRING "Address of user help wiki website")

if(CMAKE_GENERATOR STREQUAL Xcode)
  add_definitions(-DUSING_XCODE)
endif()

#---------------------------------------------------------------------------
if ( DEFAULT_INSTALL_PREFIX)
  set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/deploy" CACHE PATH "default install path" FORCE )
  message(STATUS "Overriding default install prefix ${CMAKE_INSTALL_PREFIX}")
endif()
set(MADYM_DEPLOY_DIR "madym-${MADYM_VERSION}")

#---------------------------------------------------------------------------
# Configure RPATH settings, see https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
if (APPLE)
  configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/madym_setup.sh.in
    ${CMAKE_BINARY_DIR}/generated/madym_setup.sh
  )
  set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
else()
  #set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

#---------------------------------------------------------------------------
configure_file(
  ${CMAKE_SOURCE_DIR}/mdm_version.h.in
  ${CMAKE_BINARY_DIR}/generated/mdm_version.h
)
configure_file(
  ${CMAKE_SOURCE_DIR}/scripts/postinst.in
  ${CMAKE_BINARY_DIR}/generated/postinst
)
configure_file(
  ${CMAKE_SOURCE_DIR}/scripts/prerm.in
  ${CMAKE_BINARY_DIR}/generated/prerm
)
include_directories(${CMAKE_BINARY_DIR}/generated)

file(COPY 
  ${CMAKE_SOURCE_DIR}/calibration_data/ 
  DESTINATION ${CMAKE_BINARY_DIR}/calibration_data/)


include_directories(${CMAKE_CURRENT_SOURCE_DIR} )

#Run the installation parts of the make lists
option( BUILD_INSTALL "Should the install targets be built?" NO )

option( BUILD_TESTING "Should the tests be built?" YES )

# Best to use shared libraries
option(BUILD_SHARED_LIBS "Build with shared libraries." YES)

# Best to use shared libraries
option(BUILD_DOCUMENTATION "Build documentation using doxygen." NO)

if (BUILD_DOCUMENTATION)
  # Require dot, treat the other components as optional
  find_package(Doxygen)
  
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  # set input and output files
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message(STATUS "Doxygen executable command ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}")

  # note the option ALL which allows to build the docs together with the application
  add_custom_target( docs_doxygen ALL
    COMMAND $<$<CONFIG:Release>:${DOXYGEN_EXECUTABLE}> $<$<CONFIG:Release>:${DOXYGEN_OUT}>
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )

endif(BUILD_DOCUMENTATION)

if( BUILD_TESTING )
	ENABLE_TESTING()
endif( BUILD_TESTING )

option( BUILD_TESTING "Should the tests be built?" YES )

subdirs(madym)