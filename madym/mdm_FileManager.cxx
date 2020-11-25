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
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

#include "mdm_version.h"
#include "mdm_AnalyzeFormat.h"
#include "mdm_ProgramLogger.h"

const int mdm_FileManager::MAX_DYN_IMAGES = 1024;

MDM_API mdm_FileManager::mdm_FileManager(mdm_DCEVolumeAnalysis &volumeAnalysis)
	: 
	volumeAnalysis_(volumeAnalysis),
	T1Mapper_(volumeAnalysis.T1Mapper()),
	errorTracker_(volumeAnalysis.errorTracker()),
	writeCtDataMaps_(false),
  writeCtModelMaps_(false),
	sparseWrite_(false)
{
}

MDM_API mdm_FileManager::~mdm_FileManager()
{}

//
MDM_API bool mdm_FileManager::loadROI(const std::string &path)
{
	// Read in ROI image volume
	mdm_Image3D ROI = mdm_AnalyzeFormat::readImage3D(path, false);
  ROI.setType(mdm_Image3D::ImageType::TYPE_ROI);

	if (!ROI.numVoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadROI: Failed to read ROI " + path + "\n");
		return false;
	}
	volumeAnalysis_.setROI(ROI);
	T1Mapper_.addROI(ROI);

	mdm_ProgramLogger::logProgramMessage(
		"ROI loaded from " + path + "\n");

	return true;
}

//
MDM_API bool mdm_FileManager::saveROI(const std::string &outputDir, const std::string &name)
{
  return (!volumeAnalysis_.ROI().numVoxels() ||
    saveOutputMap(name, volumeAnalysis_.ROI(), outputDir, false));
}

//
MDM_API bool mdm_FileManager::loadAIFmap(const std::string &path)
{
  // Read in AIF map image volume
  mdm_Image3D AIFmap = mdm_AnalyzeFormat::readImage3D(path, false);
  AIFmap.setType(mdm_Image3D::ImageType::TYPE_ROI);

  if (!AIFmap.numVoxels())
  {
    mdm_ProgramLogger::logProgramMessage(
      "ERROR: mdm_FileManager::loadROI: Failed to read ROI " + path + "\n");
    return false;
  }
  volumeAnalysis_.setAIFmap(AIFmap);

  mdm_ProgramLogger::logProgramMessage(
    "AIF map loaded loaded from " + path + "\n");

  return true;
}

//
MDM_API bool mdm_FileManager::saveAIFmap(const std::string &outputDir, const std::string &name)
{
  return (!volumeAnalysis_.AIFmap().numVoxels() ||
    saveOutputMap(name, volumeAnalysis_.AIFmap(), outputDir, false));
}

//
MDM_API bool mdm_FileManager::loadParameterMaps(const std::string &paramDir)
{
  //Write model parameters maps
  std::vector<std::string> paramNames = volumeAnalysis_.paramNames();
  for (int i = 0; i < paramNames.size(); i++)
  {
    std::string paramName = paramDir + "/" + paramNames[i];

    mdm_Image3D paramMap = mdm_AnalyzeFormat::readImage3D(paramName, false);
    paramMap.setType(mdm_Image3D::ImageType::TYPE_KINETICMAP);

    if (!paramMap.numVoxels())
    {
			mdm_ProgramLogger::logProgramMessage(
				"ERROR: mdm_FileManager::loadParameterMaps:  Failed to read param map from " + paramName + "\n");
      return false;
    }
    volumeAnalysis_.setDCEMap(paramNames[i], paramMap);
  }
	mdm_ProgramLogger::logProgramMessage(
		"Successfully read param maps from " + paramDir + "\n");
  return true;
}

