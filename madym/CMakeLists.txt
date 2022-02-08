# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if ( BUILD_SHARED_LIBS )
	add_definitions(-DMDM_BUILD_SHARED )
endif ( BUILD_SHARED_LIBS )

option( BUILD_TOOLS "Should the main madym executable tools be built?" YES )
option( BUILD_QT_GUI "Should the madym GUI be built?" NO )

if ( BUILD_QT_GUI )
  #We find Qt bits here, because we need to link the program logger to Qt
  find_package(Qt5Core)
  find_package(Qt5Widgets)
  find_package(Qt5Gui)

  #Suppress annoying CMake warnings by setting these Qt related policies
  cmake_policy(SET CMP0020 NEW)
  cmake_policy(SET CMP0043 NEW)

  #This allows us to use definition as compiler directive in src code
  add_definitions(-DUSING_QT)
endif ()

option( BUILD_WITH_ZLIB "Include zlib to allow compressed NIFTI format (*.gz)?" NO )
if (BUILD_WITH_ZLIB)
  find_package(ZLIB)
  add_definitions(-DHAVE_ZLIB)
  list(GET ZLIB_LIBRARIES -1 LIB0)
  get_filename_component(ZLIB_LIBRARY_DIR ${LIB0} DIRECTORY)
  message(STATUS "Zlib library ${LIB0} is in ${ZLIB_LIBRARY_DIR}")

  if ( WIN32 )
     set(ZLIB_BIN_DIR "" CACHE STRING 
      "Directory where ZLIB dlls are installed")
  endif()
endif()

option( BUILD_WITH_DCMTK "Include support for DICOM images?" NO )
if (BUILD_WITH_DCMTK)
  find_package(DCMTK NO_MODULE REQUIRED)
  include_directories(${DCMTK_INCLUDE_DIRS})
  add_definitions(-DUSING_DCMTK)
  set(DCMTK_LIBRARY_DIR "" CACHE STRING 
      "Directory where DCMTK libraries are installed")
endif()

add_definitions(-DBOOST_ALL_DYN_LINK )
find_package(Boost COMPONENTS system filesystem program_options date_time REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

#Builds library mdm_utils
add_subdirectory(utils)

#Builds library mdm_opt
add_subdirectory(opt)

#Builds library mdm_image
add_subdirectory(image_io)

#Builds library mdm_dce
add_subdirectory(dce)

#Builds library mdm_t1
add_subdirectory(t1)

#Builds library mdm_dwi
add_subdirectory(dwi)

#Builds library mdm_run
add_subdirectory(run)

if ( BUILD_TOOLS )
	add_subdirectory(tools)
endif ()

if ( BUILD_QT_GUI )
	add_subdirectory(qt_gui)
endif ()

if ( BUILD_TESTING )
  add_subdirectory(tests)
endif ( BUILD_TESTING )

