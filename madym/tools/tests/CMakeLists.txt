#
#   Mike Berks
#

project(test_mdm)

# external libraries
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

ADD_EXECUTABLE( test_mdm_tools
  test_mdm_tools.cxx

  test_config.cxx
  test_madym.cxx
  test_madym_lite.cxx
  test_calculate_T1.cxx
  test_calculate_T1_lite.cxx

)

target_link_libraries(test_mdm_tools Boost::unit_test_framework mdm)
target_compile_definitions(test_mdm_tools PUBLIC BOOST_TEST_DYN_LINK)
enable_testing()
add_test(NAME test_mdm_tools COMMAND test_mdm_tools)

