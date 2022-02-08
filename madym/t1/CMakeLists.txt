# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

set(mdm_t1_sources
	mdm_T1Mapper.cxx	        mdm_T1Mapper.h
	mdm_T1MethodGenerator.h
	mdm_T1FitterBase.cxx			mdm_T1FitterBase.h
	mdm_T1FitterVFA.cxx			mdm_T1FitterVFA.h
	mdm_T1FitterIR.cxx			mdm_T1FitterIR.h
)

add_library(mdm_t1 
	${mdm_t1_sources})

target_link_libraries( mdm_t1
  mdm_image
  mdm_opt
)

if (BUILD_INSTALL)
  install(TARGETS mdm_t1
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()

