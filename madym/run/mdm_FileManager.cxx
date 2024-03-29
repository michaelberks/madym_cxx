/**
*  @file    mdm_FileManager.cxx
*  @brief   Implementation of mdm_FileManager class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_FileManager.h"

#include <sstream>
#include <functional>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>
#include <madym/utils/mdm_SequenceNames.h>

const int mdm_FileManager::MAX_DYN_IMAGES = 1024;

MDM_API mdm_FileManager::mdm_FileManager(mdm_VolumeAnalysis &volumeAnalysis)
	: 
	volumeAnalysis_(volumeAnalysis),
	writeCtDataMaps_(false),
  writeCtModelMaps_(false),
  applyNiftiScaling_(false),
  imageWriteFormat_(mdm_ImageIO::ImageFormat::NIFTI),
  imageReadFormat_(mdm_ImageIO::ImageFormat::NIFTI),
  xtrType_(mdm_XtrFormat::XTR_type::BIDS)
{
}

MDM_API mdm_FileManager::~mdm_FileManager()
{}

//
MDM_API void mdm_FileManager::loadROI(const std::string &path)
{
	// Read in ROI image volume
  auto setFunc = std::bind(&mdm_VolumeAnalysis::setROI, &volumeAnalysis_, std::placeholders::_1);
  loadAndSetImage(path, "ROI", setFunc, mdm_Image3D::ImageType::TYPE_ROI, false);

}

//
MDM_API void mdm_FileManager::saveROI(const std::string &outputDir, const std::string &name)
{
 if (volumeAnalysis_.ROI())
    saveOutputMap(name, volumeAnalysis_.ROI(), outputDir, false, mdm_ImageDatatypes::DT_UNSIGNED_CHAR);
}

//
MDM_API void mdm_FileManager::loadAIFmap(const std::string &path)
{
  auto setFunc = std::bind(&mdm_VolumeAnalysis::setAIFmap, &volumeAnalysis_, std::placeholders::_1);
  loadAndSetImage(path, "AIF map", setFunc, mdm_Image3D::ImageType::TYPE_ROI, false);
}

//
MDM_API void mdm_FileManager::saveAIFmap(const std::string &outputDir, const std::string &name)
{
  if (volumeAnalysis_.AIFmap())
    saveOutputMap(name, volumeAnalysis_.AIFmap(), outputDir, false);
}

//
MDM_API void mdm_FileManager::loadParameterMaps(const std::string &paramDir,
  const std::vector<int> &initMapParams)
{
  std::vector<std::string> paramNames = volumeAnalysis_.paramNames();

  std::vector<int> params;
  if (initMapParams.empty())
    for (int i = 0; i < paramNames.size(); i++)
      params.push_back(i);
  else
    for (auto i : initMapParams)
      params.push_back(i-1); //Need -1 because user indexing starts at 1

  for (auto i : params)
  {
    std::string paramPath = paramDir + "/" + paramNames[i];

    auto setFunc = std::bind(
      &mdm_VolumeAnalysis::setDCEMap, &volumeAnalysis_, paramNames[i], std::placeholders::_1);
    loadAndSetImage(paramPath, "param map " + paramNames[i], 
      setFunc, mdm_Image3D::ImageType::TYPE_KINETICMAP, false);

  }
  volumeAnalysis_.setInitMapParams(params);

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read param maps from " + paramDir);

}

//
MDM_API void mdm_FileManager::loadModelResiduals(const std::string &path)
{
  auto setFunc = std::bind(
    &mdm_VolumeAnalysis::setDCEMap, &volumeAnalysis_, mdm_VolumeAnalysis::MAP_NAME_RESIDUALS, std::placeholders::_1);
  loadAndSetImage(path, mdm_VolumeAnalysis::MAP_NAME_RESIDUALS, setFunc,
    mdm_Image3D::ImageType::TYPE_KINETICMAP, xtrType_ == mdm_XtrFormat::BIDS);

}

MDM_API void mdm_FileManager::saveGeneralOutputMaps(const std::string& outputDir)
{
  //Write out ROI (if used)
  saveROI(outputDir, volumeAnalysis_.MAP_NAME_ROI);

  //Write out error tracker
  saveErrorTracker(outputDir, volumeAnalysis_.MAP_NAME_ERROR_TRACKER);
}

MDM_API void mdm_FileManager::saveT1OutputMaps(const std::string& outputDir)
{
  //Write out T1 and M0 maps (if M0 map used)
  if (volumeAnalysis_.T1Mapper().T1())
    saveOutputMap(volumeAnalysis_.MAP_NAME_T1,
      volumeAnalysis_.T1Mapper().T1(), outputDir, true);

  if (volumeAnalysis_.T1Mapper().M0())
    saveOutputMap(volumeAnalysis_.MAP_NAME_M0,
      volumeAnalysis_.T1Mapper().M0(), outputDir, true);

  if (volumeAnalysis_.T1Mapper().efficiency())
    saveOutputMap(volumeAnalysis_.MAP_NAME_EFFICIENCY,
      volumeAnalysis_.T1Mapper().efficiency(), outputDir, true);
}

MDM_API void mdm_FileManager::saveDynamicOutputMaps(const std::string& outputDir,
  const  std::string& Ct_sigPrefix, const  std::string& Ct_modPrefix,
  const std::string& indexPattern,
  const int startIndex, const int stepSize)
{
  if (writeCtDataMaps_)
  {
    for (int i = 0; i < volumeAnalysis_.numDynamics(); i++)
    {
      std::string ctName = mdm_SequenceNames::makeSequenceFilename(
        "", Ct_sigPrefix, i + 1, indexPattern,
        startIndex, stepSize);

      if (!i)
        fs::create_directories((fs::path(outputDir) / ctName).parent_path());

      saveOutputMap(ctName, volumeAnalysis_.CtDataMap(i), outputDir, true);
    }
  }
  if (writeCtModelMaps_)
  {
    for (int i = 0; i < volumeAnalysis_.numDynamics(); i++)
    {
      std::string ctName = mdm_SequenceNames::makeSequenceFilename(
        "", Ct_modPrefix, i + 1, indexPattern,
        startIndex, stepSize);
      if (!i)
        fs::create_directories((fs::path(outputDir) / ctName).parent_path());
      saveOutputMap(ctName, volumeAnalysis_.CtModelMap(i), outputDir, true);
    }
  }
}

MDM_API void mdm_FileManager::saveDynamicOutputMaps(const std::string& outputDir,
  const  std::string& Ct_sigPrefix, const  std::string& Ct_modPrefix)
{
  if (writeCtDataMaps_)
  {
    auto saveName = fs::path(outputDir) / Ct_sigPrefix;
    fs::create_directories(saveName.parent_path());
    mdm_ImageIO::writeImage4D(imageWriteFormat_, saveName.string(),
      volumeAnalysis_.CtDataMaps(),
      mdm_ImageDatatypes::DT_FLOAT, xtrType_, applyNiftiScaling_);

  }
  if (writeCtModelMaps_)
  {
    auto saveName = fs::path(outputDir) / Ct_modPrefix;
    fs::create_directories(saveName.parent_path());
    mdm_ImageIO::writeImage4D(imageWriteFormat_, saveName.string(),
      volumeAnalysis_.CtModelMaps(),
      mdm_ImageDatatypes::DT_FLOAT, xtrType_, applyNiftiScaling_);
  }
}

MDM_API void mdm_FileManager::saveDCEOutputMaps(const std::string& outputDir)
{
  //Everything after this point is only applicable to analysis with a DCE model
  if (volumeAnalysis_.modelType().empty())
    return;

  //Write model parameters maps
  {
    for (const auto paramName : volumeAnalysis_.paramNames())
      saveOutputMap(paramName, outputDir, false);

    //Write IAUC maps
    for (const auto t : volumeAnalysis_.IAUCtimes())
    {
      const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + std::to_string(int(t));
      saveOutputMap(iaucName, outputDir, false);
    }
    if (volumeAnalysis_.IAUCAtpeak())
    {
      const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + "_peak";
      saveOutputMap(iaucName, outputDir, false);
    }
    saveOutputMap(volumeAnalysis_.MAP_NAME_ENHANCING, outputDir, false);
  }

  //Write error and enhancing voxels map
  saveModelResiduals(outputDir);

  //Write output stats
  saveSummaryStats(outputDir);
}

MDM_API void mdm_FileManager::saveDWIOutputMaps(const std::string& outputDir)
{
  //Save any diffusion modelling maps
  for (const auto paramName : volumeAnalysis_.DWIMapper().paramNames())
  {
    const auto& map = volumeAnalysis_.DWIMapper().model_map(paramName);
    if (map)
      saveOutputMap(paramName, map, outputDir, false);
  }
}

//
MDM_API void mdm_FileManager::saveModelResiduals(const std::string &outputDir)
{
  saveOutputMap(volumeAnalysis_.MAP_NAME_RESIDUALS, outputDir, false);
}

//
MDM_API void mdm_FileManager::saveSummaryStats(const std::string &outputDir)
{
	//Create a new stats object
	mdm_ParamSummaryStats stats;

	//Do stats for whole ROI
	const auto &ROI = volumeAnalysis_.ROI();
	if (ROI)
		stats.setROI(ROI);

	saveMapsSummaryStats(outputDir + "/" + volumeAnalysis_.MAP_NAME_ROI, stats);

	//Repeat for the enhancing map
	const auto &enh = volumeAnalysis_.DCEMap(volumeAnalysis_.MAP_NAME_ENHANCING);
	if (enh)
	{
		stats.setROI(enh);
		saveMapsSummaryStats(outputDir + "/" + volumeAnalysis_.MAP_NAME_ENHANCING, stats);
	}
}

//
MDM_API void mdm_FileManager::loadErrorTracker(const std::string &path)
{
  // Read in Error tracker
  auto setFunc = std::bind(
    &mdm_ErrorTracker::setErrorImage, &volumeAnalysis_.errorTracker(), std::placeholders::_1);
  loadAndSetImage(path, mdm_VolumeAnalysis::MAP_NAME_ERROR_TRACKER, setFunc,
    mdm_Image3D::ImageType::TYPE_ERRORMAP, false);
}

//
MDM_API void mdm_FileManager::saveErrorTracker(const std::string &outputDir, const std::string &name)
{
  saveOutputMap(name, volumeAnalysis_.errorTracker().errorImage(), outputDir, false,
    mdm_ImageDatatypes::DT_SIGNED_INT);
}

/*Does what is says on the tin*/
MDM_API void mdm_FileManager::loadT1MappingInputImages(const std::vector<std::string> &T1InputPaths, bool useNifti4D)
{
	//Check we haven't been given too many or too few images
	auto nNumImgs = T1InputPaths.size();

	//Loop through filePaths, loaded each variable flip angle images
  for (int i = 0; i < nNumImgs; i++)
  {
    if (useNifti4D)
    {
      //Read in 4D image
      auto imgs = mdm_ImageIO::readImage4D(imageReadFormat_, T1InputPaths[i], true, applyNiftiScaling_);
      imgs[0].setType(mdm_Image3D::ImageType::TYPE_T1WTSPGR);
      if (imgs.size() == 1)
        //If it's just a single 3D image, set in T1 mapper
        volumeAnalysis_.T1Mapper().addInputImage(imgs[0]);

      else
      {
        //Otherwise, compute mean image and set that in T1 mapper
        mdm_Image3D mean_img;
        mean_img.copy(imgs[0]);
        for (const auto& img : imgs)
          mean_img += img;

        mean_img /= imgs.size();
        mean_img.setType(mdm_Image3D::ImageType::TYPE_T1WTSPGR);
        volumeAnalysis_.T1Mapper().addInputImage(mean_img);
      }
    }
    else
    {
      //For 3D input, can just use the standard loadAndSetImage bind function
      auto setFunc = std::bind(
        &mdm_T1Mapper::addInputImage, &volumeAnalysis_.T1Mapper(), std::placeholders::_1);
      loadAndSetImage(T1InputPaths[i], "T1 input", setFunc,
        mdm_Image3D::ImageType::TYPE_T1WTSPGR, true);
    }
    
  }
}

