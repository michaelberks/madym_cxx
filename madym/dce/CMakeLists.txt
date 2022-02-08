# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

set(mdm_dce_sources
  #Models
	mdm_DCEModelGenerator.h
	mdm_DCEModelBase.cxx		mdm_DCEModelBase.h
	mdm_DCEModelNONE.cxx		mdm_DCEModelNONE.h
	mdm_DCEModelETM.cxx		mdm_DCEModelETM.h
	mdm_DCEModelDIETM.cxx		mdm_DCEModelDIETM.h
	mdm_DCEModelAUEM.cxx		mdm_DCEModelAUEM.h
	mdm_DCEModel2CXM.cxx		mdm_DCEModel2CXM.h
	mdm_DCEModel2CFM.cxx		mdm_DCEModel2CFM.h
	mdm_DCEModelDI2CXM.cxx		mdm_DCEModelDI2CXM.h
	mdm_DCEModelDIBEM.cxx		mdm_DCEModelDIBEM.h
	mdm_DCEModelDIBEM_Fp.cxx	mdm_DCEModelDIBEM_Fp.h
	mdm_DCEModelDISCM.cxx		mdm_DCEModelDISCM.h
	mdm_DCEModelPatlak.cxx		mdm_DCEModelPatlak.h
	mdm_DCEVoxel.cxx			mdm_DCEVoxel.h
	mdm_DCEModelFitter.cxx		mdm_DCEModelFitter.h
	mdm_AIF.cxx					mdm_AIF.h
	mdm_Exponentials.h
)

add_library(mdm_dce 
	${mdm_dce_sources})

target_link_libraries( mdm_dce
  mdm_image
  mdm_opt
)

if (BUILD_INSTALL)
  install(TARGETS mdm_dce
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()