MDM_API bool mdm_FileManager::saveOutputMaps(const std::string &outputDir)
{
  //Write out ROI (if used)
  if (!saveROI(outputDir, volumeAnalysis_.MAP_NAME_ROI))
    return true;

	//Write out T1 and M0 maps (if M0 map used)
	if (T1Mapper_.T1().numVoxels() && 
    !saveOutputMap(volumeAnalysis_.MAP_NAME_T1, T1Mapper_.T1(), outputDir, true))
		return false;
	if (T1Mapper_.M0().numVoxels() && 
		!saveOutputMap(volumeAnalysis_.MAP_NAME_M0, T1Mapper_.M0(), outputDir, true))
		return false;

  //Write model parameters maps
  if (!volumeAnalysis_.modelType().empty())
  {
    std::vector<std::string> paramNames = volumeAnalysis_.paramNames();
    for (int i = 0; i < paramNames.size(); i++)
      if (!saveOutputMap(paramNames[i], outputDir, false))
        return false;

    //Write IAUC maps
    std::vector<double> IAUCtimes = volumeAnalysis_.IAUCtimes();
    for (int i = 0; i < IAUCtimes.size(); i++)
    {
      const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + std::to_string(int(IAUCtimes[i]));
      if (!saveOutputMap(iaucName, outputDir, false))
        return false;
    }
    if (!saveOutputMap(volumeAnalysis_.MAP_NAME_ENHANCING, outputDir, false))
      return false;
  }
	
	//Write error and enhancing voxels map
  if (!saveModelResiduals(outputDir))
		return false;

	//Write output stats
	if (!saveSummaryStats(outputDir))
		return false;

	//If used, write ROI
	if (volumeAnalysis_.ROI().numVoxels() && 
    !saveOutputMap(volumeAnalysis_.MAP_NAME_ROI, volumeAnalysis_.ROI(), outputDir, false))
		return false;

	if (writeCtDataMaps_)
	{
		const std::string &ctSigPrefix = volumeAnalysis_.MAP_NAME_CT_SIG;
		for (int i = 0; i < volumeAnalysis_.numDynamics(); i++)
		{
			std::string catName = ctSigPrefix + std::to_string(i + 1);
			if (!saveOutputMap(catName, volumeAnalysis_.CtDataMap(i), outputDir, true))
				return false;
		}
	}
  if (writeCtModelMaps_)
  {
    const std::string &ctModPrefix = volumeAnalysis_.MAP_NAME_CT_MOD;
    for (int i = 0; i < volumeAnalysis_.numDynamics(); i++)
    {
      std::string cmodName = ctModPrefix + std::to_string(i + 1);
      if (!saveOutputMap(cmodName, volumeAnalysis_.CtModelMap(i), outputDir, false))
        return false;
    }
  }
	return true;
}

//
MDM_API bool mdm_FileManager::saveModelResiduals(const std::string &outputDir)
{
  return saveOutputMap(volumeAnalysis_.MAP_NAME_RESDIUALS, outputDir, false);
}

//
MDM_API bool mdm_FileManager::saveSummaryStats(const std::string &outputDir)
{
	//Create a new stats object
	mdm_ParamSummaryStats stats;

	//Do stats for whole ROI
	const auto &roi = volumeAnalysis_.ROI();
	if (roi.numVoxels())
		stats.setROI(roi);

	if (!saveMapsSummaryStats(outputDir + "/" + volumeAnalysis_.MAP_NAME_ROI, stats))
		return false;
	

	//Repeat for the enhancing map
	const auto &enh = volumeAnalysis_.DCEMap(volumeAnalysis_.MAP_NAME_ENHANCING);
	if (enh.numVoxels())
	{
		stats.setROI(enh);
		if (!saveMapsSummaryStats(outputDir + "/" + volumeAnalysis_.MAP_NAME_ENHANCING, stats))
			return false;
	}
	

	return true;

}