//
MDM_API void mdm_FileManager::loadT1Map(const std::string &path)
{
  auto setFunc = std::bind(
    &mdm_T1Mapper::setT1, &volumeAnalysis_.T1Mapper(), std::placeholders::_1);
  loadAndSetImage(path, "T1", setFunc,
    mdm_Image3D::ImageType::TYPE_T1BASELINE, false);
}

//
MDM_API void mdm_FileManager::loadM0Map(const std::string &path)
{
  auto setFunc = std::bind(
    &mdm_T1Mapper::setM0, &volumeAnalysis_.T1Mapper(), std::placeholders::_1);
  loadAndSetImage(path, "M0", setFunc,
    mdm_Image3D::ImageType::TYPE_M0MAP, false);
}

//
MDM_API void mdm_FileManager::loadB1Map(const std::string &path, const double B1Scaling)
{
  auto setFunc = std::bind(
    &mdm_T1Mapper::setB1, &volumeAnalysis_.T1Mapper(), std::placeholders::_1);
  loadAndSetImage(path, "B1", setFunc,
    mdm_Image3D::ImageType::TYPE_B1MAP, false, B1Scaling);
}

//
MDM_API void mdm_FileManager::loadDWIMappingInputImages(const std::vector<std::string>& DWIInputPaths, bool useNifti4D)
{
  //Check we haven't been given too many or too few images
  auto nNumImgs = DWIInputPaths.size();

  //Loop through filePaths, loaded each variable flip angle images
  for (int i = 0; i < nNumImgs; i++)
  {
    if (useNifti4D)
    {
      //Read in 4D image
      auto imgs = mdm_ImageIO::readImage4D(imageReadFormat_, DWIInputPaths[i], true, applyNiftiScaling_);
      if (imgs.size() == 1)
        //If it's just a single 3D image, set in T1 mapper
        volumeAnalysis_.DWIMapper().addInputImage(imgs[0]);

      else
      {
        //Otherwise, separate into B-values, then take the mean over each
        std::vector< std::vector < mdm_Image3D >> BValsImgs;
        std::vector< double > BVals;

        //First, group the B-values
        for (const auto& img : imgs)
        {
          //See if B-value already found
          auto Bval = img.info().B.value();
          auto itr = std::find(BVals.begin(), BVals.end(), Bval);

          if (itr != BVals.end())
          {
            //If B-value found, append this image to the images vector for that value
            auto index = std::distance(BVals.begin(), itr);
            BValsImgs[index].push_back(img);
          }
          else 
          {
            //If not found, append the B-values, and start a new images vector
            BVals.push_back(Bval);
            BValsImgs.push_back({ img });
          }
        }
        //Now loop again over the B-values, making mean images and adding to the DWI mapper
        for (const auto &BValImgs : BValsImgs)
        { 
          mdm_Image3D mean_img;
          mean_img.copy(BValImgs[0]);
          for (const auto& img : BValImgs)
            mean_img += img;

          mean_img /= BValImgs.size();
          mean_img.setType(mdm_Image3D::ImageType::TYPE_DWI);
          volumeAnalysis_.DWIMapper().addInputImage(mean_img);
        }
        
      }
    }
    else
    {
      auto setFunc = std::bind(
        &mdm_DWIMapper::addInputImage, &volumeAnalysis_.DWIMapper(), std::placeholders::_1);
      loadAndSetImage(DWIInputPaths[i], "DWI input", setFunc,
        mdm_Image3D::ImageType::TYPE_DWI, true);
    }
    
  }
}

