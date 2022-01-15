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

set(mdm_utils_sources

	mdm_api.h
	mdm_Image3D.cxx				mdm_Image3D.h
	mdm_ErrorTracker.h		mdm_ErrorTracker.cxx
	mdm_exception.h
	mdm_InputTypes.h		mdm_InputTypes.cxx
	mdm_platform_defs.h
	mdm_ProgramLogger.h		mdm_ProgramLogger.cxx
	mdm_SequenceNames.h
	
)

add_library(mdm_utils 
	${mdm_utils_sources}
	${mdm_moc_files})

target_link_libraries( mdm_utils
  Boost::filesystem
  Boost::system
  Boost::program_options
  Boost::date_time
)

if ( BUILD_QT_GUI )
  qt5_use_modules(mdm_utils Core)
endif()

if ( MSVC )
	target_link_libraries( mdm_utils ws2_32 )
else ( MSVC )
	target_link_libraries( mdm_utils m)
endif ( MSVC )

set(mdm_image_sources
	
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

if (BUILD_WITH_DCMTK)
  set(mdm_image_sources ${mdm_image_sources} 
    image_io/dicom/mdm_DicomFormat.cxx image_io/dicom/mdm_DicomFormat.h
  )
endif()

add_library(mdm_image 
	${mdm_image_sources})

target_link_libraries( mdm_image
  mdm_utils
)

if (BUILD_WITH_ZLIB)
  target_link_libraries( mdm_image ZLIB::ZLIB)
endif()

if (BUILD_WITH_DCMTK)
  target_link_libraries( mdm_image ${DCMTK_LIBRARIES})
endif()

set(mdm_dce_sources
  #Models
	dce/mdm_DCEModelGenerator.h
	dce/mdm_DCEModelBase.cxx		dce/mdm_DCEModelBase.h
	dce/mdm_DCEModelNONE.cxx		dce/mdm_DCEModelNONE.h
	dce/mdm_DCEModelETM.cxx		dce/mdm_DCEModelETM.h
	dce/mdm_DCEModelDIETM.cxx		dce/mdm_DCEModelDIETM.h
	dce/mdm_DCEModelAUEM.cxx		dce/mdm_DCEModelAUEM.h
	dce/mdm_DCEModel2CXM.cxx		dce/mdm_DCEModel2CXM.h
	dce/mdm_DCEModel2CFM.cxx		dce/mdm_DCEModel2CFM.h
	dce/mdm_DCEModelDI2CXM.cxx		dce/mdm_DCEModelDI2CXM.h
	dce/mdm_DCEModelDIBEM.cxx		dce/mdm_DCEModelDIBEM.h
	dce/mdm_DCEModelDIBEM_Fp.cxx	dce/mdm_DCEModelDIBEM_Fp.h
	dce/mdm_DCEModelDISCM.cxx		dce/mdm_DCEModelDISCM.h
	dce/mdm_DCEModelPatlak.cxx		dce/mdm_DCEModelPatlak.h
	dce/mdm_DCEVoxel.cxx			dce/mdm_DCEVoxel.h
	dce/mdm_DCEModelFitter.cxx		dce/mdm_DCEModelFitter.h
	mdm_AIF.cxx					mdm_AIF.h
	mdm_Exponentials.h
)

add_library(mdm_dce 
	${mdm_dce_sources})

target_link_libraries( mdm_dce
  mdm_image
  mdm_opt
)

set(mdm_t1_sources
	t1/mdm_T1Mapper.cxx	        t1/mdm_T1Mapper.h
	t1/mdm_T1MethodGenerator.h
	t1/mdm_T1FitterBase.cxx			t1/mdm_T1FitterBase.h
	t1/mdm_T1FitterVFA.cxx			t1/mdm_T1FitterVFA.h
	t1/mdm_T1FitterIR.cxx			t1/mdm_T1FitterIR.h
)

add_library(mdm_t1 
	${mdm_t1_sources})

target_link_libraries( mdm_t1
  mdm_image
  mdm_opt
)

set(mdm_dwi_sources
	dwi/mdm_DWIMapper.cxx	        dwi/mdm_DWIMapper.h
	dwi/mdm_DWIMethodGenerator.h
	dwi/mdm_DWIFitterBase.cxx			dwi/mdm_DWIFitterBase.h
	dwi/mdm_DWIFitterADC.cxx			dwi/mdm_DWIFitterADC.h
  dwi/mdm_DWIFitterIVIM.cxx			dwi/mdm_DWIFitterIVIM.h
)

add_library(mdm_dwi 
	${mdm_dwi_sources})

target_link_libraries( mdm_dwi
  mdm_image
  mdm_opt
)

set(mdm_run_sources

	mdm_InputOptions.h
	mdm_RunTools.cxx			mdm_RunTools.h	
	mdm_FileManager.cxx			mdm_FileManager.h		
	mdm_ParamSummaryStats.cxx	mdm_ParamSummaryStats.h
	mdm_OptionsParser.cxx		mdm_OptionsParser.h
	mdm_VolumeAnalysis.cxx		mdm_VolumeAnalysis.h

  run/mdm_RunToolsDCEFit.cxx			run/mdm_RunToolsDCEFit.h
  run/mdm_RunToolsT1Fit.cxx				run/mdm_RunToolsT1Fit.h
  run/mdm_RunToolsVolumeAnalysis.cxx	run/mdm_RunToolsVolumeAnalysis.h
  run/mdm_RunTools_madym_DCE.cxx		run/mdm_RunTools_madym_DCE.h
  run/mdm_RunTools_madym_DCE_lite.cxx	run/mdm_RunTools_madym_DCE_lite.h
  run/mdm_RunTools_madym_T1.cxx			run/mdm_RunTools_madym_T1.h
  run/mdm_RunTools_madym_T1_lite.cxx	run/mdm_RunTools_madym_T1_lite.h
  run/mdm_RunTools_madym_DWI.cxx			run/mdm_RunTools_madym_DWI.h
  run/mdm_RunTools_madym_DWI_lite.cxx	run/mdm_RunTools_madym_DWI_lite.h
  run/mdm_RunTools_madym_AIF.cxx		run/mdm_RunTools_madym_AIF.h
  run/mdm_RunTools_madym_MakeXtr.cxx run/mdm_RunTools_madym_MakeXtr.h
)

if (BUILD_WITH_DCMTK)
  set(mdm_run_sources ${mdm_run_sources} 
    run/mdm_RunTools_madym_DicomConvert.cxx run/mdm_RunTools_madym_DicomConvert.h
  )
endif()

add_library(mdm
	${mdm_run_sources})

target_link_libraries( mdm
  mdm_dce
  mdm_t1
  mdm_dwi
)

if (BUILD_INSTALL)
  install(TARGETS mdm
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()

if ( BUILD_TOOLS )
	subdirs(tools)
endif ()

if ( BUILD_QT_GUI )
	subdirs(qt_gui)
endif ()

if ( BUILD_TESTING )
  subdirs(tests)
endif ( BUILD_TESTING )

