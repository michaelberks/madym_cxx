# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym_tools)

#-------------------------------------------------------------------
# Main madym analysis tool applied to whole image volumes
add_executable(madym_DCE 
	madym_DCE.cxx)

target_link_libraries( madym_DCE mdm
  )

#-------------------------------------------------------------------
# Lightweight tool for performing analysis on individual voxels
add_executable(madym_DCE_lite 
	madym_DCE_lite.cxx)

target_link_libraries( madym_DCE_lite mdm)

#-------------------------------------------------------------------
# Standalone tool for computing T1 maps
add_executable(madym_T1 
	madym_T1.cxx)

target_link_libraries( madym_T1 mdm)

#-------------------------------------------------------------------
# Standalone tool for computing T1 values
add_executable(madym_T1_lite 
	madym_T1_lite.cxx)

target_link_libraries( madym_T1_lite mdm)

#-------------------------------------------------------------------
# Standalone tool for computing an auto AIF
add_executable(madym_AIF 
	madym_AIF.cxx)

target_link_libraries( madym_AIF mdm)

#-------------------------------------------------------------------
# Standalone tool for converting raw dicom images to analyze format volumes
if (BUILD_WITH_DCMTK)
  add_executable(madym_DicomConvert 
	  madym_DicomConvert.cxx)

  target_link_libraries( madym_DicomConvert mdm)
endif()

#-------------------------------------------------------------------
# Standalone tool for making xtr files
add_executable(madym_MakeXtr 
	madym_MakeXtr.cxx)

target_link_libraries( madym_MakeXtr mdm)

#-------------------------------------------------------------------
if ( BUILD_TESTING )
	subdirs(tests)
endif ( BUILD_TESTING )

if (BUILD_INSTALL)
  install(TARGETS madym_DCE 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)
  install(TARGETS madym_DCE_lite 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)
  install(TARGETS madym_T1 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)
  install(TARGETS madym_T1_lite 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)
  install(TARGETS madym_AIF 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)
  install(TARGETS madym_MakeXtr 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)

  if (BUILD_WITH_DCMTK)
      install(TARGETS madym_DicomConvert 
          RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
          CONFIGURATIONS Release)
  endif()

  if (NOT BUILD_QT_GUI)
    #All the rest of the install command wrapped in the install script - only
    #run this if we're not building the GUI
    include (${CMAKE_SOURCE_DIR}/madym/install.cmake)
  endif()
endif()