//
MDM_API void mdm_FileManager::loadDynamicTimeseries(const std::string &dynBasePath,
	const std::string &dynPrefix, int nDyns, const std::string &indexPattern,
  const int startIndex, const int stepSize, bool Ct)
{
	bool dynFilesExist = true;
	int nDyn = 0;

	//Set flags for missing data and reaching max images based on whether
	//nDyns was set or not
	bool errorIfMissing, warnIfMax;

	if (nDyns <= 0)
	{
		//If nDyns not set, keep reading images until we hit memory max
		//warn if we hit max, but don't error when we run out of images
		nDyns = MAX_DYN_IMAGES;
		errorIfMissing = false;
		warnIfMax = true;
	}
	else
	{
		//If nDyns not set, read in nDyns images
		//Don't warn if we hit max, but error if we run out of images
		errorIfMissing = true;
		warnIfMax = false;
	}

	while (dynFilesExist)
	{
		if (nDyn == nDyns)
		{
			if (warnIfMax)
				mdm_ProgramLogger::logProgramWarning(__func__,
					"Reached maximum number of images " + std::to_string(MAX_DYN_IMAGES));
			break;
		}

		nDyn++;

		std::string dynPath = mdm_SequenceNames::makeSequenceFilename(
      dynBasePath, dynPrefix, nDyn, indexPattern,
      startIndex, stepSize);

		if (!mdm_ImageIO::filesExist(imageReadFormat_, dynPath, false))
		{
			//If nDyns was set, we expect to load in that many images, so error and return false
			//if any of them don't exist
			if (errorIfMissing)
        throw mdm_exception(__func__, dynPath + " does not exist.");
				

			//However if nDyns wasn't set, this is expected behaviour - we keep loading images
			//until we don't find them - just set dynFilesExist to false so we break the loop
			dynFilesExist = false;
		}
		else
		{
      if (Ct)
      {
        auto msg = "concentration map " + std::to_string(nDyn);
        auto setFunc = std::bind(
          &mdm_VolumeAnalysis::addCtDataMap, &volumeAnalysis_, std::placeholders::_1);
        loadAndSetImage(dynPath, msg, setFunc,
          mdm_Image3D::ImageType::TYPE_CAMAP, true);
      }
      else
      {
        auto msg = "dynamic image " + std::to_string(nDyn);
        auto setFunc = std::bind(
          &mdm_VolumeAnalysis::addStDataMap, &volumeAnalysis_, std::placeholders::_1);
        loadAndSetImage(dynPath, msg, setFunc,
          mdm_Image3D::ImageType::TYPE_T1DYNAMIC, true);
      }
      
		}
	}
}

