#include <testlib/testlib_test.h>
#include <iostream>
#include <sstream>

#include "../../tests/mdm_test_utils.h"

#include <madym/mdm_AIF.h>
#include <madym/mdm_DCEModelGenerator.h>
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
	

void run_test_madym()
{

	std::cout << "======= Testing tool: madym =======" << std::endl;
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
		Ct_img.setMatrixDims(1, 1, 1);
		Ct_img.setVoxelDims(1, 1, 1);
		Ct_img.setTimeStamp(timestamp);
		Ct_img.setVoxel(0, Ct[i_t]);

		mdm_AnalyzeFormat::writeImage3D(Ct_name, Ct_img, 
			mdm_AnalyzeFormat::DT_FLOAT, mdm_AnalyzeFormat::NEW_XTR, false);
	}

	std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct/";

	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym"
		<< " -m ETM"
		<< " -o " << Ct_output_dir
		<< " --dyn " << dyn_dir << "Ct_"
		<< " -n " << nTimes
		<< " -i " << injectionImage
		<< " -D " << dose
		<< " -H " << hct
		<< " -I " << IAUC_str
		<< " --Ct --overwrite "; //<< " -iauc 60"

	std::cout << "Command to run: " << cmd.str() << std::endl;
		
	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		TEST("Running madym failed", 0, 1);
		return;
	}

	TEST("madym tool ran without error", error, 0);

	//Load in the parameter img vols and extract the single voxel from each
	mdm_Image3D ktrans_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "Ktrans.hdr", false);
	mdm_Image3D ve_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "v_e.hdr", false);
	mdm_Image3D vp_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "v_p.hdr", false);
	mdm_Image3D tau_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "tau_a.hdr", false);
	mdm_Image3D model_fit = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "residuals.hdr", false);
	mdm_Image3D error_codes = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "error_codes.hdr", false);
	mdm_Image3D enhancing = mdm_AnalyzeFormat::readImage3D(Ct_output_dir + "enhVox.hdr", false);

	//Check the model parameters have fitted correctly
	double tol = 0.01;
	TEST_NEAR("Fitted ktrans", ktrans_fit.getVoxel(0), trueParams[0], tol);
	TEST_NEAR("Fitted Ve", ve_fit.getVoxel(0), trueParams[1], tol);
	TEST_NEAR("Fitted Vp", vp_fit.getVoxel(0), trueParams[2], tol);
	TEST_NEAR("Fitted tau", tau_fit.getVoxel(0), trueParams[3], tol);

	//Check model fit, error codes and enhancing
	TEST_NEAR("Model residuals", model_fit.getVoxel(0), 0, tol);
	TEST("No error code", error_codes.getVoxel(0), 0);
	TEST("Enhancing", enhancing.getVoxel(0), 1);
	
	//Check IAUC
	for (int i = 0; i < nIAUC; i++)
	{
		std::string iauc_name = Ct_output_dir + "IAUC" + std::to_string((int)IAUCTimes[i]) + ".hdr";
		mdm_Image3D iauc = mdm_AnalyzeFormat::readImage3D(iauc_name, false);
		TEST_NEAR("Fitted IAUC ", iauc.getVoxel(0), IAUCVals[i], tol);
	}
	
	//Tidy up
	fs::remove_all(dyn_dir);
	fs::remove_all(Ct_output_dir);
}

void test_madym()
{
	run_test_madym();
}

TESTMAIN(test_madym);