//
MDM_API bool mdm_FileManager::saveErrorMap(const std::string &outputPath)
{
	const mdm_Image3D &img = errorTracker_.errorImage();
	if (!img.numVoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::saveErrorMap: Error codes map is empty\n");
		return false;
	}
		
	if (!mdm_AnalyzeFormat::writeImage3D(outputPath, img,
		mdm_AnalyzeFormat::DT_SIGNED_INT, mdm_AnalyzeFormat::NO_XTR, sparseWrite_))
	{
		// write msg to errLog (failed to write image to file)
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::saveErrorMap: Failed to write " + outputPath + "\n");
		return false;
	}
	return true;
}

MDM_API void mdm_FileManager::setSaveCtDataMaps(bool b)
{
	writeCtDataMaps_ = b;
}

MDM_API void mdm_FileManager::setSaveCtModelMaps(bool b)
{
  writeCtModelMaps_ = b;
}

MDM_API void mdm_FileManager::setSparseWrite(bool b)
{
	sparseWrite_ = b;
}

MDM_API bool mdm_FileManager::loadErrorMap(const std::string &errorPath, bool warnMissing)
{
	//To avoid triggering all the warnings for missing images (given we speculatively
	//try to load the error on the chance it may exist, but happily create it if it
	//doesn't - do our own check here and just return false if it doesn't exist yet
	//This avoids having to put a warning check on all our analyze read functions
	// (this is the only case where we don't care if an image loads)
	if (!warnMissing)
	{
		bool b;
		if (!mdm_AnalyzeFormat::filesExist(errorPath, b, false))
			return false;		
	}
		
	mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(errorPath, false);
	img.setType(mdm_Image3D::ImageType::TYPE_ERRORMAP);

	//Don't bother with a file manager check here - the errorTracker will
	//check image is non-zero size and of correct type
	if (errorTracker_.setErrorImage(img))
	{
		mdm_ProgramLogger::logProgramMessage(
			"Successfully read error codes image from " + errorPath + "\n");
		return true;
	}
	else
		return false;
}

/*Does what is says on the tin*/
MDM_API bool mdm_FileManager::loadT1MappingInputImages(const std::vector<std::string> &T1InputPaths)
{
	//Check we haven't been given too many or too few images
	auto nNumImgs = T1InputPaths.size();

	//Loop through filePaths, loaded each variable flip angle images
	for (int i = 0; i < nNumImgs; i++)
	{
		if (!loadT1InputImage(T1InputPaths[i], i + 1))
			return false;

	}
	return true;
}

MDM_API bool mdm_FileManager::loadT1Map(const std::string &T1path)
{
	mdm_Image3D T1_map = mdm_AnalyzeFormat::readImage3D(T1path, false);
  T1_map.setType(mdm_Image3D::ImageType::TYPE_T1BASELINE);

	if (!T1_map.numVoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadT1Map: Failed to read T1 map from " + T1path + "\n");
		return false;
	}

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read T1 map from " + T1path + "\n");

	//If image successfully read, add it to the T1 mapper object
	T1Mapper_.addT1Map(T1_map);
	return true;
}

MDM_API bool mdm_FileManager::loadM0Map(const std::string &M0path)
{
	mdm_Image3D M0_map = mdm_AnalyzeFormat::readImage3D(M0path, false);
  M0_map.setType(mdm_Image3D::ImageType::TYPE_M0MAP);

	if (!M0_map.numVoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadM0Map: Failed to read M0 map from " + M0path + "\n");
		return false;
	}

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read M0 map from " + M0path + "\n");

	//If image successfully read, add it to the T1 mapper object
	T1Mapper_.addM0Map(M0_map);
	return true;
}

