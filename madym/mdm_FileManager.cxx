/**
 *  @file    mdm_fileLoad.c
 *  @brief   File-loading for madym
 *
 *  Original Author GJM Parker 2002
 *  Moved to this file by GA Buonaccorsi
 *  (c) Copyright ISBE, University of Manchester 2002
 *
 *  GAB mods:
 *  21 April 2004
 *  -  Copied DefineFilePopup.c active loading function here
 *     and did some serious mods
 *  -  Stripped all references to XView GUI
 *  24 October 2006
 *  - Modifications to allow concentration time series reading. GJMP & GAB. Version 1.06.
 *  28 Feb 2012
 *  - Big refactor at Bioxydyn Version 1.21.alpha
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_FileManager.h"

#include <sstream>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "mdm_version.h"
#include "mdm_AnalyzeFormat.h"
#include "mdm_T1Voxel.h"

#include "mdm_ProgramLogger.h"

const int MAX_IMAGES = 1024;

MDM_API mdm_FileManager::mdm_FileManager(mdm_AIF &AIF,
	mdm_T1VolumeAnalysis &T1Mapper,
	mdm_DCEVolumeAnalysis &volumeAnalysis,
	mdm_ErrorTracker &errorTracker)
	: AIF_(AIF),
	T1Mapper_(T1Mapper),
	volumeAnalysis_(volumeAnalysis),
	errorTracker_(errorTracker),
	FAPaths_(0),
	dynPaths_(0),
	catPaths_(0),
	T1Path_(""),
	S0Path_(""),
	AIFPath_(""),
  PIFPath_(""),
	ROIPath_(""),
	writeCtDataMaps_(false),
  writeCtModelMaps_(false),
	sparseWrite_(false)
{

}

MDM_API mdm_FileManager::~mdm_FileManager()
{}

/*This depends on whther using an auto or population AIF*/
MDM_API bool mdm_FileManager::loadAIF(const std::string &AIFpath)
{
	bool success = AIF_.readAIF(AIFpath, volumeAnalysis_.getNDyns());
	if (success)
		AIFPath_ = AIFpath;

	return success;
}

MDM_API bool mdm_FileManager::loadPIF(const std::string &PIFpath)
{
  bool success = AIF_.readPIF(PIFpath, volumeAnalysis_.getNDyns());
  if (success)
    PIFPath_ = PIFpath;

  return success;
}

MDM_API bool mdm_FileManager::loadROI(const std::string &ROIpath)
{
	// Read in ROI image volume
	mdm_Image3D ROI_image = mdm_AnalyzeFormat::readImage3D(ROIpath, false);

	if (!ROI_image.getNvoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadROI: Failed to read ROI " + ROIpath + "\n");
		return false;
	}
	volumeAnalysis_.setROIimage(ROI_image);
	T1Mapper_.addROI(ROI_image);

	mdm_ProgramLogger::logProgramMessage(
		"ROI loaded from " + ROIpath + "\n");
	ROIPath_ = ROIpath;

	return true;
}

MDM_API bool mdm_FileManager::loadParameterMaps(const std::string &paramDir)
{
  //Write model parameters maps
  std::vector<std::string> paramNames = volumeAnalysis_.paramNames();
  for (int i = 0; i < paramNames.size(); i++)
  {
    std::string paramName = paramDir + "/" + paramNames[i];

    mdm_Image3D paramMap = mdm_AnalyzeFormat::readImage3D(paramName, false);

    if (!paramMap.getNvoxels())
    {
			mdm_ProgramLogger::logProgramMessage(
				"ERROR: mdm_FileManager::loadParameterMaps:  Failed to read param map from " + paramName + "\n");
      return false;
    }
    volumeAnalysis_.setModelMap(paramNames[i], paramMap);
  }
	mdm_ProgramLogger::logProgramMessage(
		"Successfully read param maps from " + paramDir + "\n");
  return true;
}

