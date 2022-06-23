#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <sstream>

#include <madym/tests/mdm_test_utils.h>

#include <madym/dce/mdm_AIF.h>
#include <madym/dce/mdm_DCEModelGenerator.h>
#include <madym/run/mdm_RunTools_madym_DCE.h>
#include <madym/image_io/mdm_ImageIO.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/run/mdm_ParamSummaryStats.h>
#include <cmath>
#include <mdm_version.h>

namespace fs = boost::filesystem;

void check_output(
	const std::string& Ct_output_dir,
	const std::vector<double> &trueParams,
	const std::vector<double> IAUCTimes,
	const std::vector<double> IAUCVals,
  mdm_ImageIO::ImageFormat imageFormat)
{
  double tol = 1.0;

  //Even with empty model, these should be created
  mdm_Image3D error_codes = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "error_tracker", false, false);
  mdm_Image3D enhancing = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "enhVox", false, false);

  //Check error codes and enhancing
  BOOST_TEST_MESSAGE("No error code");
  BOOST_CHECK(!error_codes.voxel(0));
  BOOST_TEST_MESSAGE("Enhancing");
  BOOST_CHECK(enhancing.voxel(0));

  if (!trueParams.empty())
  {
    //Load in the parameter img vols and extract the single voxel from each
    mdm_Image3D ktrans_fit = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "Ktrans", false, false);
    mdm_Image3D ve_fit = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "v_e", false, false);
    mdm_Image3D vp_fit = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "v_p", false, false);
    mdm_Image3D tau_fit = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "tau_a", false, false);
    mdm_Image3D model_fit = mdm_ImageIO::readImage3D(imageFormat, Ct_output_dir + "residuals", false, false);

    //Check the model parameters have fitted correctly
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

    //Read stats file
    mdm_ParamSummaryStats stats;
    stats.openStatsFile(Ct_output_dir + "ROI_summary_stats.csv");
    for (int param = 0; param < 4; param++)
    {
      stats.readStats();
      BOOST_CHECK_CLOSE(stats.stats().mean_, trueParams[param], tol);
      BOOST_CHECK_EQUAL(stats.stats().stddev_, 0);
      BOOST_CHECK_EQUAL(stats.stats().validVoxels_, 1);
    }

    stats.closeStatsFile();
  }

	//Check IAUC
	for (auto i = 0; i < IAUCTimes.size(); i++)
	{
		std::string iauc_name = Ct_output_dir + "IAUC" + std::to_string((int)IAUCTimes[i]);
		mdm_Image3D iauc = mdm_ImageIO::readImage3D(imageFormat, iauc_name, false, false);
		BOOST_TEST_MESSAGE("Fitted IAUC" + std::to_string((int)IAUCTimes[i]));
		BOOST_CHECK_CLOSE(iauc.voxel(0), IAUCVals[i], tol);
	}

	//Tidy up
	fs::remove_all(Ct_output_dir);
}
	

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_DCE) {
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
		std::string Ct_name = dyn_dir + "Ct_" + (boost::format("%02u") % (i_t + 1)).str();

		mdm_Image3D Ct_img;
		Ct_img.setDimensions(1, 1, 1);
		Ct_img.setVoxelDims(1, 1, 1);
		Ct_img.setTimeStampFromMins(dynTimes[i_t]);
		Ct_img.setVoxel(0, Ct[i_t]);

		mdm_AnalyzeFormat::writeImage3D(Ct_name, Ct_img, 
			mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NEW_XTR, false);

		if (!i_t)
			BOOST_TEST_MESSAGE("Saved 1st dynamic image " << Ct_name);
	}

	//Run 2 types of tests:
	// 1) Using a run tools object, this runs the complete pipeline but doesn't involve a system call
	// 2) Calling system to run a command line call

	//-------------------------------------------------------------------------------
	// 1) Using a run tools object
	//-------------------------------------------------------------------------------
	{
		std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct1/";
    mdm_RunTools_madym_DCE madym_exe;
		auto &madym_options = madym_exe.options();

		madym_options.model.set("ETM");
		madym_options.outputDir.set(Ct_output_dir);
		madym_options.dynDir.set(dyn_dir);
		madym_options.dynName.set("Ct_");
		madym_options.sequenceFormat.set("%02u");
		madym_options.nDyns.set(nTimes);
		madym_options.injectionImage.set(injectionImage);
		madym_options.dose.set(dose);
		madym_options.hct.set(hct);
		madym_options.IAUCTimes.set(IAUCTimes);
		madym_options.inputCt.set(true);
    madym_options.imageWriteFormat.set("ANALYZE");
		madym_options.overwrite.set(true);
    madym_options.noAudit.set(true);
		
		madym_exe.parseInputs("test_madym_DCE");
		int result = madym_exe.run_catch();

		BOOST_CHECK_MESSAGE(!result, "Running madym_DCE failed");
		check_output(Ct_output_dir,
			trueParams,
			IAUCTimes,
			IAUCVals,
      mdm_ImageIO::ImageFormat::ANALYZE
    );
	}

	//-------------------------------------------------------------------------------
	// 2) From the command line
	//-------------------------------------------------------------------------------
	{
		std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct2/";
		std::stringstream cmd;
		cmd << mdm_test_utils::tools_exe_dir() << "madym_DCE"
			<< " -m ETM"
			<< " -o " << Ct_output_dir
			<< " --dyn " << dyn_dir << "Ct_"
			<< " --sequence_format " << "%02u"
			<< " -n " << nTimes
			<< " -i " << injectionImage
			<< " -D " << dose
			<< " -H " << hct
			<< " -I " << IAUC_str
			<< " --Ct"
      << " --img_fmt_w " << "ANALYZE"
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
			BOOST_CHECK_MESSAGE(false, "Running madym_DCE failed");
			return;
		}

		BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DCE tool");
		check_output(Ct_output_dir,
			trueParams,
			IAUCTimes,
			IAUCVals,
      mdm_ImageIO::ImageFormat::ANALYZE);
	}

	//-------------------------------------------------------------------------------
	// 3) Using a run tools object with empty IAUC values
	//-------------------------------------------------------------------------------
	{
		std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct3/";
    mdm_RunTools_madym_DCE madym_exe;
    auto &madym_options = madym_exe.options();
		madym_options.model.set("ETM");
		madym_options.outputDir.set(Ct_output_dir);
		madym_options.dynDir.set(dyn_dir);
		madym_options.sequenceFormat.set("%02u");
		madym_options.dynName.set("Ct_");
		madym_options.nDyns.set(nTimes);
		madym_options.injectionImage.set(injectionImage);
		madym_options.dose.set(dose);
		madym_options.hct.set(hct);
		madym_options.IAUCTimes.set({});
		madym_options.inputCt.set(true);
    madym_options.imageWriteFormat.set("ANALYZE");
    madym_options.overwrite.set(true);
    madym_options.noAudit.set(true);

		madym_exe.parseInputs("test_madym_DCE_noI");
		int result = madym_exe.run_catch();

		BOOST_CHECK_MESSAGE(!result, "Running madym_DCE failed");
		check_output(Ct_output_dir,
			trueParams,
			{},
			{},
      mdm_ImageIO::ImageFormat::ANALYZE);
	}

  //-------------------------------------------------------------------------------
  // 4) Using a run tools object with empty model
  //-------------------------------------------------------------------------------
  {
    std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct4/";
    mdm_RunTools_madym_DCE madym_exe;
    auto &madym_options = madym_exe.options();
    madym_options.model.set("NONE");
    madym_options.outputDir.set(Ct_output_dir);
    madym_options.dynDir.set(dyn_dir);
    madym_options.sequenceFormat.set("%02u");
    madym_options.dynName.set("Ct_");
    madym_options.nDyns.set(nTimes);
    madym_options.injectionImage.set(injectionImage);
    madym_options.dose.set(dose);
    madym_options.hct.set(hct);
    madym_options.IAUCTimes.set(IAUCTimes);
    madym_options.inputCt.set(true);
    madym_options.imageWriteFormat.set("ANALYZE");
    madym_options.overwrite.set(true);
    madym_options.noAudit.set(true);

    madym_exe.parseInputs("test_madym_DCE_noM");
    int result = madym_exe.run_catch();

    BOOST_CHECK_MESSAGE(!result, "Running madym_DCE failed");
    check_output(Ct_output_dir,
      {},
      IAUCTimes,
      IAUCVals,
      mdm_ImageIO::ImageFormat::ANALYZE);
  }

  //-------------------------------------------------------------------------------
  // 5) Using NIFTI as image format
  //-------------------------------------------------------------------------------
  {
    std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct1/";
    mdm_RunTools_madym_DCE madym_exe;
    auto &madym_options = madym_exe.options();

    madym_options.model.set("ETM");
    madym_options.outputDir.set(Ct_output_dir);
    madym_options.dynDir.set(dyn_dir);
    madym_options.dynName.set("Ct_");
    madym_options.sequenceFormat.set("%02u");
    madym_options.nDyns.set(nTimes);
    madym_options.injectionImage.set(injectionImage);
    madym_options.dose.set(dose);
    madym_options.hct.set(hct);
    madym_options.IAUCTimes.set(IAUCTimes);
    madym_options.inputCt.set(true);
    madym_options.imageWriteFormat.set("NIFTI");
    madym_options.overwrite.set(true);
    madym_options.noAudit.set(true);

    madym_exe.parseInputs("test_madym_DCE");
    int result = madym_exe.run_catch();

    BOOST_CHECK_MESSAGE(!result, "Running madym_DCE failed");
    check_output(Ct_output_dir,
      trueParams,
      IAUCTimes,
      IAUCVals,
      mdm_ImageIO::ImageFormat::NIFTI
    );
  }

  //-------------------------------------------------------------------------------
  // 6) Fit once, then reload params and call no opt
  //-------------------------------------------------------------------------------
  {
    std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct1/";
    {
      mdm_RunTools_madym_DCE madym_exe;
      auto &madym_options = madym_exe.options();

      madym_options.model.set("ETM");
      madym_options.outputDir.set(Ct_output_dir);
      madym_options.dynDir.set(dyn_dir);
      madym_options.dynName.set("Ct_");
      madym_options.sequenceFormat.set("%02u");
      madym_options.nDyns.set(nTimes);
      madym_options.injectionImage.set(injectionImage);
      madym_options.dose.set(dose);
      madym_options.hct.set(hct);
      madym_options.IAUCTimes.set(IAUCTimes);
      madym_options.inputCt.set(true);
      madym_options.imageWriteFormat.set("ANALYZE");
      madym_options.overwrite.set(true);
      madym_options.noAudit.set(true);

      madym_exe.parseInputs("test_madym_DCE");
      int result = madym_exe.run_catch();

      BOOST_CHECK_MESSAGE(!result, "Running madym_DCE failed");
    }
    {
      mdm_RunTools_madym_DCE madym_exe;
      auto &madym_options = madym_exe.options();

      madym_options.model.set("ETM");
      madym_options.outputDir.set(Ct_output_dir);
      madym_options.dynDir.set(dyn_dir);
      madym_options.dynName.set("Ct_");
      madym_options.sequenceFormat.set("%02u");
      madym_options.nDyns.set(nTimes);
      madym_options.injectionImage.set(injectionImage);
      madym_options.dose.set(dose);
      madym_options.hct.set(hct);
      madym_options.IAUCTimes.set(IAUCTimes);
      madym_options.inputCt.set(true);
      madym_options.imageWriteFormat.set("ANALYZE");
      madym_options.initMapsDir.set(Ct_output_dir);
      madym_options.noOptimise.set(true);
      madym_options.overwrite.set(true);
      madym_options.noAudit.set(true);

      madym_exe.parseInputs("test_madym_DCE");
      int result = madym_exe.run_catch();

      BOOST_CHECK_MESSAGE(!result, "Running madym_DCE from init params map failed");
    }


    check_output(Ct_output_dir,
      trueParams,
      IAUCTimes,
      IAUCVals,
      mdm_ImageIO::ImageFormat::ANALYZE
    );
  }
  //-------------------------------------------------------------------------------
  // 6) Check voxel size checks works as expected
  //-------------------------------------------------------------------------------
  {
    //
    mdm_Image3D ROI;
    ROI.setDimensions(1, 1, 1);
    ROI.setVoxelDims(1, 1, 2);
    ROI.setVoxel(0, 1);

    std::string ROI_name = dyn_dir + "ROI";
    mdm_AnalyzeFormat::writeImage3D(ROI_name, ROI,
      mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::NO_XTR, false);

    std::string Ct_output_dir = test_dir + "/mdm_analysis_Ct1/";

    {
      mdm_RunTools_madym_DCE madym_exe;
      auto &madym_options = madym_exe.options();

      madym_options.model.set("ETM");
      madym_options.outputDir.set(Ct_output_dir);
      madym_options.dynDir.set(dyn_dir);
      madym_options.dynName.set("Ct_");
      madym_options.sequenceFormat.set("%02u");
      madym_options.nDyns.set(nTimes);
      madym_options.roiName.set(ROI_name);
      madym_options.injectionImage.set(injectionImage);
      madym_options.dose.set(dose);
      madym_options.hct.set(hct);
      madym_options.IAUCTimes.set(IAUCTimes);
      madym_options.inputCt.set(true);
      madym_options.imageWriteFormat.set("ANALYZE");
      madym_options.overwrite.set(true);
      madym_options.noAudit.set(true);

      madym_exe.parseInputs("test_madym_DCE");
      int result = madym_exe.run_catch();
      BOOST_CHECK_EQUAL(result, 1.0);

    }
    {
      mdm_RunTools_madym_DCE madym_exe;
      auto &madym_options = madym_exe.options();

      madym_options.model.set("ETM");
      madym_options.outputDir.set(Ct_output_dir);
      madym_options.dynDir.set(dyn_dir);
      madym_options.dynName.set("Ct_");
      madym_options.sequenceFormat.set("%02u");
      madym_options.nDyns.set(nTimes);
      madym_options.roiName.set(ROI_name);
      madym_options.injectionImage.set(injectionImage);
      madym_options.dose.set(dose);
      madym_options.hct.set(hct);
      madym_options.IAUCTimes.set(IAUCTimes);
      madym_options.inputCt.set(true);
      madym_options.imageWriteFormat.set("ANALYZE");
      madym_options.voxelSizeWarnOnly.set(true);
      madym_options.overwrite.set(true);
      madym_options.noAudit.set(true);

      madym_exe.parseInputs("test_madym_DCE");
      int result = madym_exe.run_catch();

      BOOST_CHECK_MESSAGE(!result, "Running madym_DCE failed");
    }


    check_output(Ct_output_dir,
      trueParams,
      IAUCTimes,
      IAUCVals,
      mdm_ImageIO::ImageFormat::ANALYZE
    );
  }

	//---------------------------------------------------------------------------
	//Tidy up
	fs::remove_all(dyn_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
