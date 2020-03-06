#
#   Mike Berks
#

LINK_LIBRARIES(mdm testlib)

ADD_EXECUTABLE( mdm_tools_test_driver
  test_driver.cxx

  test_config.cxx
  test_madym.cxx
  test_madym_lite.cxx
  test_calculate_T1.cxx
  test_calculate_T1_lite.cxx

)

add_test( NAME mdm_tools_test_config COMMAND $<TARGET_FILE:mdm_tools_test_driver> test_config )
add_test( NAME mdm_tools_test_madym COMMAND $<TARGET_FILE:mdm_tools_test_driver> test_madym )
add_test( NAME mdm_tools_test_madym_lite COMMAND $<TARGET_FILE:mdm_tools_test_driver> test_madym_lite )
add_test( NAME mdm_tools_test_calculate_T1 COMMAND $<TARGET_FILE:mdm_tools_test_driver> test_calculate_T1 )
add_test( NAME mdm_tools_test_calculate_T1_lite COMMAND $<TARGET_FILE:mdm_tools_test_driver> test_calculate_T1_lite )