MDM_API bool mdm_FileManager::loadStDataMaps(const std::string &dynBasePath,
	const std::string &dynPrefix, int nDyns, const std::string &indexPattern)
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
				mdm_ProgramLogger::logProgramMessage(
					"WARNING: mdm_FileManager::loadStDataMaps: reached maximum number of images "
					+ std::to_string(MAX_DYN_IMAGES));
			break;
		}

		nDyn++;

		/* This sets various globals (see function header comment) */
		std::string dynPath;
		makeSequenceFilename(dynBasePath, dynPrefix, nDyn, dynPath, indexPattern);

		if (!boost::filesystem::exists(dynPath))
		{
			//If nDyns was set, we expect to load in that many images, so error and return false
			//if any of them don't exist
			if (errorIfMissing)
			{
				mdm_ProgramLogger::logProgramMessage(
					"ERROR: mdm_FileManager::loadStDataMaps: " + dynPath + " does not exist.");
				return false;
			}

			//However if nDyns wasn't set, this is expected behaviour - we keep loading images
			//until we don't find them - just set catFilesExist to false so we break the loop
			dynFilesExist = false;
		}
		else
		{
			mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(dynPath, true);
      img.setType(mdm_Image3D::ImageType::TYPE_T1DYNAMIC);

			if (!img.numVoxels())
			{
				mdm_ProgramLogger::logProgramMessage(
					"ERROR: mdm_FileManager::loadStDataMaps:  Failed to read dynamic image " +
					std::to_string(nDyn) + " from " + dynPath + "\n");
				return false;
			}
			//If successfully loaded, add image to the volume analysis
			volumeAnalysis_.addStDataMap(img);

			mdm_ProgramLogger::logProgramMessage(
				"Successfully read dynamic image " +
				std::to_string(nDyn) + " from " + dynPath + "\n");

			//For first image, try initialising errorImage, if that's already been
			//set it will just return true
			if (nDyn == 1)
				errorTracker_.initErrorImage(img);
		}
	}

	return true;
}

MDM_API bool mdm_FileManager::loadCtDataMaps(const std::string &catBasePath,
	const std::string &catPrefix, int nDyns, const std::string &indexPattern)
{
	bool catFilesExist = true;
	int nCat = 0;

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

	while (catFilesExist)
	{
		if (nCat == nDyns)
		{
			if (warnIfMax)
				mdm_ProgramLogger::logProgramMessage(
					"WARNING: mdm_FileManager::loadCtDataMaps: reached maximum number of images " 
					+ std::to_string(MAX_DYN_IMAGES));
			break;
		}
		nCat++;

		/* This sets various globals (see function header comment) */
		std::string catPath;
		makeSequenceFilename(catBasePath, catPrefix, nCat, catPath, indexPattern);

		if (!boost::filesystem::exists(catPath))
		{
			//If nDyns was set, we expect to load in that many images, so error and return false
			//if any of them don't exist
			if (errorIfMissing)
			{
				mdm_ProgramLogger::logProgramMessage(
					"ERROR: mdm_FileManager::loadCtDataMaps: " + catPath + " does not exist.");
				return false;
			}
			
			//However if nDyns wasn't set, this is expected behaviour - we keep loading images
			//until we don't find them - just set catFilesExist to false so we break the loop
			catFilesExist = false;
		}
		else
		{
			mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(catPath, true);
      img.setType(mdm_Image3D::ImageType::TYPE_CAMAP);

			if (!img.numVoxels())
			{
				mdm_ProgramLogger::logProgramMessage(
					"ERROR: mdm_FileManager::loadCtDataMaps: Failed to read concentration image " +
					std::to_string(nCat) + " from " + catPath + "\n");
				return false;
			}
			//If successfully loaded, add image to the volume analysis
			volumeAnalysis_.addCtDataMap(img);

			mdm_ProgramLogger::logProgramMessage(
				"Successfully read concentration image  " +
				std::to_string(nCat) + " from " + catPath + "\n");

			//For first image, try initialising errorImage, if that's already been
			//set it will just return true
			if (nCat == 1)
				errorTracker_.initErrorImage(img);
		}
	}

	return true;
}

