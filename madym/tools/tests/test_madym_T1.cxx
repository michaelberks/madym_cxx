#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>

#include <madym/t1_methods/mdm_T1FitterVFA.h>
#include <madym/image_io/nifti/mdm_NiftiFormat.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include <madym/mdm_Image3D.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_T1) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym T1 =======");

	//Generate some signals from sample FA, TR, T1 and S0 values
	double T1 = 1000;
	double M0 = 2000;
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
		FA_img.setDimensions(1, 1, 1);
		FA_img.setVoxelDims(1, 1, 1);
		FA_img.info().flipAngle.setValue(FAs[i_fa]);
		FA_img.info().TR.setValue(TR);
		FA_img.setVoxel(0, mdm_T1FitterVFA::T1toSignal(T1, M0, PI*FAs[i_fa]/180, TR));

		FA_names[i_fa] = FA_dir + "FA_" + std::to_string((int)FAs[i_fa]);

		mdm_NiftiFormat::writeImage3D(FA_names[i_fa], FA_img,
			mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);
	}

	//Call calculate_T1 to fit T1 and S0
	std::string T1_output_dir = test_dir + "/madym_T1/";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_T1"
		<< " -T VFA "
		<< " --T1_vols " << FA_names[0] << "," << FA_names[1] << "," << FA_names[2]
		<< " -o " << T1_output_dir
		<< " --overwrite"
    << " --no_audit";

	BOOST_TEST_MESSAGE("Command to run: " + cmd.str());

	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		BOOST_CHECK_MESSAGE(false, "Running madym_T1 failed");
		return;
	}

	BOOST_CHECK_MESSAGE(!error, "Error returned from madym_T1 tool");

	//Load in the parameter img vols and extract the single voxel from each
	mdm_Image3D T1_fit = mdm_NiftiFormat::readImage3D(T1_output_dir + "T1", false);
	mdm_Image3D M0_fit = mdm_NiftiFormat::readImage3D(T1_output_dir + "M0", false);

	//Check the model parameters have fitted correctly
	double tol = 0.1;
	BOOST_TEST_MESSAGE("Testing fitted T1");
	BOOST_CHECK_CLOSE(T1_fit.voxel(0), T1, tol);
	BOOST_TEST_MESSAGE("Testing fitted M0");
	BOOST_CHECK_CLOSE(M0_fit.voxel(0), M0, tol);

	//Tidy up
	fs::remove_all(FA_dir);
	fs::remove_all(T1_output_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
