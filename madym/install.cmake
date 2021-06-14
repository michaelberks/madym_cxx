#-------------------------------------------------------------------------
# Script containing CMake instructions for installing and packaging Madym
# Many of these are
# 1) Platform dependent
# 2) Dependent on whether or not we're building the Qt GUI 
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
#Install python wrappers

#First run version script to generate python version file
install(CODE
        "execute_process(
            COMMAND ${CMAKE_SOURCE_DIR}/python/version.sh 
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
            )"
        COMPONENT Python
        CONFIGURATIONS Release)

#Now copy over the python files from source to deploy
install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/python" DESTINATION "${MADYM_DEPLOY_DIR}"
    USE_SOURCE_PERMISSIONS
    COMPONENT Python
    CONFIGURATIONS Release
    PATTERN "build" EXCLUDE
    PATTERN "dist" EXCLUDE
    PATTERN "__pycache__" EXCLUDE
    PATTERN "*.egg-info" EXCLUDE
    )

#-------------------------------------------------------------------------
#Install example data
#Install example data and config scripts
install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/examples" DESTINATION "${MADYM_DEPLOY_DIR}"
    USE_SOURCE_PERMISSIONS
    COMPONENT Examples
    CONFIGURATIONS Release)

#-------------------------------------------------------------------------
# Now do platform specific dependency linking

#-------------------------------------------------------------------------
#Windows
if (WIN32)
    if (BUILD_QT_GUI)
        
        #Install Qt dependencies
        install(
            DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy/" DESTINATION "${MADYM_DEPLOY_DIR}/bin"
            COMPONENT GUI
            CONFIGURATIONS Release)

        #Install boost dependencies using fixup bundle
        set(DIRS 
            ${Boost_LIBRARY_DIR_RELEASE} ${ZLIB_LIBRARY_DIR} ${ZLIB_BIN_DIR} ${DCMTK_LIBRARY_DIR})
        message("Fixup bundle search paths: ${DIRS}")
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.exe 
            \"\" \"${DIRS}\") 
            " 
            COMPONENT GUI
            CONFIGURATIONS Release)

        #Install Windows runtime dependencies
        set(CMAKE_INSTALL_MFC_LIBRARIES TRUE)
        set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin")
        include(InstallRequiredSystemLibraries)

    else() #NOT BUILD_QT_GUI

        #We need to include boost dlls for windows
        set(APPS "\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/madym_DCE.exe")
        set(DIRS "${Boost_LIBRARY_DIR_RELEASE}")
    
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
            " 
            COMPONENT Tools
            CONFIGURATIONS Release)
    endif()

