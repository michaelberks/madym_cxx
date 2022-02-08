#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>
#include <madym/run/mdm_OptionsParser.h>
#include <madym/run/mdm_RunTools_madym_DCE.h>

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_config) {
	BOOST_TEST_MESSAGE("======= Testing generation of config files for madym tools =======");

	std::string params_name = mdm_test_utils::temp_dir() + "/params.txt";
	mdm_RunTools_madym_DCE madym_write;
  auto &options = madym_write.options();

	options.outputRoot.set("root/"); //str
	options.outputCt_sig.set(true); //bool
	options.aifName.set("Aif.txt"); //Str
	options.IAUCTimes.set({ 20.0, 40.0 }); //vector<double>
	options.dose.set(0.25); //double
	options.fixedParams.set({ 1, 2, 3 }); //vector<int>
	options.T1inputNames.set({ "fa1", "fa2" }); //vector<str>
	options.maxIterations.set(100); //Int
	options.testEnhancement.set(false);

	BOOST_TEST_MESSAGE("Writing params file");
	BOOST_CHECK(!madym_write.parseInputs("test_write"));
	BOOST_CHECK_NO_THROW(madym_write.saveConfigFile(params_name));

  mdm_RunTools_madym_DCE madym_read;
	auto &options_in = madym_read.options();
	options_in.configFile.set(params_name);
	

	BOOST_TEST_MESSAGE("Reading params file");
	BOOST_CHECK(!madym_read.parseInputs("test_read"));

	BOOST_TEST_MESSAGE("Reading and writing params file, values match: outputRoot");
	BOOST_CHECK_EQUAL(options.outputRoot(), options_in.outputRoot());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: outputCt_sig");
	BOOST_CHECK_EQUAL(options.outputCt_sig(), options_in.outputCt_sig());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: aifName");
	BOOST_CHECK_EQUAL(options.aifName(), options_in.aifName());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: IAUCTimes");
	BOOST_CHECK_VECTORS(options.IAUCTimes(), options_in.IAUCTimes());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: dose");
	BOOST_CHECK_EQUAL(options.dose(), options_in.dose());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: fixedParams");
	BOOST_CHECK_VECTORS(options.fixedParams(), options_in.fixedParams());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: T1inputNames");
	BOOST_CHECK_VECTORS(options.T1inputNames(), options_in.T1inputNames());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: maxIterations");
	BOOST_CHECK_EQUAL(options.maxIterations(), options_in.maxIterations());
	BOOST_TEST_MESSAGE("Reading and writing params file, values match: testEnhancemnet");
	BOOST_CHECK_EQUAL(options.testEnhancement(), options_in.testEnhancement());
}

BOOST_AUTO_TEST_SUITE_END() //
