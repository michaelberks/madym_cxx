# Created: 05-Apr-2017
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester 

project(madym_tools)

#-------------------------------------------------------------------
# Main madym analysis tool applied to whole image volumes
ADD_EXECUTABLE(madym 
	madym.cxx)

TARGET_LINK_LIBRARIES( madym mdm
  )

#-------------------------------------------------------------------
# Lightweight tool for performing analysis on individual voxels
ADD_EXECUTABLE(madym_lite 
	madym_lite.cxx)

TARGET_LINK_LIBRARIES( madym_lite mdm)

#-------------------------------------------------------------------
# Standalone tool for computing T1 maps
ADD_EXECUTABLE(calculate_T1 
	calculate_T1.cxx)

TARGET_LINK_LIBRARIES( calculate_T1 mdm)

#-------------------------------------------------------------------
# Standalone tool for computing T1 values
ADD_EXECUTABLE(calculate_T1_lite 
	calculate_T1_lite.cxx)

TARGET_LINK_LIBRARIES( calculate_T1_lite mdm)

#-------------------------------------------------------------------
# Standalone tool for computing an auto AIF
#ADD_EXECUTABLE(fit_auto_AIF 
#	fit_auto_AIF.cxx)
#
#TARGET_LINK_LIBRARIES( fit_auto_AIF mdm)

#-------------------------------------------------------------------
# Standalone tool for converting raw dicom images to analyze format volumes
#ADD_EXECUTABLE(dicom2analyze 
#	dicom2analyze.cxx)
#
#TARGET_LINK_LIBRARIES( dicom2analyze mdm)

#-------------------------------------------------------------------
# Standalone tool for computing summary statistics from Madym output
#ADD_EXECUTABLE(trials_datastreams_calc_stats 
#	trials_datastreams_calc_stats.cxx)
#
#TARGET_LINK_LIBRARIES( trials_datastreams_calc_stats mdm)

#-------------------------------------------------------------------
if ( BUILD_TESTING )
	SUBDIRS(tests)
endif ( BUILD_TESTING )
