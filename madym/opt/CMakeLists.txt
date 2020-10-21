# Created: 09-Jan-2018
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 5153 
# Copyright: (C) University of Manchester 

SET( CMAKE_VERBOSE_MAKEFILE ON)

SET(mdm_opt_sources
	#mdm_opt_api.h
	alglibinternal.cpp		alglibinternal.h
	alglibmisc.cpp			alglibmisc.h
	ap.cpp					ap.h
	dataanalysis.cpp		dataanalysis.h
	diffequations.cpp		diffequations.h
	fasttransforms.cpp		fasttransforms.h
	integration.cpp			integration.h
	interpolation.cpp		interpolation.h
	linalg.cpp				linalg.h
	optimization.cpp		optimization.h
	solvers.cpp				solvers.h
	specialfunctions.cpp	specialfunctions.h
	statistics.cpp			statistics.h
	stdafx.h     
)

add_library(mdm_opt ${mdm_opt_sources})
#add_executable(alglib_demo demo.cxx ${mdm_opt_sources})
#target_compile_definitions(alglib_demo PRIVATE /Ox PRIVATE /DAE_CPU=AE_INTEL)

#TARGET_LINK_LIBRARIES( alglib_demo mdm_opt)