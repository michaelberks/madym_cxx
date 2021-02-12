# Created: 09-Jan-2018
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 5153 
# Copyright: (C) University of Manchester 


SET(mdm_dicom_sources
	mdm_dicom_api.h
	mdm_analyzeUtils.cxx		mdm_analyzeUtils.h     
    mdm_dicomIO.cxx mdm_dicomIO.h
    mdm_flip.cxx mdm_flip.h
    mdm_nemaIO.cxx mdm_nemaIO.h
    mdm_utils.cxx		mdm_utils.h
    tina_swap.cxx tina_swap.h
    
)
add_library(mdm_dicom ${mdm_dicom_sources})
target_link_libraries(mdm_dicom mdm_ana)