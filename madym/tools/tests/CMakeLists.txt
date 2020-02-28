#
#   Mike Berks
#

#
#   Mike Berks
#

LINK_LIBRARIES(mdm testlib)

ADD_EXECUTABLE( mdm_tools_test_driver
  test_driver.cxx

  test_madym.cxx
  test_madym_lite.cxx
  test_calculate_T1.cxx
  test_calculate_T1_lite.cxx

)

add_test( NAME mdm_tools_test_madym COMMAND $<TARGET_FILE:mdm_tools_test_driver> test_madym )

