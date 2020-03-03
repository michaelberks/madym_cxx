PROJECT( manchester_qbi_madym )

cmake_minimum_required(VERSION 3.10)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

####################################################
#Get version of the git repository
####################################################
if (WIN32)
  set(GIT_EXE_PATH "C:/Program Files/Git/bin/")
endif(WIN32)

# Get the current working branch
execute_process(
  COMMAND ${GIT_EXE_PATH}git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND ${GIT_EXE_PATH}git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the current tag
execute_process(
  COMMAND ${GIT_EXE_PATH}git describe --tags --abbrev=0
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${CMAKE_SOURCE_DIR}/mdm_version.h.in
  ${CMAKE_BINARY_DIR}/generated/mdm_version.h
)

string(REPLACE "." ";" VERSION_LIST ${GIT_TAG})
list(GET VERSION_LIST 0 MADYM_VERSION_MAJOR)
list(GET VERSION_LIST 1 MADYM_VERSION_MINOR)
list(GET VERSION_LIST 2 MADYM_VERSION_PATCH)

MESSAGE(STATUS "Madym version ${MADYM_VERSION_MAJOR}.${MADYM_VERSION_MINOR}.${MADYM_VERSION_PATCH}")

file(COPY 
  ${CMAKE_CURRENT_SOURCE_DIR}/calibration_data/ 
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/calibration_data/)

include_directories(${CMAKE_BINARY_DIR}/generated)
#################################################################

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
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

#-----------------------------------------------------------------------------
SETIFEMPTY(CMAKE_INSTALL_LIBRARY_DESTINATION lib)
SETIFEMPTY(CMAKE_INSTALL_ARCHIVE_DESTINATION lib)
SETIFEMPTY(CMAKE_INSTALL_RUNTIME_DESTINATION bin)

SETIFEMPTY(IMSCI_QBI_INSTALL_RUNTIME_DIR bin)
SETIFEMPTY(IMSCI_QBI_INSTALL_LIBRARY_DIR lib)
SETIFEMPTY(IMSCI_QBI_INSTALL_ARCHIVE_DIR lib)
SETIFEMPTY(IMSCI_QBI_INSTALL_INCLUDE_DIR include)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} )

OPTION( BUILD_TESTING "Should the tests be built?" YES )

# Best to use shared libraries
OPTION(BUILD_SHARED_LIBS "Build with shared libraries." YES)

# Best to use shared libraries
OPTION(BUILD_DOCUMENTATION "Build documentation using doxygen." NO)

IF (BUILD_DOCUMENTATION)
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


#################################################################

#################################################################
# The CMake command FIND_PACKAGE(VXL) attempts to find VXL binary
# installation.  CMake will look in the directory specified by the
# CMake variable VXL_DIR.  Normally, CMake will initially not find
# VXL, will warn you that it could not find VXL, and then give you the
# chance to set the variable VXL_DIR and reconfigure.  VXL_DIR now
# replaces VXL_BINARY_PATH.

FIND_PACKAGE(VXL)

# Whether FIND_PACKAGE(VXL) worked is stored in the variable
# VXL_FOUND.  If VXL was found we then include `UseVXL.cmake'
# which will set many variables prefixed with VXL_ that your project
# can use to determine what parts of VXL are present and how to use
# them.  `UseVXL.cmake' holds all the necessary definitions that
# CMake needs to use VXL.
IF(VXL_FOUND)

  IF( BUILD_TESTING )
    ENABLE_TESTING()
  ENDIF( BUILD_TESTING )

  OPTION( BUILD_TESTING "Should the tests be built?" YES )

  INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)

  # The following indicates where VXL specific CMake modules are to be found
  # It may only be necessary if you keep your ISBE clue libraries in a separate directory tree from VXL
  SET(MODULE_PATH ${VXL_CMAKE_DIR})

  # Tell the system where to find the VXL source 
  # It may be possible to pick this up from the UseVXL.cmake, but this works too.
  SET(VXL_SOURCE_DIR )

  INCLUDE_DIRECTORIES(${VXL_ROOT_SOURCE_DIR})
  INCLUDE_DIRECTORIES(${VXL_CORE_INCLUDE_DIR})

  message(“VXL found”)

ENDIF(VXL_FOUND)

SUBDIRS(madym)