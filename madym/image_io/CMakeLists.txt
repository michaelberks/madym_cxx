# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym)

set(mdm_image_sources
	
  mdm_ImageDatatypes.h
  mdm_ImageIO.cxx mdm_ImageIO.h
  analyze/mdm_AnalyzeFormat.cxx analyze/mdm_AnalyzeFormat.h
  xtr/mdm_XtrFormat.cxx xtr/mdm_XtrFormat.h
  nifti/mdm_NiftiFormat.cxx nifti/mdm_NiftiFormat.h
  nifti/mdm_NiftiFormatAscii.cxx  nifti/mdm_NiftiFormatTransforms.cxx 
  nifti/znzlib.cpp nifti/znzlib.h
  nifti/nifti_swaps.h
	nifti/nifti1.h
  nifti/nifti2.h  
)

if (BUILD_WITH_DCMTK)
  set(mdm_image_sources ${mdm_image_sources} 
    dicom/mdm_DicomFormat.cxx dicom/mdm_DicomFormat.h
  )
endif()

add_library(mdm_image 
	${mdm_image_sources})

target_link_libraries( mdm_image
  mdm_utils
)

if (BUILD_WITH_ZLIB)
  target_link_libraries( mdm_image ZLIB::ZLIB)
endif()

if (BUILD_WITH_DCMTK)
  target_link_libraries( mdm_image ${DCMTK_LIBRARIES})
endif()

if (BUILD_INSTALL)
  install(TARGETS mdm_image
    ARCHIVE DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    LIBRARY DESTINATION "${MADYM_DEPLOY_DIR}/lib" COMPONENT Libs
    RUNTIME DESTINATION "${MADYM_DEPLOY_DIR}/bin" COMPONENT Libs
    CONFIGURATIONS Release) 
endif()

