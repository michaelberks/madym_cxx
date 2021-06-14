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

if (BUILD_INSTALL)
  install(TARGETS madym_gui 
    BUNDLE DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT GUI
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT GUI
    CONFIGURATIONS Release)

  #For the different platforms we run some custom post build
  #commands to generate the necessary library dependencies 
  #that can be copied into the install packages
  if (WIN32)
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
        COMMENT "Running ${WINDEPLOYQT_EXECUTABLE} to get GUI dependencies..."
        CONFIGURATIONS Release
    )

    #Uncomment to generate dependencies in Debug mode
    #set(GUI_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/madym_gui.exe")
    #add_custom_command(TARGET madym_gui POST_BUILD
    #  COMMAND "${CMAKE_COMMAND}" -E
    #      env PATH="${QT_BIN_DIR}" "${WINDEPLOYQT_EXECUTABLE}" "${GUI_PATH}"
    #          --no-compiler-runtime
    #          --dir "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy_d"
    #  COMMENT "Running ${WINDEPLOYQT_EXECUTABLE} to get GUI dependencies..."
    #  CONFIGURATIONS Debug
    #)

  elseif (APPLE)
    #Configure mac bundle for the GUI tool
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
    set_target_properties(
        madym_gui
        PROPERTIES
        INSTALL_RPATH 
            "${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/lib;${Boost_LIBRARY_DIRS};${ZLIB_LIBRARY_DIR};${DCMTK_LIBRARY_DIR}"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        CONFIGURATIONS Release)

    #For MacOs we run the QT deploy tool on the install (not build) bundle
    get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")

    message(STATUS "mac deploy tool found at ${MACDEPLOYQT_EXECUTABLE}")
    install(CODE
        "execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} 
            \${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.app )
        "
    COMPONENT GUI
    CONFIGURATIONS Release)

  elseif(UNIX)
    #Find QMake executable
    get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    set(GUI_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/madym_gui")

    #Add custom command to run cqtdeployer to get GUI dependencies
    add_custom_command(TARGET madym_gui POST_BUILD
    COMMAND "cqtdeployer" 
            -bin "${GUI_PATH}"
            -qmake "${QMAKE_EXECUTABLE}"
            -targetDir "${CMAKE_BINARY_DIR}/qt_deploy"
            -name "madym-${MADYM_VERSION}"
    COMMENT "Running ${cqtdeployer} to get GUI dependencies..."
    CONFIGURATIONS Release
    )
    add_custom_command(TARGET madym_gui POST_BUILD
        COMMAND rm "${CMAKE_BINARY_DIR}/qt_deploy/bin/madym_gui"
        COMMENT "Removing unwanted binary, we just want the libraries and Qt plugins..."
        CONFIGURATIONS Release
        )
  endif()

  #All the rest of the install command wrapped in the install script
  include (${CMAKE_SOURCE_DIR}/madym/install.cmake)
endif()

