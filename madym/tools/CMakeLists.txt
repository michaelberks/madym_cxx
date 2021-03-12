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
  install(TARGETS madym_MakeXtr 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
        CONFIGURATIONS Release)
  if (BUILD_WITH_DCMTK)
    install(TARGETS madym_DicomConvert 
      RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Tools
      CONFIGURATIONS Release)
  endif()

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

    #Cpack options common to all systems
    set(CPACK_PACKAGE_NAME "Madym")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Madym DCE-MRI analysis tools")
    set(CPACK_COMPONENTS_ALL GUI Tools Libs Examples)
    set(CPACK_PACKAGE_VENDOR "University of Manchester")
    set(CPACK_PACKAGE_CONTACT "michael.berks@manchester.ac.uk")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Madym")
    set(CPACK_PACKAGE_EXECUTABLES "madym_gui" "Madym")
    set(CPACK_PACKAGE_VERSION_MAJOR "${MADYM_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${MADYM_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${MADYM_VERSION_PATCH}")

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
      install(CODE "
        set(LIB_PATH \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\")
        set(RUN_PATH \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\")
        set(LIB_DEST \"\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/lib\")
        "
        COMPONENT Tools
        CONFIGURATIONS Release)

      if (BUILD_WITH_DCMTK)
        install(CODE "
        set(MADYM_EXE \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/madym_DicomConvert\")
        "
        COMPONENT Tools
        CONFIGURATIONS Release)
      else()
        install(CODE "
        set(MADYM_EXE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/madym_DCE)
        "
        COMPONENT Tools
        CONFIGURATIONS Release)
      endif()
      install(CODE [[
        message(STATUS "Getting dependencies for ${MADYM_EXE}")
        file(GET_RUNTIME_DEPENDENCIES
          EXECUTABLES ${MADYM_EXE}
          RESOLVED_DEPENDENCIES_VAR _r_deps
          UNRESOLVED_DEPENDENCIES_VAR _u_deps
          PRE_EXCLUDE_REGEXES "(^/lib/.*$)" 
          POST_EXCLUDE_REGEXES "(^/lib/.*$)")
        foreach(_file ${_r_deps})          
          file(INSTALL
            DESTINATION ${LIB_DEST}
            TYPE SHARED_LIBRARY
            FILES "${_file}"
          )
          get_filename_component(_resolvedFile "${_file}" REALPATH)
          if (NOT "${_file}" STREQUAL "${_resolvedFile}")
            file(INSTALL
              DESTINATION ${LIB_DEST}
              TYPE SHARED_LIBRARY
              FILES "${_resolvedFile}"
            )
          endif()
        endforeach()
        list(LENGTH _u_deps _u_length)
        if("${_u_length}" GREATER 0)
          message(WARNING "Unresolved dependencies detected!")
        endif()
        ]]
        COMPONENT Tools
        CONFIGURATIONS Release)

      message(STATUS "Set generator to deb;tgz for ${CPACK_ARCHIVE_FILE_NAME}")
      set(CPACK_GENERATOR "DEB;TGZ")
      set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.png")
      set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
      set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Michael Berks")
      set(CPACK_DEBIAN_FILE_NAME "madym_${MADYM_VERSION}.deb")
      set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui")
      set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Madym debian package")

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
