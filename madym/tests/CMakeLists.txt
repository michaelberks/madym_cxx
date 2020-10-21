#
#   Mike Berks
#

project(test_mdm)

# external libraries
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

ADD_EXECUTABLE( make_calibration_files
    make_calibration_files.cxx)
target_link_libraries(make_calibration_files mdm)

ADD_EXECUTABLE( test_mdm

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

target_link_libraries(test_mdm Boost::unit_test_framework mdm)
target_compile_definitions(test_mdm PUBLIC BOOST_TEST_DYN_LINK)
enable_testing()
add_test(NAME test_mdm COMMAND test_mdm)

#See https://cmake.org/cmake/help/v3.8/prop_test/FIXTURES_REQUIRED.html#prop_test:FIXTURES_REQUIRED
#for how to set dependencies between tests

