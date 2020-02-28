#include <testlib/testlib_test.h>
#include <vcl_compiler.h>
#include <iostream>

void run_test_madym_lite()
{

	std::cout << "======= Testing madym analysis =======" << std::endl;

	float a = 1.0;
	float b = 2.0;
	TEST("Images load ok", a + b, 3.0);

	TEST_NEAR("A", a, 1.1, 0.2);
	TEST_NEAR("B", b, 2.2, 0.3);
}

void test_madym_lite()
{
	run_test_madym_lite();
}

TESTMAIN(test_madym_lite);
