#include <testlib/testlib_test.h>
#include <iostream>

#include "../../tests/mdm_test_utils.h"

#include <madym/mdm_T1Voxel.h>
#include <madym/mdm_AnalyzeFormat.h>
#include <madym/mdm_Image3D.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

void run_test_calculate_T1()
{

	std::cout << "======= Testing tool: calculate T1 =======" << std::endl;

	//Generate some signals from sample FA, TR, T1 and S0 values
	double T1 = 1000;
	double S0 = 2000;
	double TR = 3.5;
	std::vector<double>	FAs = { 2, 10, 18 };
	const auto PI = acos(-1.0);

	std::string test_dir = mdm_test_utils::temp_dir();
	std::string FA_dir = test_dir + "/FAs/";
	fs::create_directories(FA_dir);

	//Compute signal for each FA and write out image
	std::vector<std::string> FA_names(3);
	for (int i_fa = 0; i_fa < 3; i_fa++)
	{
		//Create VFA signal image
		mdm_Image3D FA_img;
		FA_img.setMatrixDims(1, 1, 1);
		FA_img.setVoxelDims(1, 1, 1);
		FA_img.info_.flipAngle.setValue(FAs[i_fa]);
		FA_img.info_.TR.setValue(TR);
		FA_img.setVoxel(0, mdm_T1Voxel::T1toSignal(T1, S0, PI*FAs[i_fa]/180, TR));

		FA_names[i_fa] = FA_dir + "FA_" + std::to_string((int)FAs[i_fa]);

		mdm_AnalyzeFormat::writeImage3D(FA_names[i_fa], FA_img,
			mdm_AnalyzeFormat::DT_FLOAT, mdm_AnalyzeFormat::NEW_XTR, false);
	}

	//Call calculate_T1 to fit T1 and S0
	std::string T1_output_dir = test_dir + "/calculate_T1/";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "calculate_T1"
		<< " -m VFA "
		<< " -maps " << FA_names[0] << "," << FA_names[1] << "," << FA_names[2]
		<< " -o " << T1_output_dir;

	std::cout << "Command to run: " << cmd.str() << std::endl;

	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		TEST("Running calculate_T1 failed", 0, 1);
		return;
	}

	TEST("calculate_T1 tool ran without error", error, 0);

	//Load in the parameter img vols and extract the single voxel from each
	mdm_Image3D T1_fit = mdm_AnalyzeFormat::readImage3D(T1_output_dir + "T1.hdr", false);
	mdm_Image3D S0_fit = mdm_AnalyzeFormat::readImage3D(T1_output_dir + "S0.hdr", false);

	//Check the model parameters have fitted correctly
	double tol = 0.1;
	TEST_NEAR("Fitted T1", T1_fit.getVoxel(0), T1, tol);
	TEST_NEAR("Fitted S0", S0_fit.getVoxel(0), S0, tol);

	//Tidy up
	fs::remove_all(FA_dir);
	fs::remove_all(T1_output_dir);
}

void test_calculate_T1()
{
	run_test_calculate_T1();
}

TESTMAIN(test_calculate_T1);