//
MDM_API void mdm_FileManager::loadDynamicTimeseries(const std::string& basePath,
  const std::string& StName, bool Ct)
{
  auto imgName = basePath.empty() ? StName :
    (fs::path(basePath) / StName).string();
  auto imgs = mdm_ImageIO::readImage4D(imageReadFormat_, imgName, true, applyNiftiScaling_);
  double dynTime = 0;
  double temp_res = imgs[0].info().temporalResolution.isSet() ?
    imgs[0].info().temporalResolution.value() : 0;
  for (auto img : imgs)
  {
    //If set, use the temporal resolution field, otherwise assume acquisition times
    //have been set from JSON meta file
    if (temp_res)
      img.setTimeStampFromSecs(dynTime);
    if (Ct)
    {
      img.setType(mdm_Image3D::ImageType::TYPE_CAMAP);
      volumeAnalysis_.addCtDataMap(img);
    }
    else
    {
      img.setType(mdm_Image3D::ImageType::TYPE_T1DYNAMIC);
      volumeAnalysis_.addStDataMap(img);
    }
    if (temp_res)
      dynTime += temp_res;
  }

}

MDM_API void mdm_FileManager::setSaveCtDataMaps(bool b)
{
  writeCtDataMaps_ = b;
}

MDM_API void mdm_FileManager::setSaveCtModelMaps(bool b)
{
  writeCtModelMaps_ = b;
}

