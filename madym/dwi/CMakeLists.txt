# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

set(mdm_dwi_sources
	mdm_DWIMapper.cxx	        mdm_DWIMapper.h
	mdm_DWIModelGenerator.h
	mdm_DWIFitterBase.cxx			mdm_DWIFitterBase.h
	mdm_DWIFitterADC.cxx			mdm_DWIFitterADC.h
  mdm_DWIFitterIVIM.cxx			mdm_DWIFitterIVIM.h
)

add_library(mdm_dwi 
	${mdm_dwi_sources})

target_link_libraries( mdm_dwi
  mdm_image
  mdm_opt
)

if (BUILD_INSTALL)
  install(TARGETS mdm_dwi
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()
