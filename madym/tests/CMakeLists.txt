#
#   Mike Berks
#

LINK_LIBRARIES(mdm testlib)

ADD_EXECUTABLE( make_calibration_files
    make_calibration_files.cxx)

ADD_EXECUTABLE( mdm_test_driver
  test_driver.cxx

  mdm_test_utils.h
  test_mdm.cxx
  test_image3D.cxx
  test_boost.cxx
  test_analyze.cxx
  test_T1.cxx
  test_AIF.cxx
  test_DCE_models.cxx
  test_DCE_fit.cxx
)

add_test( NAME mdm_test_mdm COMMAND $<TARGET_FILE:mdm_test_driver> test_mdm )
add_test( NAME mdm_test_image3D COMMAND $<TARGET_FILE:mdm_test_driver> test_image3D )
add_test( NAME mdm_test_boost COMMAND $<TARGET_FILE:mdm_test_driver> test_boost )
add_test( NAME mdm_test_analyze COMMAND $<TARGET_FILE:mdm_test_driver> test_analyze )
add_test( NAME mdm_test_T1 COMMAND $<TARGET_FILE:mdm_test_driver> test_T1 )
add_test( NAME mdm_test_AIF COMMAND $<TARGET_FILE:mdm_test_driver> test_AIF )
add_test( NAME mdm_test_DCE_models COMMAND $<TARGET_FILE:mdm_test_driver> test_DCE_models )
add_test( NAME mdm_test_DCE_fit COMMAND $<TARGET_FILE:mdm_test_driver> test_DCE_fit )

#See https://cmake.org/cmake/help/v3.8/prop_test/FIXTURES_REQUIRED.html#prop_test:FIXTURES_REQUIRED
#for how to set dependencies between tests

