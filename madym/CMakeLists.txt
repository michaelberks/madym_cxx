# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

cmake_minimum_required(VERSION 2.8)
SET (BUILD_PYTHON OFF CACHE BOOL "build python wrappers")

IF ( BUILD_SHARED_LIBS )
	ADD_DEFINITIONS(-DMDM_BUILD_SHARED )
ENDIF ( BUILD_SHARED_LIBS )

ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK )
FIND_PACKAGE(Boost COMPONENTS system filesystem program_options REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY(opt)

SET(mdm_sources
		mdm_RunTools.cxx				mdm_RunTools.h
		mdm_AIF.cxx						mdm_AIF.h
		mdm_AnalyzeFormat.cxx			mdm_AnalyzeFormat.h
		mdm_ErrorTracker.cxx			mdm_ErrorTracker.h		
		
		mdm_DCEModelGenerator.h
		mdm_DCEModelBase.cxx			mdm_DCEModelBase.h
		mdm_DCEModelETM.cxx				mdm_DCEModelETM.h
		mdm_DCEModelDIETM.cxx			mdm_DCEModelDIETM.h
		mdm_DCEModelAUEM.cxx			mdm_DCEModelAUEM.h
		mdm_DCEModel2CXM.cxx			mdm_DCEModel2CXM.h
		mdm_DCEModelDI2CXM.cxx			mdm_DCEModelDI2CXM.h
		mdm_DCEModelDIBEM.cxx			mdm_DCEModelDIBEM.h
		mdm_DCEModelDIBEM_Fp.cxx		mdm_DCEModelDIBEM_Fp.h
		mdm_DCEModelDISCM.cxx			mdm_DCEModelDISCM.h
		mdm_DCEVolumeAnalysis.cxx		mdm_DCEVolumeAnalysis.h
		mdm_DCEVoxel.cxx				mdm_DCEVoxel.h
		
		mdm_FileManager.cxx				mdm_FileManager.h
		mdm_Image3D.cxx					mdm_Image3D.h
		mdm_ProgramLogger.cxx			mdm_ProgramLogger.h

		mdm_T1Voxel.cxx					mdm_T1Voxel.h
		mdm_T1VolumeAnalysis.cxx		mdm_T1VolumeAnalysis.h	

		mdm_OptionsParser.cxx			mdm_OptionsParser.h
		mdm_InputTypes.cxx				mdm_InputTypes.h
										mdm_InputOptions.h
										mdm_platform_defs.h
										mdm_api.h
)

ADD_LIBRARY(mdm 
	${mdm_sources})

TARGET_LINK_LIBRARIES( mdm LINK_PUBLIC 
  mdm_opt
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

IF ( MSVC )
	TARGET_LINK_LIBRARIES( mdm ws2_32 )
ELSE ( MSVC )
	TARGET_LINK_LIBRARIES( mdm m)
ENDIF ( MSVC )

OPTION( BUILD_TOOLS "Should the main madym executable tools be built?" YES )
OPTION( BUILD_QT_GUI "Should the madym GUI be built?" NO )

if ( BUILD_TOOLS )
	SUBDIRS(tools)
endif ()

if ( BUILD_QT_GUI )
	SUBDIRS(qt_gui)
endif ()

if ( BUILD_TESTING )
  SUBDIRS(tests)
endif ( BUILD_TESTING )

#if ( BUILD_PYTHON )
#	SUBDIRS(python)
#endif ( BUILD_PYTHON )

