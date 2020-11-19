project(madym_gui)

# Add folder where .h files are created (from the .ui files) to the search path
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

find_package(Qt5Core)
find_package(Qt5Widgets)

#Suppress annoying CMake warnings by setting these Qt related policies
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)

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
  add_executable( madym_gui
  MACOSX_BUNDLE
    ${madym_gui_sources}
    ${madym_gui_ui_h_files}
    ${madym_gui_moc_files} 
    ${madym_gui_res_files}
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
  
if ( WIN32 )
  #Use windeployqt to generate Qt dependencies post release build
  #these can then be installed in to the deploy dir below
  get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
  set(GUI_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/madym_gui.exe")

  add_custom_command(TARGET madym_gui POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${QT_BIN_DIR}" "${WINDEPLOYQT_EXECUTABLE}" "${GUI_PATH}"
            --no-compiler-runtime
            --dir "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy"
    COMMENT "Deploying Qt..."
    CONFIGURATIONS Release
  )

elseif ( APPLE )
  #Configure mac bundle
  set(MACOSX_BUNDLE_BUNDLE_NAME "madym_gui")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${MADYM_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.qbi-lab.madym")
  set(MACOSX_BUNDLE_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.png")
  set(MACOSX_BUNDLE_INFO_STRING "Madym")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "madym_${MADYM_VERSION}-${CMAKE_GENERATOR}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${MADYM_VERSION}")

  #Use windeployqt to generate Qt dependencies post release build
  #these can then be installed in to the deploy dir below
  get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
  find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
  set(GUI_PATH "madym_gui.app")

message(STATUS "mac deploy tool found at ${MACDEPLOYQT_EXECUTABLE}")
  add_custom_command(TARGET madym_gui POST_BUILD
    COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${GUI_PATH}"
    COMMENT "Deploying Qt..."
    CONFIGURATIONS Release
  )
endif()

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------

install(TARGETS madym_gui 
  BUNDLE DESTINATION "${MADYM_DEPLOY_DIR}" COMPONENT GUI
  RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}" COMPONENT GUI
    CONFIGURATIONS Release)

set(CPACK_PACKAGE_FILE_NAME "madym_${MADYM_VERSION}-${CMAKE_SYSTEM}")

#Cpack options common to all systems
set(CPACK_PACKAGE_NAME "Madym")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Madym DCE-MRI analysis tools")
set(CPACK_COMPONENTS_ALL GUI Tools Libs)
set(CPACK_PACKAGE_VENDOR "University of Manchester")
set(CPACK_PACKAGE_CONTACT "michael.berks@manchester.ac.uk")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Madym")
set(CPACK_PACKAGE_EXECUTABLES "madym_gui" "Madym")
set(CPACK_PACKAGE_VERSION_MAJOR "${MADYM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MADYM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MADYM_VERSION_PATCH}")

#Windows
if ( WIN32 )
  message(STATUS "Set generator to NSIS for ${CPACK_ARCHIVE_FILE_NAME}")
  set(CPACK_GENERATOR "NSIS")
  
  #Install Windows runtime dependencies
  set(CMAKE_INSTALL_MFC_LIBRARIES TRUE)
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}")
  include(InstallRequiredSystemLibraries)

  #Install Qt dependencies
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy/" DESTINATION "${MADYM_DEPLOY_DIR}"
      COMPONENT GUI
      CONFIGURATIONS Release)

  #Install boost dependencies using fixup bundle
  install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${GUI_PATH}\" \"\" \"${Boost_LIBRARY_DIR_RELEASE}\") 
    " 
    COMPONENT GUI
    CONFIGURATIONS Release)

  #Set Cpack variable to cofigure NSIS installer
  set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images\\\\madym.ico")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images\\\\madym.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "${MADYM_DEPLOY_DIR}/madym_gui.exe")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${MADYM_DEPLOY_DIR}")
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
  set(CPACK_ARCHIVE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
  message(STATUS "Set generator to tgz for ${CPACK_ARCHIVE_FILE_NAME}")
  set(CPACK_GENERATOR "DragNDrop;TGZ")
  set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.png")

#Unix
elseif ( UNIX )
  set(CPACK_ARCHIVE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
  message(STATUS "Set generator to tgz for ${CPACK_ARCHIVE_FILE_NAME}")
  set(CPACK_GENERATOR "DEB;TGZ")
  set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/madym.png")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

endif()

include(CPack)