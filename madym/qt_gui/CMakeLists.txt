project(madym_gui)

# Add folder where .h files are created (from the .ui files) to the search path
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# Convert these .ui files to .h
QT5_WRAP_UI( madym_gui_ui_h_files
	madym_gui.ui
	madym_model_configure.ui
)	

# Convert these .h files to .cxx
QT5_WRAP_CPP( madym_gui_moc_files 
	madym_gui_ui.h
	madym_gui_processor.h
	madym_gui_model_configure.h
)

# Convert these resource files
QT5_ADD_RESOURCES( madym_gui_res_files
	madym_gui.qrc	
)

set(madym_gui_sources
	madym_gui.cxx
	madym_gui_ui.h madym_gui_ui.cxx 
	madym_gui_processor.h madym_gui_processor.cxx
	madym_gui_model_configure.h madym_gui_model_configure.cxx
  madym_gui.rc
)

if ( APPLE )
  set(MADYM_ICON ${CMAKE_CURRENT_SOURCE_DIR}/images/madym.icns)
  set_source_files_properties(${MADYM_ICON} PROPERTIES
         MACOSX_PACKAGE_LOCATION "Resources")

  add_executable( madym_gui
  MACOSX_BUNDLE
    ${madym_gui_sources}
    ${madym_gui_ui_h_files}
    ${madym_gui_moc_files} 
    ${madym_gui_res_files}
    ${MADYM_ICON}
  )

else()
  add_executable( madym_gui
    ${madym_gui_sources}
    ${madym_gui_ui_h_files}
    ${madym_gui_moc_files} 
    ${madym_gui_res_files}
  )
endif()

target_link_libraries( madym_gui mdm)

qt5_use_modules(madym_gui Widgets)
qt5_use_modules(madym_gui Core)
qt5_use_modules(madym_gui Gui)
  
#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------

