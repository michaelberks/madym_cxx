#include <testlib/testlib_test.h>
#include <iostream>

#include "../../tests/mdm_test_utils.h"

//#include <madym/mdm_RunTools.h>
#include <madym/mdm_InputOptions.h>

void run_test_config()
{

	std::cout << "======= Testing generation of config files for madym tools =======" << std::endl;

	std::string params_name = mdm_test_utils::temp_dir() + "/params.txt";
	mdm_DefaultValues options;
	mdm_InputOptions options_parser_;

	options.aifName.set("Aif.txt"); //Str
	options.IAUCTimes.set({ 20.0, 40.0 }); //vector<double>
	options.dose.set(0.25); //double
	options.fixedParams.set({ 1, 2, 3 }); //vector<int>
	options.T1inputNames.set({ "fa1", "fa2" }); //vector<str>

	options_parser_.madym_inputs("test_write", options);
	bool wrote = options_parser_.to_file(params_name, options);
	TEST("Writing params file", wrote, true);

	mdm_DefaultValues options_in;
	options_in.configFile.set(params_name);
	int read = options_parser_.madym_inputs("test_read", options_in);
	TEST("Reading params file", read, 0);

	TEST("Reading and writing params file, values match: aifName", options.aifName(), options_in.aifName());
	TEST("Reading and writing params file, values match: aifName", options.IAUCTimes(), options_in.IAUCTimes());
	TEST("Reading and writing params file, values match: aifName", options.dose(), options_in.dose());
	TEST("Reading and writing params file, values match: fixedParams", options.fixedParams(), options_in.fixedParams());
	TEST("Reading and writing params file, values match: T1inputNames", options.T1inputNames(), options_in.T1inputNames());
}

void test_config()
{
	run_test_config();
}

TESTMAIN(test_config);
