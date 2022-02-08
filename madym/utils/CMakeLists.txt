# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

if ( BUILD_QT_GUI )
  #Wrap the program logger header
  QT5_WRAP_CPP( mdm_moc_files 
	  mdm_ProgramLogger.h
  )
else ()
  set(mdm_moc_files "")
endif ()

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

if (BUILD_INSTALL)
  install(TARGETS mdm_utils
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()
