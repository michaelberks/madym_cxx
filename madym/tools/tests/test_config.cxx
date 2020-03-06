#include <testlib/testlib_test.h>
#include <iostream>

#include "../../tests/mdm_test_utils.h"

#include <madym/mdm_RunTools.h>

void run_test_config()
{

	std::cout << "======= Testing generation of config files for madym tools =======" << std::endl;

	std::string params_name = mdm_test_utils::temp_dir() + "/params.txt";
	mdm_ToolsOptions options;
	options.aifName = "Aif.txt"; //Str
	options.IAUCTimes = { 20.0, 40.0 }; //vector<double>
	options.dose = 0.25; //double
	options.fixedParams = { 1, 2, 3 }; //vector<int>
	options.T1inputNames = { "fa1", "fa2" }; //vector<str>

	bool wrote = options.to_file(params_name);
	TEST("Writing params file", wrote, true);

	mdm_ToolsOptions options_in;
	bool read = options_in.from_file(params_name);
	TEST("Reading params file", read, true);

	TEST("Reading and writing params file, values match: aifName", options.aifName, options_in.aifName);
	TEST("Reading and writing params file, values match: aifName", options.IAUCTimes, options_in.IAUCTimes);
	TEST("Reading and writing params file, values match: aifName", options.dose, options_in.dose);
	TEST("Reading and writing params file, values match: fixedParams", options.fixedParams, options_in.fixedParams);
	TEST("Reading and writing params file, values match: T1inputNames", options.T1inputNames, options_in.T1inputNames);
}

void test_config()
{
	run_test_config();
}

TESTMAIN(test_config);
