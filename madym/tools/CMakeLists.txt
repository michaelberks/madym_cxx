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
#add_executable(dicom2analyze 
#	dicom2analyze.cxx)
#
#target_link_libraries( dicom2analyze mdm)

#-------------------------------------------------------------------
if ( BUILD_TESTING )
	subdirs(tests)
endif ( BUILD_TESTING )

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
if (BUILD_INSTALL)

  if (APPLE)
    set(APPLE_CODESIGN_ID "" CACHE STRING 
      "Apple developer ID for certificate used to code sign Apple targets")
  endif()
  #set(CPACK_ARCHIVE_FILE_NAME "madym_${MADYM_VERSION}-${CMAKE_GENERATOR}") this should work but docs are wrong: cmake bug

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

  #Install example data and config scripts
  install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples" DESTINATION "${MADYM_DEPLOY_DIR}"
      USE_SOURCE_PERMISSIONS
      COMPONENT Examples
      CONFIGURATIONS Release)

  #Only build the archive when we're not build the main GUI. If we build
  #the GUI, we make a proper installer package

  if (NOT BUILD_QT_GUI)

    set(CPACK_PACKAGE_FILE_NAME "madym_${MADYM_VERSION}-${CMAKE_GENERATOR}")
    set(CPACK_ARCHIVE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
      
    #Windows
    if (WIN32)
      message(STATUS "Set generator to zip for ${CPACK_ARCHIVE_FILE_NAME}")
      set(CPACK_GENERATOR "ZIP")

      #We need to include boost dlls for windows
      set(APPS "\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/madym_DCE.exe")
      set(DIRS 
        "${Boost_LIBRARY_DIR_RELEASE}")

      install(CODE "
          include(BundleUtilities)
          fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
          " 
          COMPONENT Tools
          CONFIGURATIONS Release)

    #Apple
    elseif(APPLE)
      
      #Sign the tools on Apple
      install(CODE
        "execute_process(COMMAND codesign -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_DCE )"
        "execute_process(COMMAND codesign -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_DCE_lite )"
        "execute_process(COMMAND codesign -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_T1 )"
        "execute_process(COMMAND codesign -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_T1_lite )"
        "execute_process(COMMAND codesign -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_AIF )"
        COMPONENT Tools
        CONFIGURATIONS Release)
      
      message(STATUS "Set generator to tgz for ${CPACK_ARCHIVE_FILE_NAME}")
      set(CPACK_GENERATOR "TGZ")

    #Unix
    elseif(UNIX)
      message(STATUS "Set generator to tgz for ${CPACK_ARCHIVE_FILE_NAME}")
      set(CPACK_GENERATOR "TGZ")

    endif ()

    set(CPACK_SOURCE_IGNORE_FILES
      \\.git/
      build/
      ".*~$"
    )

    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    set(CPACK_VERBATIM_VARIABLES YES)
    include(CPack)

  endif()
endif()
