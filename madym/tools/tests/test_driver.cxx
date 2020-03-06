#include <testlib/testlib_register.h>

DECLARE(test_config);
DECLARE(test_madym);
DECLARE(test_madym_lite);
DECLARE(test_calculate_T1);
DECLARE(test_calculate_T1_lite);

void
register_tests()
{
	REGISTER(test_config);
	REGISTER(test_madym);
	REGISTER(test_madym_lite);
	REGISTER(test_calculate_T1);
	REGISTER(test_calculate_T1_lite);
}

DEFINE_MAIN;