#-------------------------------------------------------------------------
#Apple
elseif (APPLE)

    #Copy the post-install script we use to manually set MADYM_ROOT on Apple
    install(
        PROGRAMS "${CMAKE_BINARY_DIR}/generated/madym_setup.sh" DESTINATION "${MADYM_DEPLOY_DIR}"
        COMPONENT Tools
        CONFIGURATIONS Release)

    # Transfer the value of ${MY_DEPENDENCY_PATHS} into the install script
    install(CODE "
      set(LIB_DEST \"\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/lib\")
      set(MADYM_EXE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/madym_DCE)
      "
      COMPONENT Tools
      CONFIGURATIONS Release)

    #Now use inbuilt CMake runtime dependency checker, copying
    #the libraries we need to the deploy directory
    #Note we also need to check if any dependencies are symlinks
    #and if so, resolve them and copy the actual lib
    install(CODE [[
        message(STATUS "Getting dependencies for ${MADYM_EXE}")
        file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES ${MADYM_EXE}
        RESOLVED_DEPENDENCIES_VAR _r_deps
        UNRESOLVED_DEPENDENCIES_VAR _u_deps
            PRE_EXCLUDE_REGEXES "(^/System/.*$)" )
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

    if (BUILD_QT_GUI)

        # Copy the GUI dependencies to the commandline tools lib
        install(CODE "
            set(GUI_DEST \"\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/bin/madym_gui.app\")
            "
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
    endif ()

#-------------------------------------------------------------------------
#Linux
elseif( UNIX )

    #Set lib destination evaluated during install
    install(CODE "
        set(LIB_DEST \"\${CMAKE_INSTALL_PREFIX}/${MADYM_DEPLOY_DIR}/lib\")"
        COMPONENT Tools
        CONFIGURATIONS Release)

    if (BUILD_QT_GUI)

        #Copy Qt dependencies to install dploy dir
        install(
            DIRECTORY "${CMAKE_BINARY_DIR}/qt_deploy/" DESTINATION "${MADYM_DEPLOY_DIR}"
            USE_SOURCE_PERMISSIONS
            COMPONENT GUI
            CONFIGURATIONS Release)

        #If building the GUI, set executable to GUI for dependency linking
        install(CODE "
            set(MADYM_EXE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/madym_gui)"
            COMPONENT Tools
            CONFIGURATIONS Release)

    else() 
        #If not building the GUI, use madym_DCE for dependency linking
        install(CODE "
            set(MADYM_EXE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/madym_DCE)
            "
            COMPONENT Tools
            CONFIGURATIONS Release)

    endif()

    #Now use inbuilt CMake runtime dependency checker, copying
    #the libraries we need to the deploy directory
    #Note we also need to check if any dependencies are symlinks
    #and if so, resolve them and copy the actual lib
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

endif() #Platform specific stuff 

#---------------------------------------------------------------------------
# Do Apple code signing
if (APPLE)

    set(APPLE_CODESIGN_ID "" CACHE STRING 
        "Apple developer ID for certificate used to code sign Apple targets")

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

    #Sign all the binaries in the copied libs
    # Transfer the value of ${MY_DEPENDENCY_PATHS} into the install script
    install(CODE "set(_id ${APPLE_CODESIGN_ID})"
      COMPONENT Tools
      CONFIGURATIONS Release)

    install(CODE [[
      file(GLOB_RECURSE lib_files
        LIST_DIRECTORIES false
        "${LIB_DEST}/*")
      foreach(lib_file ${lib_files})
        execute_process(COMMAND codesign --force -s 
          ${_id}
          ${lib_file} )
      endforeach()
      
     ]]
     COMPONENT Tools
     CONFIGURATIONS Release)

     #Sign the GUI bundle
     if (BUILD_QT_GUI)
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
     endif()
endif()

#---------------------------------------------------------------------------
# Now do the packaging with CPack
#---------------------------------------------------------------------------
# Set CPACK generator
if (BUILD_QT_GUI)    
    if (WIN32)
        set(CPACK_GENERATOR "NSIS;ZIP")
    elseif(APPLE)
        set(CPACK_GENERATOR "DragNDrop;TGZ")
    elseif(UNIX)
        set(CPACK_GENERATOR "DEB;TGZ")
    endif ()
else()
    if (WIN32)
        set(CPACK_GENERATOR "ZIP")
    elseif(APPLE)
        set(CPACK_GENERATOR "DragNDrop;TGZ")
    elseif(UNIX)
        set(CPACK_GENERATOR "DEB;TGZ")
    endif ()
endif()
message(STATUS "Set generator to ${CPACK_GENERATOR} for ${CPACK_ARCHIVE_FILE_NAME}")

#---------------------------------------------------------------------------
# Set CPACK OPTIONS
set(CPACK_PACKAGE_FILE_NAME "madym_${MADYM_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}")
set(CPACK_ARCHIVE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})

#Cpack options common to all systems
set(CPACK_PACKAGE_NAME "Madym")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Madym DCE-MRI analysis tools")
set(CPACK_PACKAGE_VENDOR "University of Manchester")
set(CPACK_PACKAGE_CONTACT "michael.berks@manchester.ac.uk")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Madym")
set(CPACK_PACKAGE_EXECUTABLES "madym_gui" "Madym")
set(CPACK_PACKAGE_VERSION_MAJOR "${MADYM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MADYM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MADYM_VERSION_PATCH}")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/madym/qt_gui/images/madym.png")
set(CPACK_COMPONENTS_ALL Tools Libs  Python)

if (APPLE)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/madym/qt_gui/images/madym.icns")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/madym/qt_gui/images/madym.icns")
elseif (UNIX)
    #Debian specific cpack
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Michael Berks")
    set(CPACK_DEBIAN_FILE_NAME "madym_${MADYM_VERSION}.deb")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Madym debian package")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
      "${CMAKE_BINARY_DIR}/generated/postinst" 
      "${CMAKE_BINARY_DIR}/generated/madym_config.sh" 
      "${CMAKE_BINARY_DIR}/generated/prerm")
endif ()

#---------------------------------------------------------------------------
# GUI specific packaging
if (BUILD_QT_GUI)

  #Add GUI to components list
  set(CPACK_COMPONENTS_ALL GUI ${CPACK_COMPONENTS_ALL})

  #Windows
  if ( WIN32 )
    
    #Set Cpack variables to cofigure NSIS installer
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/madym/qt_gui/images\\\\madym.ico")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/madym/qt_gui/images\\\\madym.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${MADYM_DEPLOY_DIR}\\\\bin\\\\madym_gui.exe")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${MADYM_DEPLOY_DIR}\\\\bin")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS " ExecWait 'setx MADYM_ROOT \\\"$INSTDIR\\\\${MADYM_DEPLOY_DIR}\\\\bin\\\"'")
    set(CPACK_NSIS_HELP_LINK "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui")
    set(CPACK_NSIS_CONTACT "michael.berks@manchester.ac.uk")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MENU_LINKS
      "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/madym_gui" "Madym user guide (online)"
      "http://qbi-lab.org" "QBI Lab Website")
    set(CPACK_NSIS_MODIFY_PATH ON)
    
    set(CPACK_CREATE_DESKTOP_LINKS "madym_gui" "Madym")

  endif()

endif( BUILD_QT_GUI )

#Finally call CPACK
include(CPack)