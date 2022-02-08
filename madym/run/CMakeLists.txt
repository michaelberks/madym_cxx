# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

set(mdm_run_sources

	mdm_InputOptions.h
	mdm_RunTools.cxx			mdm_RunTools.h	
	mdm_FileManager.cxx			mdm_FileManager.h		
	mdm_ParamSummaryStats.cxx	mdm_ParamSummaryStats.h
	mdm_OptionsParser.cxx		mdm_OptionsParser.h
	mdm_VolumeAnalysis.cxx		mdm_VolumeAnalysis.h

  mdm_RunToolsDCEFit.cxx			mdm_RunToolsDCEFit.h
  mdm_RunToolsT1Fit.cxx				mdm_RunToolsT1Fit.h
  mdm_RunToolsVolumeAnalysis.cxx	mdm_RunToolsVolumeAnalysis.h
  mdm_RunTools_madym_DCE.cxx		mdm_RunTools_madym_DCE.h
  mdm_RunTools_madym_DCE_lite.cxx	mdm_RunTools_madym_DCE_lite.h
  mdm_RunTools_madym_T1.cxx			mdm_RunTools_madym_T1.h
  mdm_RunTools_madym_T1_lite.cxx	mdm_RunTools_madym_T1_lite.h
  mdm_RunTools_madym_DWI.cxx			mdm_RunTools_madym_DWI.h
  mdm_RunTools_madym_DWI_lite.cxx	mdm_RunTools_madym_DWI_lite.h
  mdm_RunTools_madym_AIF.cxx		mdm_RunTools_madym_AIF.h
  mdm_RunTools_madym_MakeXtr.cxx mdm_RunTools_madym_MakeXtr.h
)

if (BUILD_WITH_DCMTK)
  set(mdm_run_sources ${mdm_run_sources} 
    mdm_RunTools_madym_DicomConvert.cxx mdm_RunTools_madym_DicomConvert.h
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