bool mdm_FileManager::loadT1InputImage(const std::string& filePath, int nVFA)
{
	mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(filePath, true);
  img.setType(mdm_Image3D::ImageType::TYPE_T1WTSPGR);

	if (!img.numVoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadFAImage: Failed to read T1 input image " + 
			std::to_string(nVFA) + " file from " + filePath + "\n");
		return false;
	}

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read T1 input image " + std::to_string(nVFA) + " from " + filePath + "\n");

	//If image successfully read, add it to the T1 mapper object
	T1Mapper_.addInputImage(img);

	//For first image, try initialising errorImage, if that's already been
	//set it will just return true
	if (nVFA == 1)
		errorTracker_.initErrorImage(img);

	return true;
}

bool mdm_FileManager::saveOutputMap(const std::string &mapName, const std::string &outputDir, bool writeXtr/* = true*/)
{
	const mdm_Image3D &img = volumeAnalysis_.DCEMap(mapName);
  if (img.numVoxels())
    return saveOutputMap(mapName, img, outputDir, writeXtr);
  else
    return true;
}

bool mdm_FileManager::saveOutputMap(const std::string &mapName, const mdm_Image3D &img, const std::string &outputDir, bool writeXtr/* = true*/)
{
	std::string saveName = outputDir + "/" + mapName;

	mdm_AnalyzeFormat::XTR_type xtr = (writeXtr ? mdm_AnalyzeFormat::XTR_type::NEW_XTR : mdm_AnalyzeFormat::XTR_type::NO_XTR);

	if (!mdm_AnalyzeFormat::writeImage3D(saveName, img,
		mdm_AnalyzeFormat::DT_FLOAT, xtr, sparseWrite_))
	{
		// write msg to errLog (failed to write image to file)
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::saveOutputMap: Failed to write " + mapName + "\n");
		return false;
	}
	return true;
}

bool mdm_FileManager::saveMapsSummaryStats(const std::string &roiName, mdm_ParamSummaryStats &stats)
{
	if (!stats.writeROISummary(roiName + "_summary.txt"))
		return false;
	if (!stats.openNewStatsFile(roiName + "_summary_stats.csv"))
		return false;

	//Write out T1 and M0 maps (if M0 map used)
	if (T1Mapper_.T1().numVoxels() &&
		!saveMapSummaryStats(volumeAnalysis_.MAP_NAME_T1, T1Mapper_.T1(), stats))
		return false;

	if (T1Mapper_.M0().numVoxels() &&
		!saveMapSummaryStats(volumeAnalysis_.MAP_NAME_M0, T1Mapper_.M0(), stats))
		return false;

	//Write model parameters maps
	if (!volumeAnalysis_.modelType().empty())
	{
		const auto &paramNames = volumeAnalysis_.paramNames();
		for (const auto mapName : paramNames)
			if (!saveMapSummaryStats(mapName, volumeAnalysis_.DCEMap(mapName), stats))
				return false;

		//Write IAUC maps
		const auto &IAUCtimes = volumeAnalysis_.IAUCtimes();
		for (const auto time : IAUCtimes)
		{
			const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + std::to_string(int(time));
			if (!saveMapSummaryStats(iaucName, volumeAnalysis_.DCEMap(iaucName), stats))
				return false;
		}
		if (!saveMapSummaryStats(volumeAnalysis_.MAP_NAME_ENHANCING,
			volumeAnalysis_.DCEMap(volumeAnalysis_.MAP_NAME_ENHANCING), stats))
			return false;
	}

	return stats.closeNewStatsFile();
}

//
bool mdm_FileManager::saveMapSummaryStats(const std::string &mapName, const mdm_Image3D &img, mdm_ParamSummaryStats &stats)
{
	stats.makeStats(img, mapName);
	return stats.writeStats();
}

//
void mdm_FileManager::makeSequenceFilename(const std::string &path, const std::string &prefix,
	const int fileNumber, std::string &filePath, const std::string &fileNumberFormat)
{
	auto formattedFilenumber = boost::format(fileNumberFormat.c_str()) % fileNumber;
	auto imageName = boost::format("%1%%2%.img") % prefix % formattedFilenumber;
	filePath = (fs::path(path) / imageName.str()).string();
}