//
MDM_API void mdm_FileManager::setImageReadFormat(const std::string &fmt)
{
  imageReadFormat_ = mdm_ImageIO::formatFromString(fmt);
}

//
MDM_API void mdm_FileManager::setImageWriteFormat(const std::string &fmt)
{
  imageWriteFormat_ = mdm_ImageIO::formatFromString(fmt);
}

//
MDM_API void mdm_FileManager::setApplyNiftiScaling(bool flag)
{
  applyNiftiScaling_ = flag;
}

//
MDM_API void mdm_FileManager::setXtrType(bool use_bids)
{
  xtrType_ = use_bids ? 
    mdm_XtrFormat::XTR_type::BIDS : mdm_XtrFormat::XTR_type::NEW_XTR;
}

//---------------------------------------------------------------------------
//Private functions
//---------------------------------------------------------------------------

void mdm_FileManager::saveOutputMap(
  const std::string &mapName, const std::string &outputDir, bool writeXtr/* = true*/)
{
	const mdm_Image3D &img = volumeAnalysis_.DCEMap(mapName);
  if (img)
    saveOutputMap(mapName, img, outputDir, writeXtr);
}

void mdm_FileManager::saveOutputMap(const std::string &mapName, const mdm_Image3D &img, 
  const std::string &outputDir, bool writeXtr/* = true*/,
  const mdm_ImageDatatypes::DataType format /*= mdm_ImageDatatypes::DT_FLOAT*/)
{
	std::string saveName = (fs::path(outputDir) / mapName).string();

  mdm_XtrFormat::XTR_type xtr = writeXtr ? 
    xtrType_ : 
    mdm_XtrFormat::XTR_type::NO_XTR;


  try {
    mdm_ImageIO::writeImage3D(imageWriteFormat_, saveName, img,
      format, xtr, applyNiftiScaling_);
  }
  catch (mdm_exception &e)
  {
    e.append("Failed to write output map " + mapName);
    throw;
  }
}