MDM_API bool mdm_FileManager::writeOutputMaps(const std::string &outputDir)
{
	//Write out T1 and S0 maps (if S0 map used)
	if (T1Mapper_.T1Map().getNvoxels() && 
    !writeOutputMap(volumeAnalysis_.MAP_NAME_T1, T1Mapper_.T1Map(), outputDir, true))
		return false;
	if (T1Mapper_.S0Map().getNvoxels() && 
		!writeOutputMap(volumeAnalysis_.MAP_NAME_S0, T1Mapper_.S0Map(), outputDir, true))
		return false;

  //Write model parameters maps
  if (!volumeAnalysis_.modelType().empty())
  {
    std::vector<std::string> paramNames = volumeAnalysis_.paramNames();
    for (int i = 0; i < paramNames.size(); i++)
      if (!writeOutputMap(paramNames[i], outputDir, false))
        return false;

    //Write IAUC maps
    std::vector<double> IAUCtimes = volumeAnalysis_.IAUCtimes();
    for (int i = 0; i < IAUCtimes.size(); i++)
    {
      const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + std::to_string(int(IAUCtimes[i]));
      if (!writeOutputMap(iaucName, outputDir, false))
        return false;
    }
    if (!writeOutputMap(volumeAnalysis_.MAP_NAME_ENHANCING, outputDir, false))
      return false;
  }
	
	//Write error and enhancing voxels map
  if (!writeModelResiduals(outputDir))
		return false;

	//If used, write ROI
	if (volumeAnalysis_.ROIimage().getNvoxels() && 
    !writeOutputMap(volumeAnalysis_.MAP_NAME_ROI, volumeAnalysis_.ROIimage(), outputDir, false))
		return false;

	/** 1.22 */
	if (writeCtDataMaps_)
	{
		const std::string &ctSigPrefix = volumeAnalysis_.MAP_NAME_CT_SIG;
		for (int i = 0; i < volumeAnalysis_.getNDyns(); i++)
		{
			std::string catName = ctSigPrefix + std::to_string(i + 1);
			if (!writeOutputMap(catName, volumeAnalysis_.CtDataMap(i), outputDir, true))
				return false;
		}
	}
  if (writeCtModelMaps_)
  {
    const std::string &ctModPrefix = volumeAnalysis_.MAP_NAME_CT_MOD;
    for (int i = 0; i < volumeAnalysis_.getNDyns(); i++)
    {
      std::string cmodName = ctModPrefix + std::to_string(i + 1);
      if (!writeOutputMap(cmodName, volumeAnalysis_.CtModelMap(i), outputDir, false))
        return false;
    }
  }
	return true;
}

MDM_API bool mdm_FileManager::writeModelResiduals(const std::string &outputDir)
{
  return writeOutputMap(volumeAnalysis_.MAP_NAME_RESDIUALS, outputDir, false);
}

MDM_API bool mdm_FileManager::writeErrorMap(const std::string &outputPath)
{
	const mdm_Image3D &img = errorTracker_.errorImage();
	if (!img.getNvoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::writeErrorMap: Error codes map is empty\n");
		return false;
	}
		
	if (!mdm_AnalyzeFormat::writeImage3D(outputPath, img,
		mdm_AnalyzeFormat::DT_SIGNED_INT, mdm_AnalyzeFormat::NO_XTR, sparseWrite_))
	{
		// write msg to errLog (failed to write image to file)
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::writeErrorMap: Failed to write " + outputPath + "\n");
		return false;
	}
	return true;
}

MDM_API void mdm_FileManager::setWriteCtDataMaps(bool b)
{
	writeCtDataMaps_ = b;
}

MDM_API void mdm_FileManager::setWriteCtModelMaps(bool b)
{
  writeCtModelMaps_ = b;
}

MDM_API void mdm_FileManager::setSparseWrite(bool b)
{
	sparseWrite_ = b;
}

MDM_API bool mdm_FileManager::saveAIF(const std::string &AIFpath)
{
  return AIF_.writeAIF(AIFpath);
}

MDM_API bool mdm_FileManager::savePIF(const std::string &PIFpath)
{
  return AIF_.writeAIF(PIFpath);
}

MDM_API bool mdm_FileManager::loadErrorImage(const std::string &errorPath, bool warnMissing)
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
	img.setType(mdm_Image3D::imageType::TYPE_ERRORMAP);

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
MDM_API bool mdm_FileManager::loadFAImages(const std::vector<std::string> &FApaths)
{
	//Check we haven't been given too many or too few images
	auto nVFA = FApaths.size();

	if (nVFA < mdm_T1Voxel::MINIMUM_FAS)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadFAImages: Not enough FA image paths supplied, given " +
			std::to_string(nVFA) + ", require at least " + 
			std::to_string(mdm_T1Voxel::MINIMUM_FAS));
		return false;
	}
	else if (nVFA > mdm_T1Voxel::MAXIMUM_FAS)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadFAImages: Too many FA image paths supplied, given " +
			std::to_string(nVFA) + ", require at most " + 
			std::to_string(mdm_T1Voxel::MAXIMUM_FAS));
		return false;
	}

	//Loop through filePaths, loaded each variable flip angle images
	for (int i = 0; i < nVFA; i++)
	{
		if (!loadFAImage(FApaths[i], i + 1))
			return false;

	}
	FAPaths_ = FApaths;
	return true;
}

MDM_API bool mdm_FileManager::loadT1Image(const std::string &T1path)
{
	mdm_Image3D T1_map = mdm_AnalyzeFormat::readImage3D(T1path, false);

	if (!T1_map.getNvoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadT1Image: Failed to read T1 map from " + T1path + "\n");
		return false;
	}

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read T1 map from " + T1path + "\n");

	//If image successfully read, add it to the T1 mapper object
	T1Mapper_.addT1Map(T1_map);
	T1Path_ = T1path;
	return true;
}

