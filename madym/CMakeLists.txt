# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

if ( BUILD_SHARED_LIBS )
	add_definitions(-DMDM_BUILD_SHARED )
endif ( BUILD_SHARED_LIBS )

add_definitions(-DBOOST_ALL_DYN_LINK )
find_package(Boost COMPONENTS system filesystem program_options date_time REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(opt)

set(mdm_sources
	mdm_RunTools.cxx				  mdm_RunTools.h
	mdm_AIF.cxx						    mdm_AIF.h
	mdm_AnalyzeFormat.cxx			mdm_AnalyzeFormat.h
	mdm_ErrorTracker.cxx			mdm_ErrorTracker.h			
		
	mdm_DCEVolumeAnalysis.cxx	mdm_DCEVolumeAnalysis.h
	mdm_DCEVoxel.cxx				  mdm_DCEVoxel.h
	mdm_DCEModelFitter.cxx		mdm_DCEModelFitter.h
		
	mdm_FileManager.cxx				mdm_FileManager.h
	mdm_Image3D.cxx					  mdm_Image3D.h
	mdm_ProgramLogger.cxx			mdm_ProgramLogger.h
	mdm_ParamSummaryStats.cxx	mdm_ParamSummaryStats.h

	mdm_T1VolumeAnalysis.cxx	mdm_T1VolumeAnalysis.h	

	mdm_OptionsParser.cxx			mdm_OptionsParser.h
	mdm_InputTypes.cxx				mdm_InputTypes.h
	mdm_InputOptions.h
	mdm_platform_defs.h
	mdm_api.h
)
set(mdm_run_sources
  run/mdm_RunToolsDCEFit.cxx			run/mdm_RunToolsDCEFit.h
  run/mdm_RunToolsT1Fit.cxx				run/mdm_RunToolsT1Fit.h
  run/mdm_RunToolsVolumeAnalysis.cxx	run/mdm_RunToolsVolumeAnalysis.h
  run/mdm_RunTools_madym_DCE.cxx		run/mdm_RunTools_madym_DCE.h
  run/mdm_RunTools_madym_DCE_lite.cxx	run/mdm_RunTools_madym_DCE_lite.h
  run/mdm_RunTools_madym_T1.cxx			run/mdm_RunTools_madym_T1.h
  run/mdm_RunTools_madym_T1_lite.cxx	run/mdm_RunTools_madym_T1_lite.h
  run/mdm_RunTools_madym_AIF.cxx		run/mdm_RunTools_madym_AIF.h
)
set(mdm_dce_model_sources
  #Models
  dce_models/mdm_DCEModelGenerator.h
  dce_models/mdm_DCEModelBase.cxx		dce_models/mdm_DCEModelBase.h
  dce_models/mdm_DCEModelNONE.cxx		dce_models/mdm_DCEModelNONE.h
  dce_models/mdm_DCEModelETM.cxx		dce_models/mdm_DCEModelETM.h
  dce_models/mdm_DCEModelDIETM.cxx		dce_models/mdm_DCEModelDIETM.h
  dce_models/mdm_DCEModelAUEM.cxx		dce_models/mdm_DCEModelAUEM.h
  dce_models/mdm_DCEModel2CXM.cxx		dce_models/mdm_DCEModel2CXM.h
  dce_models/mdm_DCEModelDI2CXM.cxx		dce_models/mdm_DCEModelDI2CXM.h
  dce_models/mdm_DCEModelDIBEM.cxx		dce_models/mdm_DCEModelDIBEM.h
  dce_models/mdm_DCEModelDIBEM_Fp.cxx	dce_models/mdm_DCEModelDIBEM_Fp.h
  dce_models/mdm_DCEModelDISCM.cxx		dce_models/mdm_DCEModelDISCM.h
)

set(mdm_t1_method_sources
	t1_methods/mdm_T1MethodGenerator.h
	t1_methods/mdm_T1FitterBase.cxx			t1_methods/mdm_T1FitterBase.h
	t1_methods/mdm_T1FitterVFA.cxx			t1_methods/mdm_T1FitterVFA.h
)

add_library(mdm 
	${mdm_sources} ${mdm_run_sources} ${mdm_dce_model_sources} ${mdm_t1_method_sources})

target_link_libraries( mdm
  mdm_opt
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
)

if ( MSVC )
	target_link_libraries( mdm ws2_32 )
else ( MSVC )
	target_link_libraries( mdm m)
endif ( MSVC )

install(TARGETS mdm
  ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}" COMPONENT Libs
  LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}" COMPONENT Libs
  RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}" COMPONENT Libs
  CONFIGURATIONS Release)

option( BUILD_TOOLS "Should the main madym executable tools be built?" YES )
option( BUILD_QT_GUI "Should the madym GUI be built?" NO )

if ( BUILD_TOOLS )
	subdirs(tools)
endif ()

if ( BUILD_QT_GUI )
	subdirs(qt_gui)
endif ()

if ( BUILD_TESTING )
  subdirs(tests)
endif ( BUILD_TESTING )

#if ( BUILD_PYTHON )
#	subdirs(python)
#endif ( BUILD_PYTHON )