void mdm_FileManager::saveMapsSummaryStats(const std::string &roiName, mdm_ParamSummaryStats &stats)
{
	stats.writeROISummary(roiName + "_summary.txt");
  stats.openNewStatsFile(roiName + "_summary_stats.csv");

	//Write out T1 and M0 maps (if M0 map used)
	if (volumeAnalysis_.T1Mapper().T1())
		saveMapSummaryStats(volumeAnalysis_.MAP_NAME_T1, 
      volumeAnalysis_.T1Mapper().T1(), stats);

	if (volumeAnalysis_.T1Mapper().M0())
		saveMapSummaryStats(volumeAnalysis_.MAP_NAME_M0, 
      volumeAnalysis_.T1Mapper().M0(), stats);

	//Write model parameters maps
	if (!volumeAnalysis_.modelType().empty())
	{
		const auto &paramNames = volumeAnalysis_.paramNames();
		for (const auto mapName : paramNames)
			saveMapSummaryStats(mapName, volumeAnalysis_.DCEMap(mapName), stats);

		//Write IAUC maps
		const auto &IAUCtimes = volumeAnalysis_.IAUCtimes();
		for (const auto time : IAUCtimes)
		{
			const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + std::to_string(int(time));
			saveMapSummaryStats(iaucName, volumeAnalysis_.DCEMap(iaucName), stats);
		}
    if (volumeAnalysis_.IAUCAtpeak())
    {
      const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + "_peak";
      saveMapSummaryStats(iaucName, volumeAnalysis_.DCEMap(iaucName), stats);
    }

		saveMapSummaryStats(volumeAnalysis_.MAP_NAME_ENHANCING,
			volumeAnalysis_.DCEMap(volumeAnalysis_.MAP_NAME_ENHANCING), stats);
	}
  stats.closeNewStatsFile();
	 
}

//
void mdm_FileManager::saveMapSummaryStats(const std::string &mapName, const mdm_Image3D &img, mdm_ParamSummaryStats &stats)
{
	stats.makeStats(img, mapName);
	stats.writeStats();
}

//
template <class T> void  mdm_FileManager::loadAndSetImage(
  const std::string &path, const std::string &msgName, T setFunc,
  const mdm_Image3D::ImageType type, bool loadXtr, double scaling)
{
  try {
    //Read in image and set type
    mdm_Image3D img = mdm_ImageIO::readImage3D(imageReadFormat_, path, loadXtr, applyNiftiScaling_);
    img.setType(type);

    //Scale if necessary
    if (scaling && scaling != 1)
      img /= scaling;

    //Call the appropriate volumeAnalysis set function based on functor input
    setFunc(img);
  }
  catch (mdm_dimension_mismatch &e)
  {
    //Dimension errors all cause a breaking exception
    e.append("Dimension error reading " + msgName + " from " + path);
    throw;
  }
  catch (mdm_voxelsize_mismatch &e)
  {
    //Voxel size errors can be caught and replaced with a warning if user opts
    e.append("Voxel size error reading " + msgName + " from " + path);
    throw;
  }
  catch (mdm_exception &e)
  {
    //Any other error (eg loading) should break
    e.append("Error reading " + msgName + " from " + path);
    throw;
  }
  mdm_ProgramLogger::logProgramMessage(
    msgName + " loaded from " + path);
}

/*template void  mdm_FileManager::setImage< std::function<void(mdm_Image3D)> >(
  const std::string &msgName,
  std::function<void(mdm_Image3D)> setFunc);

template void  mdm_FileManager::setImage< std::function<void(const std::string&, mdm_Image3D)> >(
  const std::string &msgName,
  std::function<void(const std::string&, mdm_Image3D)> setFunc);*/