MDM_API bool mdm_FileManager::loadS0Image(const std::string &S0path)
{
	mdm_Image3D S0_map = mdm_AnalyzeFormat::readImage3D(S0path, false);

	if (!S0_map.getNvoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadS0Image: Failed to read S0 map from " + S0path + "\n");
		return false;
	}

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read S0 map from " + S0path + "\n");

	//If image successfully read, add it to the T1 mapper object
	T1Mapper_.addS0Map(S0_map);
	S0Path_ = S0path;
	return true;
}

MDM_API bool mdm_FileManager::loadStDataMaps(const std::string &dynBasePath,
	const std::string &dynPrefix, int nDyns)
{
	bool dynFilesExist = true;
	int nDyn = 0;
	dynPaths_.clear();

	//Set flags for missing data and reaching max images based on whether
	//nDyns was set or not
	bool errorIfMissing, warnIfMax;

	if (nDyns <= 0)
	{
		//If nDyns not set, keep reading images until we hit memory max
		//warn if we hit max, but don't error when we run out of images
		nDyns = MAX_IMAGES;
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
					+ std::to_string(MAX_IMAGES));
			break;
		}

		nDyn++;

		/* This sets various globals (see function header comment) */
		std::string dynPath;
		makeSequenceFilename(dynBasePath, dynPrefix, nDyn, dynPath);

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
			if (!img.getNvoxels())
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
			dynPaths_.push_back(dynPath);

			//For first image, try initialising errorImage, if that's already been
			//set it will just return true
			if (nDyn == 1)
				errorTracker_.initErrorImage(img);
		}
	}

	//Set the times in the AIF from the dynamic times
  AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());

	return true;
}

MDM_API bool mdm_FileManager::loadCtDataMaps(const std::string &catBasePath,
	const std::string &catPrefix, int nDyns)
{
	bool catFilesExist = true;
	int nCat = 0;
	catPaths_.clear();

	//Set flags for missing data and reaching max images based on whether
	//nDyns was set or not
	bool errorIfMissing, warnIfMax;

	if (nDyns <= 0)
	{
		//If nDyns not set, keep reading images until we hit memory max
		//warn if we hit max, but don't error when we run out of images
		nDyns = MAX_IMAGES;
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
					+ std::to_string(MAX_IMAGES));
			break;
		}
		nCat++;

		/* This sets various globals (see function header comment) */
		std::string catPath;
		makeSequenceFilename(catBasePath, catPrefix, nCat, catPath);

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
			if (!img.getNvoxels())
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

			catPaths_.push_back(catPath);

			//For first image, try initialising errorImage, if that's already been
			//set it will just return true
			if (nCat == 1)
				errorTracker_.initErrorImage(img);
		}
	}

  //Set the times in the AIF from the dynamic times
  AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());

	return true;
}

bool mdm_FileManager::loadFAImage(const std::string& filePath, int nVFA)
{
	mdm_Image3D FA_img = mdm_AnalyzeFormat::readImage3D(filePath, true);

	if (!FA_img.getNvoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::loadFAImage: Failed to read VFA " + 
			std::to_string(nVFA) + " file from " + filePath + "\n");
		return false;
	}

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read VFA " + std::to_string(nVFA) + " from " + filePath + "\n");

	//If image successfully read, add it to the T1 mapper object
	T1Mapper_.addFlipAngleImage(FA_img);

	//For first image, try initialising errorImage, if that's already been
	//set it will just return true
	if (nVFA == 1)
		errorTracker_.initErrorImage(FA_img);

	return true;
}

bool mdm_FileManager::writeOutputMap(const std::string &mapName, const std::string &outputDir, bool writeXtr/* = true*/)
{
	const mdm_Image3D &img = volumeAnalysis_.modelMap(mapName);
  if (img.getNvoxels())
    return writeOutputMap(mapName, img, outputDir, writeXtr);
  else
    return true;
}

bool mdm_FileManager::writeOutputMap(const std::string &mapName, const mdm_Image3D &img, const std::string &outputDir, bool writeXtr/* = true*/)
{
	std::string saveName = outputDir + "/" + mapName;

	mdm_AnalyzeFormat::XTR_type xtr = (writeXtr ? mdm_AnalyzeFormat::XTR_type::NEW_XTR : mdm_AnalyzeFormat::XTR_type::NO_XTR);

	if (!mdm_AnalyzeFormat::writeImage3D(saveName, img,
		mdm_AnalyzeFormat::DT_FLOAT, xtr, sparseWrite_))
	{
		// write msg to errLog (failed to write image to file)
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_FileManager::writeOutputMap: Failed to write " + mapName + "\n");
		return false;
	}
	return true;
}

void mdm_FileManager::makeSequenceFilename(const std::string &path, const std::string &prefix,
	const int fileNumber, std::string &filePath)
{
	std::stringstream ss;
	ss << path << "/" << prefix << fileNumber << ".img";
	filePath = ss.str();
}