if (BUILD_INSTALL)

  if ( WIN32 )
    #Use windeployqt to generate Qt dependencies post release build
    #these can then be installed in to the deploy dir below
    get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    message(STATUS "Running ")
    set(GUI_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/madym_gui.exe")

    add_custom_command(TARGET madym_gui POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E
          env PATH="${QT_BIN_DIR}" "${WINDEPLOYQT_EXECUTABLE}" "${GUI_PATH}"
              --no-compiler-runtime
              --dir "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy"
      COMMENT "Running ${WINDEPLOYQT_EXECUTABLE} to get GUI dependencies..."
      CONFIGURATIONS Release
    )

  elseif ( APPLE )
    #Configure mac bundle
    set(MACOSX_BUNDLE_BUNDLE_NAME "madym_gui")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${MADYM_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.qbi-lab.madym")
    set(MACOSX_BUNDLE_ICON_FILE madym.icns)
    set(MACOSX_BUNDLE_INFO_STRING "Madym")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "madym_${MADYM_VERSION}-${CMAKE_GENERATOR}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${MADYM_VERSION}")

    #Override global rpath properties - hardcode to the original libs
    #then the Qt deploy tool will do the relative rpath replacement 
    message(STATUS "Setting gui RPATH to ${Boost_LIBRARY_DIRS};${ZLIB_LIBRARY_DIR};${DCMTK_LIBRARY_DIR}")
    set_target_properties(madym_gui
      PROPERTIES
      INSTALL_RPATH 
      "${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/lib;${Boost_LIBRARY_DIRS};${ZLIB_LIBRARY_DIR};${DCMTK_LIBRARY_DIR}"
      XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
      CONFIGURATIONS Release)
    
  endif()

  install(TARGETS madym_gui 
    BUNDLE DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT GUI
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT GUI
      CONFIGURATIONS Release)

  set(CPACK_PACKAGE_FILE_NAME "madym_${MADYM_VERSION}-${CMAKE_SYSTEM}")

  #Cpack options common to all systems
  set(CPACK_PACKAGE_NAME "Madym")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Madym DCE-MRI analysis tools")
  set(CPACK_COMPONENTS_ALL GUI Tools Libs Examples)
  set(CPACK_PACKAGE_VENDOR "University of Manchester")
  set(CPACK_PACKAGE_CONTACT "michael.berks@manchester.ac.uk")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Madym")
  set(CPACK_PACKAGE_EXECUTABLES "madym_gui" "Madym")
  set(CPACK_PACKAGE_VERSION_MAJOR "${MADYM_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${MADYM_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${MADYM_VERSION_PATCH}")

  #Windows
  if ( WIN32 )
    message(STATUS "Set generator to NSIS for ${CPACK_ARCHIVE_FILE_NAME}")
    set(CPACK_GENERATOR "NSIS")
    
    #Install Qt dependencies
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy/" DESTINATION "${MADYM_DEPLOY_DIR}/bin"
        COMPONENT GUI
        CONFIGURATIONS Release)

    #Install boost dependencies using fixup bundle
    install(CODE "
      include(BundleUtilities)
      fixup_bundle(\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.exe 
      \"\" \"${Boost_LIBRARY_DIR_RELEASE}\") 
      " 
      COMPONENT GUI
      CONFIGURATIONS Release)

    #Install Windows runtime dependencies
    set(CMAKE_INSTALL_MFC_LIBRARIES TRUE)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin")
    include(InstallRequiredSystemLibraries)

    #Set Cpack variable to cofigure NSIS installer
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images\\\\madym.ico")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images\\\\madym.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${MADYM_DEPLOY_DIR}\\\\bin\\\\madym_gui.exe")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${MADYM_DEPLOY_DIR}\\\\bin")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_NSIS_HELP_LINK "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui")
    set(CPACK_NSIS_CONTACT "michael.berks@manchester.ac.uk")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MENU_LINKS
      "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui" "Madym user guide (online)"
      "http://qbi-lab.org" "QBI Lab Website")
    set(CPACK_NSIS_MODIFY_PATH ON)
    
    set(CPACK_CREATE_DESKTOP_LINKS "madym_gui" "Madym")

  #Apple
  elseif ( APPLE )
    #For MacOs we run the QT deploy tool on the install (not build) bundle
    get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")

    message(STATUS "mac deploy tool found at ${MACDEPLOYQT_EXECUTABLE}")

    #Note the backslash before ${CMAKE_INSTALL_PREFIX} - this delays the resolving of the 
    #variable until install time, so Cpack packages the deploy GUI app bundle
    install(CODE
      "execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} 
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.app )
      message(STATUS \"Code-signing GUI using ID ${APPLE_CODESIGN_ID}\")
      execute_process(COMMAND codesign --deep -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.app )"
      COMPONENT GUI
      CONFIGURATIONS Release)

    # Transfer the value of ${MY_DEPENDENCY_PATHS} into the install script
    install(CODE "
      set(LIB_PATH \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\")
      set(RUN_PATH \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\")
      set(LIB_DEST \"\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/lib\")
      set(GUI_DEST \"\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.app\")
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
        UNRESOLVED_DEPENDENCIES_VAR _u_deps)
      foreach(_file ${_r_deps})
        file(INSTALL
          DESTINATION ${LIB_DEST}
          TYPE SHARED_LIBRARY
          FILES "${_file}"
        )
      endforeach()
      list(LENGTH _u_deps _u_length)
      if("${_u_length}" GREATER 0)
        message(WARNING "Unresolved dependencies detected!")
      endif()
      ]]
      COMPONENT Tools
      CONFIGURATIONS Release)

    install(CODE [[
      file(INSTALL
          DESTINATION ${LIB_DEST}
          FILES ${GUI_DEST}/Contents/Frameworks/QtCore.framework
      )
      ]]
      COMPONENT Tools
      CONFIGURATIONS Release)

    install(CODE
      "message(STATUS \"Code-signing cmd line tools using ID ${APPLE_CODESIGN_ID}\")
      execute_process(COMMAND codesign -s 
          ${APPLE_CODESIGN_ID}
          \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_DCE )
      execute_process(COMMAND codesign -s 
        ${APPLE_CODESIGN_ID}
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_DCE_lite )
      execute_process(COMMAND codesign -s 
        ${APPLE_CODESIGN_ID}
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_T1 )
      execute_process(COMMAND codesign -s 
        ${APPLE_CODESIGN_ID}
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_T1_lite )
      execute_process(COMMAND codesign -s 
        ${APPLE_CODESIGN_ID}
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_AIF )
      execute_process(COMMAND codesign -s 
        ${APPLE_CODESIGN_ID}
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_MakeXtr )"
      COMPONENT Tools
      CONFIGURATIONS Release)  

    if (BUILD_WITH_DCMTK)
      install(CODE
      "execute_process(COMMAND codesign -s 
        ${APPLE_CODESIGN_ID}
        \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_DicomConvert )"
      COMPONENT Tools
      CONFIGURATIONS Release)  
    endif()

    set(CPACK_ARCHIVE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
    message(STATUS "Set generator to tgz for ${CPACK_ARCHIVE_FILE_NAME}")
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.icns")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.icns")

  #Unix
  elseif ( UNIX )
    set(CPACK_ARCHIVE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
    message(STATUS "Set generator to tgz for ${CPACK_ARCHIVE_FILE_NAME}")
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.png")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

  endif()

  include(CPack)
endif()