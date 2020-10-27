#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <sstream>

#include <madym/tests/mdm_test_utils.h>

#include <madym/mdm_AIF.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>
#include <madym/mdm_AnalyzeFormat.h>
#include <madym/mdm_Image3D.h>
#include <cmath>
#include <mdm_version.h>

namespace fs = boost::filesystem;

double secs_to_timestamp(double t_in_secs)
{
	//Convert time in seconds into the xtr timestamp format
	//hhmmss.msecs represented as a single decimal number
	//
	double hh = std::floor(t_in_secs / (3600));
	double mm = std::floor((t_in_secs - 3600 * hh) / 60);
	double ss = t_in_secs - 3600 * hh - 60 * mm;
	double timestamp = 10000 * hh + 100 * mm + ss;
	return timestamp;
}
	

double mins_to_timestamp(double t_in_mins)
{
	// Convert time in minutes(the form used for dynamic time in madym)
	//into the xtr timestamp format
	//hhmmss.msecs represented as a single decimal number
	return secs_to_timestamp(60 * t_in_mins);
}
	

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym_DCE =======");
	//Need to generate a dataset of analyze images. To do this, load in calibration
	//data

	//Read in dyn times
	int nTimes;
	std::string timesFileName(mdm_test_utils::calibration_dir() + "dyn_times.dat");
	std::ifstream timesFileStream(timesFileName, std::ios::in | std::ios::binary);
	timesFileStream.read(reinterpret_cast<char*>(&nTimes), sizeof(int));
	std::vector<double> dynTimes(nTimes);
	for (double &t : dynTimes)
		timesFileStream.read(reinterpret_cast<char*>(&t), sizeof(double));
	timesFileStream.close();

	//Read in AIF parameters
	int injectionImage;
	double hct;
	double dose;

	std::string aifFileName(mdm_test_utils::calibration_dir() + "aif.dat");
	std::ifstream aifFileStream(aifFileName, std::ios::in | std::ios::binary);
	aifFileStream.read(reinterpret_cast<char*>(&injectionImage), sizeof(int));
	aifFileStream.read(reinterpret_cast<char*>(&hct), sizeof(double));
	aifFileStream.read(reinterpret_cast<char*>(&dose), sizeof(double));
	aifFileStream.close();

	//Read (noisy) ETM times series from calibration data
	int nParams;
	std::vector<double> Ct(nTimes);
	std::string modelFileName = mdm_test_utils::calibration_dir() + "ETM_noise.dat";

	std::ifstream modelFileStream(modelFileName, std::ios::in | std::ios::binary);
	modelFileStream.read(reinterpret_cast<char*>(&nParams), sizeof(int));

	std::vector<double> trueParams(nParams);
	for (double &p : trueParams)
		modelFileStream.read(reinterpret_cast<char*>(&p), sizeof(double));
	for (double &c : Ct)
		modelFileStream.read(reinterpret_cast<char*>(&c), sizeof(double));
	modelFileStream.close();

	//Read IAUC values
	int nIAUC;
	std::string iaucFileName = mdm_test_utils::calibration_dir() + "ETM_IAUC.dat";
	std::ifstream iaucFileStream(iaucFileName, std::ios::in | std::ios::binary);
	iaucFileStream.read(reinterpret_cast<char*>(&nIAUC), sizeof(int));
	std::vector<double> IAUCTimes(nIAUC);
	std::vector<double> IAUCVals(nIAUC);
	for (double &it : IAUCTimes)
		iaucFileStream.read(reinterpret_cast<char*>(&it), sizeof(double));

	for (double &iv : IAUCVals)
		iaucFileStream.read(reinterpret_cast<char*>(&iv), sizeof(double));
	iaucFileStream.close();

	std::string IAUC_str = std::to_string((int)IAUCTimes[0]);
	for (int i = 1; i < nIAUC; i++)
		IAUC_str += ("," + std::to_string((int)IAUCTimes[i]));

	//Create a temporary directory where we'll run these tests, which we can then cleanup
	//easily at the end
	std::string test_dir = mdm_test_utils::temp_dir();
	std::string dyn_dir = test_dir +"/dynamics/";
	fs::create_directories(dyn_dir);

	for (int i_t = 0; i_t < nTimes; i_t++)
	{
		//Write out 1x1 concentration maps and xtr files
		std::string Ct_name = dyn_dir + "Ct_" + std::to_string(i_t + 1);
		double timestamp = mins_to_timestamp(dynTimes[i_t]);

		mdm_Image3D Ct_img;
		Ct_img.setDimensions(1, 1, 1);
		Ct_img.setVoxelDims(1, 1, 1);
		Ct_img.setTimeStamp(timestamp);
		Ct_img.setVoxel(0, Ct[i_t]);

		mdm_AnalyzeFormat::writeImage3D(Ct_name, Ct_img, 
			mdm_AnalyzeFormat::DT_FLOAT, mdm_AnalyzeFormat::NEW_XTR, false);
	}

	std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct/";

	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_DCE"
		<< " -m ETM"
		<< " -o " << Ct_output_dir
		<< " --dyn " << dyn_dir << "Ct_"
		<< " -n " << nTimes
		<< " -i " << injectionImage
		<< " -D " << dose
		<< " -H " << hct
		<< " -I " << IAUC_str
		<< " --Ct --overwrite "; //<< " -iauc 60"

	BOOST_TEST_MESSAGE("Command to run: " + cmd.str());
		
	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		BOOST_CHECK_MESSAGE(false, "Running madym_DCE failed");
		return;
	}

	BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DCE tool");

	//Load in the parameter img vols and extract the single voxel from each
	mdm_Image3D ktrans_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "Ktrans.hdr", false);
	mdm_Image3D ve_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "v_e.hdr", false);
	mdm_Image3D vp_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "v_p.hdr", false);
	mdm_Image3D tau_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "tau_a.hdr", false);
	mdm_Image3D model_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "residuals.hdr", false);
	mdm_Image3D error_codes = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "error_codes.hdr", false);
	mdm_Image3D enhancing = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "enhVox.hdr", false);

	//Check the model parameters have fitted correctly
	double tol = 0.1;
	BOOST_TEST_MESSAGE(boost::format("Fitted ktrans (%1.2f, %2.2f)")
		% ktrans_fit.voxel(0) % trueParams[0]);
	BOOST_CHECK_CLOSE(ktrans_fit.voxel(0), trueParams[0], tol);
	BOOST_TEST_MESSAGE(boost::format("Fitted Ve (%1.2f, %2.2f)")
		% ve_fit.voxel(0) % trueParams[1]);
	BOOST_CHECK_CLOSE(ve_fit.voxel(0), trueParams[1], tol);
	BOOST_TEST_MESSAGE(boost::format("Fitted Vp (%1.2f, %2.2f)")
		% vp_fit.voxel(0) % trueParams[2]);
	BOOST_CHECK_CLOSE(vp_fit.voxel(0), trueParams[2], tol);
	BOOST_TEST_MESSAGE(boost::format("Fitted tau (%1.2f, %2.2f)")
		% tau_fit.voxel(0) % trueParams[3]);
	BOOST_CHECK_CLOSE(tau_fit.voxel(0), trueParams[3], tol);

	//Check model fit, error codes and enhancing
	BOOST_TEST_MESSAGE(boost::format("Model residuals = %1%")
		% model_fit.voxel(0));
	BOOST_TEST_MESSAGE("No error code");
	BOOST_CHECK(!error_codes.voxel(0));
	BOOST_TEST_MESSAGE("Enhancing");
	BOOST_CHECK(enhancing.voxel(0));
	
	//Check IAUC
	for (int i = 0; i < nIAUC; i++)
	{
		std::string iauc_name = Ct_output_dir + "IAUC" + std::to_string((int)IAUCTimes[i]) + ".hdr";
		mdm_Image3D iauc = mdm_AnalyzeFormat::readImage3D(iauc_name, false);
		BOOST_TEST_MESSAGE("Fitted IAUC" + std::to_string((int)IAUCTimes[i]));
		BOOST_CHECK_CLOSE(iauc.voxel(0), IAUCVals[i], tol);
	}
	
	//Tidy up
	fs::remove_all(dyn_dir);
	fs::remove_all(Ct_output_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
