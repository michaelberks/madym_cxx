#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>

#include <madym/t1_methods/mdm_T1FitterVFA.h>
#include <madym/t1_methods/mdm_T1FitterIR.h>
#include <madym/image_io/nifti/mdm_NiftiFormat.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include <madym/mdm_Image3D.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_T1_VFA) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym T1 VFA =======");

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

BOOST_AUTO_TEST_CASE(test_madym_T1_VFA_B1) {
  BOOST_TEST_MESSAGE("======= Testing tool: madym T1 VFA with B1 correction =======");

  //Generate some signals from sample FA, TR, T1 and S0 values
  double T1 = 1000;
  double M0 = 2000;
  double B1 = 120;
  double B1_scaling = 100;
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
    auto effective_FA = FAs[i_fa] * B1 / B1_scaling;
    FA_img.setVoxel(0, mdm_T1FitterVFA::T1toSignal(T1, M0, PI*effective_FA / 180, TR));

    FA_names[i_fa] = FA_dir + "FA_" + std::to_string((int)FAs[i_fa]);

    mdm_NiftiFormat::writeImage3D(FA_names[i_fa], FA_img,
      mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);
  }

  std::string B1_name = FA_dir + "B1";
  mdm_Image3D B1_img;
  B1_img.setDimensions(1, 1, 1);
  B1_img.setVoxelDims(1, 1, 1);
  B1_img.setVoxel(0, B1);
  mdm_NiftiFormat::writeImage3D(B1_name, B1_img,
    mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);

  //Call calculate_T1 to fit T1 and S0
  std::string T1_output_dir = test_dir + "/madym_T1/";
  std::stringstream cmd;
  cmd << mdm_test_utils::tools_exe_dir() << "madym_T1"
    << " -T VFA_B1 "
    << " --T1_vols " << FA_names[0] << "," << FA_names[1] << "," << FA_names[2]
    << " --B1 " << B1_name
    << " --B1_scaling " << B1_scaling
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

BOOST_AUTO_TEST_CASE(test_madym_T1_IR) {
  BOOST_TEST_MESSAGE("======= Testing tool: madym T1 IR =======");

  //Generate some signals from sample FA, TR, T1 and S0 values
  std::vector<double> TIs = { 50, 300, 800, 1000, 2000, 4000 };
  int nTIs = (int)TIs.size();
  double T1 = 1500;
  double M0 = 1000;
  double TR = 1e5;

  std::string test_dir = mdm_test_utils::temp_dir();
  std::string IR_dir = test_dir + "/IRs/";
  fs::create_directories(IR_dir);

  //Compute signal for each IR and write out image
  std::vector<std::string> IR_names(nTIs);
  for (int i_ti = 0; i_ti < nTIs; i_ti++)
  {
    //Create inversion recovery signal image
    mdm_Image3D IR_img;
    IR_img.setDimensions(1, 1, 1);
    IR_img.setVoxelDims(1, 1, 1);
    IR_img.info().TI.setValue(TIs[i_ti]);
    IR_img.info().TR.setValue(TR);
    IR_img.setVoxel(0, mdm_T1FitterIR::T1toSignal(T1, M0, TIs[i_ti], TR));

    IR_names[i_ti] = IR_dir + "IR_" + std::to_string((int)TIs[i_ti]);

    mdm_NiftiFormat::writeImage3D(IR_names[i_ti], IR_img,
      mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);
  }

  //Call calculate_T1 to fit T1 and S0
  std::string T1_output_dir = test_dir + "/madym_T1/";
  std::stringstream cmd;
  cmd << mdm_test_utils::tools_exe_dir() << "madym_T1"
    << " -T IR "
    << " --T1_vols " << IR_names[0];
  for (int i_ti = 1; i_ti < nTIs; i_ti++)
    cmd << "," << IR_names[i_ti];

  cmd << " --TR " << TR
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

  // Repeat the test with inversion recovery

  //Tidy up
  fs::remove_all(IR_dir);
  fs::remove_all(T1_output_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
