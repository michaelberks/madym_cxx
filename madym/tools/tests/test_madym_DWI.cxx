#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>

#include <madym/dwi/mdm_DWIFitterADC.h>
#include <madym/dwi/mdm_DWIFitterIVIM.h>
#include <madym/image_io/nifti/mdm_NiftiFormat.h>
#include <madym/image_io/meta/mdm_XtrFormat.h>
#include <madym/utils/mdm_Image3D.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_DWI_ADC) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym DWI ADC =======");

	//Generate some signals from sample B, ADC and S0 values
  std::vector<double> Bvals = { 0, 150, 500, 800 };
  int nBvals = (int)Bvals.size();
  double S0 = 100;
  double ADC = 0.8e-3;
  auto signals = mdm_DWIFitterADC::modelToSignals({ S0, ADC }, Bvals);

	std::string test_dir = mdm_test_utils::temp_dir();
	std::string Bval_dir = test_dir + "/Bvals/";
	fs::create_directories(Bval_dir);

	//Compute signal for each Bval and write out image
	std::vector<std::string> Bval_names(nBvals);
  std::string Bvals_str = "";
	for (int i_b = 0; i_b < nBvals; i_b++)
	{
		//Create ADC signal image
		mdm_Image3D Bval_img;
		Bval_img.setDimensions(1, 1, 1);
		Bval_img.setVoxelDims(1, 1, 1);
		Bval_img.info().B.setValue(Bvals[i_b]);
    Bval_img.setVoxel(0, signals[i_b]);

		Bval_names[i_b] = Bval_dir + "Bval_" + std::to_string((int)Bvals[i_b]);

		mdm_NiftiFormat::writeImage3D(Bval_names[i_b], Bval_img,
			mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);

    if (i_b)
      Bvals_str += ",";

    Bvals_str += Bval_names[i_b];
	}

	//Call madym_DWI to fit ADC and S0
	std::string DWI_output_dir = test_dir + "/madym_DWI/";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_DWI"
		<< " --DWI_model ADC "
		<< " --DWI_vols " << Bvals_str
		<< " -o " << DWI_output_dir
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
		BOOST_CHECK_MESSAGE(false, "Running madym_DWI failed");
		return;
	}

	BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DWI tool");

	//Load in the parameter img vols and extract the single voxel from each
	mdm_Image3D ADC_fit = mdm_NiftiFormat::readImage3D(DWI_output_dir + "ADC", false);
	mdm_Image3D S0_fit = mdm_NiftiFormat::readImage3D(DWI_output_dir + "S0", false);

	//Check the model parameters have fitted correctly
	double tol = 0.1;
	BOOST_TEST_MESSAGE("Testing fitted ADC");
	BOOST_CHECK_CLOSE(ADC_fit.voxel(0), ADC, tol);
	BOOST_TEST_MESSAGE("Testing fitted S0");
	BOOST_CHECK_CLOSE(S0_fit.voxel(0), S0, tol);

	//Tidy up
	fs::remove_all(Bval_dir);
	fs::remove_all(DWI_output_dir);
}

BOOST_AUTO_TEST_CASE(test_madym_DWI_IVIM) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym DWI IVIM =======");

	//Generate some signals from sample B, S0, d, f and d* values
	std::vector<double> Bvals = { 0.0, 20.0, 40.0, 60.0, 80.0, 100.0, 300.0, 500.0, 800.0 };
	int nBvals = (int)Bvals.size();
	double S0 = 100;
	double d = 0.8e-3;
	double f = 0.2;
	double dstar = 15e-3;
	auto signals = mdm_DWIFitterIVIM::modelToSignals({ S0, d, f, dstar }, Bvals);

	std::string test_dir = mdm_test_utils::temp_dir();
	std::string Bval_dir = test_dir + "/Bvals/";
	fs::create_directories(Bval_dir);

	//Compute signal for each Bval and write out image
	std::vector<std::string> Bval_names(nBvals);
	std::string Bvals_str = "";
	for (int i_b = 0; i_b < nBvals; i_b++)
	{
		//Create IVIM signal image
		mdm_Image3D Bval_img;
		Bval_img.setDimensions(1, 1, 1);
		Bval_img.setVoxelDims(1, 1, 1);
		Bval_img.info().B.setValue(Bvals[i_b]);
		Bval_img.setVoxel(0, signals[i_b]);

		Bval_names[i_b] = Bval_dir + "Bval_" + std::to_string((int)Bvals[i_b]);

		mdm_NiftiFormat::writeImage3D(Bval_names[i_b], Bval_img,
			mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);

		if (i_b)
			Bvals_str += ",";

		Bvals_str += Bval_names[i_b];
	}

	//Call madym_DWI to fit IVIM and S0
	std::string DWI_output_dir = test_dir + "/madym_DWI/";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_DWI"
		<< " --DWI_model IVIM "
		<< " --DWI_vols " << Bvals_str
		<< " --Bvals_thresh 40.0,60.0,100.0,150.0"
		<< " -o " << DWI_output_dir
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
		BOOST_CHECK_MESSAGE(false, "Running madym_DWI failed");
		return;
	}

	BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DWI tool");

	//Load in the parameter img vols and extract the single voxel from each
	mdm_Image3D S0_fit = mdm_NiftiFormat::readImage3D(DWI_output_dir + "S0", false);
	mdm_Image3D d_fit = mdm_NiftiFormat::readImage3D(DWI_output_dir + "D", false);
	mdm_Image3D f_fit = mdm_NiftiFormat::readImage3D(DWI_output_dir + "f", false);
	mdm_Image3D dstar_fit = mdm_NiftiFormat::readImage3D(DWI_output_dir + "Dstar", false);
	
	//Check the model parameters have fitted correctly
	double tol = 0.5;
	BOOST_TEST_MESSAGE("Testing fitted S0");
	BOOST_CHECK_CLOSE(S0_fit.voxel(0), S0, tol);
	BOOST_TEST_MESSAGE("Testing fitted d");
	BOOST_CHECK_CLOSE(d_fit.voxel(0), d, tol);
	BOOST_TEST_MESSAGE("Testing fitted f");
	BOOST_CHECK_CLOSE(f_fit.voxel(0), f, tol);
	BOOST_TEST_MESSAGE("Testing fitted dstar");
	BOOST_CHECK_CLOSE(dstar_fit.voxel(0), dstar, tol);
	
	//Tidy up
	fs::remove_all(Bval_dir);
	fs::remove_all(DWI_output_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
