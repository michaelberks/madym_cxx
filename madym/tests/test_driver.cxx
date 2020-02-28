#include <testlib/testlib_register.h>

DECLARE(  test_mdm );
DECLARE(  test_image3D);
DECLARE(  test_boost);
DECLARE(  test_analyze );
DECLARE(  test_T1  );
DECLARE(  test_AIF);
DECLARE(  test_DCE_models);
DECLARE(  test_DCE_fit);

void
register_tests()
{
  REGISTER(  test_mdm );
	REGISTER(  test_image3D);
	REGISTER(  test_boost);
	REGISTER(  test_analyze );
	REGISTER(  test_T1);
	REGISTER(  test_AIF);
	REGISTER(  test_DCE_models);
	REGISTER(  test_DCE_fit);
}

DEFINE_MAIN;
