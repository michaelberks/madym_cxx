# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

if ( BUILD_SHARED_LIBS )
	add_definitions(-DMDM_BUILD_SHARED )
endif ( BUILD_SHARED_LIBS )

add_subdirectory(opt)

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

  #Wrap the program logger header
  QT5_WRAP_CPP( mdm_moc_files 
	  mdm_ProgramLogger.h
  )
else ()
  set(mdm_moc_files "")
endif ()

add_definitions(-DBOOST_ALL_DYN_LINK )
find_package(Boost COMPONENTS system filesystem program_options date_time REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option( BUILD_WITH_ZLIB "Include zlib to allow compressed NIFTI format (*.gz)?" NO )
if (BUILD_WITH_ZLIB)
  find_package(ZLIB)
  add_definitions(-DHAVE_ZLIB)
endif()

option( BUILD_WITH_DCMTK "Include support for DICOM images?" NO )
if (BUILD_WITH_DCMTK)
  find_package(DCMTK NO_MODULE REQUIRED)
  include_directories(${DCMTK_INCLUDE_DIRS})
  add_definitions(-DUSING_DCMTK)
endif()


set(mdm_sources
	mdm_RunTools.cxx				  mdm_RunTools.h
	mdm_AIF.cxx						    mdm_AIF.h
	mdm_ErrorTracker.cxx			mdm_ErrorTracker.h			
		
	mdm_DCEVoxel.cxx				  mdm_DCEVoxel.h
	mdm_DCEModelFitter.cxx		mdm_DCEModelFitter.h
		
	mdm_FileManager.cxx				mdm_FileManager.h
	mdm_Image3D.cxx					  mdm_Image3D.h
	mdm_ProgramLogger.cxx			mdm_ProgramLogger.h
	mdm_ParamSummaryStats.cxx	mdm_ParamSummaryStats.h

	mdm_T1Mapper.cxx	        mdm_T1Mapper.h	
  mdm_VolumeAnalysis.cxx	  mdm_VolumeAnalysis.h
	
	mdm_OptionsParser.cxx			mdm_OptionsParser.h
	mdm_InputTypes.cxx				mdm_InputTypes.h
	mdm_InputOptions.h
	mdm_platform_defs.h
  mdm_exception.h
	mdm_api.h
)
set(mdm_io_sources
  image_io/mdm_ImageDatatypes.h
  image_io/mdm_ImageIO.cxx image_io/mdm_ImageIO.h
  image_io/analyze/mdm_AnalyzeFormat.cxx image_io/analyze/mdm_AnalyzeFormat.h
  image_io/xtr/mdm_XtrFormat.cxx image_io/xtr/mdm_XtrFormat.h
  image_io/nifti/mdm_NiftiFormat.cxx image_io/nifti/mdm_NiftiFormat.h
  image_io/nifti/mdm_NiftiFormatAscii.cxx  image_io/nifti/mdm_NiftiFormatTransforms.cxx 
  image_io/nifti/znzlib.cpp image_io/nifti/znzlib.h
  image_io/nifti/nifti_swaps.h
	image_io/nifti/nifti1.h
  image_io/nifti/nifti2.h  
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
  run/mdm_RunTools_madym_MakeXtr.cxx run/mdm_RunTools_madym_MakeXtr.h
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

if (BUILD_WITH_DCMTK)
  set(mdm_io_sources ${mdm_io_sources} 
    image_io/dicom/mdm_DicomFormat.cxx image_io/dicom/mdm_DicomFormat.h
  )
  set(mdm_run_sources ${mdm_run_sources} 
    run/mdm_RunTools_madym_DicomConvert.cxx run/mdm_RunTools_madym_DicomConvert.h
  )
endif()

add_library(mdm 
	${mdm_sources} 
  ${mdm_io_sources}
  ${mdm_run_sources} 
  ${mdm_dce_model_sources} 
  ${mdm_t1_method_sources} 
  ${mdm_moc_files})

target_link_libraries( mdm
  mdm_opt
  Boost::filesystem
  Boost::system
  Boost::program_options
  Boost::date_time
)
if (BUILD_WITH_ZLIB)
  target_link_libraries( mdm ZLIB::ZLIB)
endif()

if (BUILD_WITH_DCMTK)
  target_link_libraries( mdm ${DCMTK_LIBRARIES})
endif()

if ( BUILD_QT_GUI )
  qt5_use_modules(mdm Core)
endif()

if (BUILD_INSTALL)
  install(TARGETS mdm
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()

if ( MSVC )
	target_link_libraries( mdm ws2_32 )
else ( MSVC )
	target_link_libraries( mdm m)
endif ( MSVC )

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

