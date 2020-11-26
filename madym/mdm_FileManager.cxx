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

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

const int mdm_FileManager::MAX_DYN_IMAGES = 1024;

MDM_API mdm_FileManager::mdm_FileManager(mdm_VolumeAnalysis &volumeAnalysis)
	: 
	volumeAnalysis_(volumeAnalysis),
	writeCtDataMaps_(false),
  writeCtModelMaps_(false),
	sparseWrite_(false)
{
}

MDM_API mdm_FileManager::~mdm_FileManager()
{}

//
MDM_API void mdm_FileManager::loadROI(const std::string &path)
{
	// Read in ROI image volume
  try {
    mdm_Image3D ROI = mdm_AnalyzeFormat::readImage3D(path, false);
    ROI.setType(mdm_Image3D::ImageType::TYPE_ROI);
    volumeAnalysis_.setROI(ROI);
  }
  catch (mdm_exception &e)
  {
    e.append("Error reading ROI");
    throw;
  }

	mdm_ProgramLogger::logProgramMessage(
		"ROI loaded from " + path);

}

//
MDM_API void mdm_FileManager::saveROI(const std::string &outputDir, const std::string &name)
{
 if (volumeAnalysis_.ROI().numVoxels())
    saveOutputMap(name, volumeAnalysis_.ROI(), outputDir, false, mdm_AnalyzeFormat::DT_UNSIGNED_CHAR);
}

//
MDM_API void mdm_FileManager::loadAIFmap(const std::string &path)
{
  try {
    // Read in AIF map image volume
    mdm_Image3D AIFmap = mdm_AnalyzeFormat::readImage3D(path, false);
    AIFmap.setType(mdm_Image3D::ImageType::TYPE_ROI);
    volumeAnalysis_.setAIFmap(AIFmap);
  }
  catch (mdm_exception &e)
  {
    e.append("Error reading AIF map");
    throw;
  }
  
  mdm_ProgramLogger::logProgramMessage(
    "AIF map loaded loaded from " + path);
}

//
MDM_API void mdm_FileManager::saveAIFmap(const std::string &outputDir, const std::string &name)
{
  if (volumeAnalysis_.AIFmap().numVoxels())
    saveOutputMap(name, volumeAnalysis_.AIFmap(), outputDir, false);
}

//
MDM_API void mdm_FileManager::loadParameterMaps(const std::string &paramDir)
{
  //Write model parameters maps
  std::vector<std::string> paramNames = volumeAnalysis_.paramNames();
  for (int i = 0; i < paramNames.size(); i++)
  {
    std::string paramName = paramDir + "/" + paramNames[i];

    try {
      mdm_Image3D paramMap = mdm_AnalyzeFormat::readImage3D(paramName, false);
      paramMap.setType(mdm_Image3D::ImageType::TYPE_KINETICMAP);
      volumeAnalysis_.setDCEMap(paramNames[i], paramMap);
    }
    catch (mdm_exception &e)
    {
      e.append("Error reading parameter map " + paramName);
      throw;
    }
  }
	mdm_ProgramLogger::logProgramMessage(
		"Successfully read param maps from " + paramDir);

}

MDM_API void mdm_FileManager::saveOutputMaps(const std::string &outputDir)
{
  //Write out ROI (if used)
  saveROI(outputDir, volumeAnalysis_.MAP_NAME_ROI);

  //Write out error tracker
  saveErrorTracker(outputDir, volumeAnalysis_.MAP_NAME_ERROR_TRACKER);

  //Write out T1 and M0 maps (if M0 map used)
  if (volumeAnalysis_.T1Mapper().T1().numVoxels())
    saveOutputMap(volumeAnalysis_.MAP_NAME_T1, 
      volumeAnalysis_.T1Mapper().T1(), outputDir, true);
		
	if (volumeAnalysis_.T1Mapper().M0().numVoxels())
		saveOutputMap(volumeAnalysis_.MAP_NAME_M0, 
      volumeAnalysis_.T1Mapper().M0(), outputDir, true);

  //Write model parameters maps
  if (!volumeAnalysis_.modelType().empty())
  {
    for (const auto paramName : volumeAnalysis_.paramNames())
      saveOutputMap(paramName, outputDir, false);

    //Write IAUC maps
    for (const auto t : volumeAnalysis_.IAUCtimes())
    {
      const std::string iaucName = volumeAnalysis_.MAP_NAME_IAUC + std::to_string(int(t));
      saveOutputMap(iaucName, outputDir, false);
    }
    saveOutputMap(volumeAnalysis_.MAP_NAME_ENHANCING, outputDir, false);
  }
	
	//Write error and enhancing voxels map
  saveModelResiduals(outputDir);

	//Write output stats
	saveSummaryStats(outputDir);

	//If used, write ROI
	if (volumeAnalysis_.ROI().numVoxels()) 
    saveOutputMap(volumeAnalysis_.MAP_NAME_ROI, volumeAnalysis_.ROI(), outputDir, false);

	if (writeCtDataMaps_)
	{
		const std::string &ctSigPrefix = volumeAnalysis_.MAP_NAME_CT_SIG;
		for (int i = 0; i < volumeAnalysis_.numDynamics(); i++)
		{
			std::string ctName = ctSigPrefix + std::to_string(i + 1);
			saveOutputMap(ctName, volumeAnalysis_.CtDataMap(i), outputDir, true);
		}
	}
  if (writeCtModelMaps_)
  {
    const std::string &ctModPrefix = volumeAnalysis_.MAP_NAME_CT_MOD;
    for (int i = 0; i < volumeAnalysis_.numDynamics(); i++)
    {
      std::string cmodName = ctModPrefix + std::to_string(i + 1);
      saveOutputMap(cmodName, volumeAnalysis_.CtModelMap(i), outputDir, false);
    }
  }
}

//
MDM_API void mdm_FileManager::saveModelResiduals(const std::string &outputDir)
{
  return saveOutputMap(volumeAnalysis_.MAP_NAME_RESDIUALS, outputDir, false);
}

//
MDM_API void mdm_FileManager::saveSummaryStats(const std::string &outputDir)
{
	//Create a new stats object
	mdm_ParamSummaryStats stats;

	//Do stats for whole ROI
	const auto &roi = volumeAnalysis_.ROI();
	if (roi.numVoxels())
		stats.setROI(roi);

	saveMapsSummaryStats(outputDir + "/" + volumeAnalysis_.MAP_NAME_ROI, stats);

	//Repeat for the enhancing map
	const auto &enh = volumeAnalysis_.DCEMap(volumeAnalysis_.MAP_NAME_ENHANCING);
	if (enh.numVoxels())
	{
		stats.setROI(enh);
		saveMapsSummaryStats(outputDir + "/" + volumeAnalysis_.MAP_NAME_ENHANCING, stats);
	}
}

//
MDM_API void mdm_FileManager::loadErrorTracker(const std::string &errorPath)
{
  // Read in ROI image volume
  try {
    mdm_Image3D errorMap = mdm_AnalyzeFormat::readImage3D(errorPath, false);
    errorMap.setType(mdm_Image3D::ImageType::TYPE_ERRORMAP);

    volumeAnalysis_.errorTracker().setErrorImage(errorMap);
  }
  catch (mdm_exception &e)
  {
    e.append("Error reading error tracker map");
    throw;
  }

  mdm_ProgramLogger::logProgramMessage(
    "Error tracker map loaded from " + errorPath);
}

//
MDM_API void mdm_FileManager::saveErrorTracker(const std::string &outputDir, const std::string &name)
{
  saveOutputMap(name, volumeAnalysis_.errorTracker().errorImage(), outputDir, false,
    mdm_AnalyzeFormat::DT_SIGNED_INT);
}

/*Does what is says on the tin*/
MDM_API void mdm_FileManager::loadT1MappingInputImages(const std::vector<std::string> &T1InputPaths)
{
	//Check we haven't been given too many or too few images
	auto nNumImgs = T1InputPaths.size();

	//Loop through filePaths, loaded each variable flip angle images
	for (int i = 0; i < nNumImgs; i++)
	  loadT1InputImage(T1InputPaths[i], i + 1);

}

//
MDM_API void mdm_FileManager::loadT1Map(const std::string &T1path)
{
  try {
    mdm_Image3D T1_map = mdm_AnalyzeFormat::readImage3D(T1path, false);
    T1_map.setType(mdm_Image3D::ImageType::TYPE_T1BASELINE);

    //If image successfully read, add it to the T1 mapper object
    volumeAnalysis_.T1Mapper().setT1(T1_map);
  }
  catch (mdm_exception &e)
  {
    e.append("Error loading T1 map");
    throw;
  }

  mdm_ProgramLogger::logProgramMessage(
    "Successfully read T1 map from " + T1path);
}

//
MDM_API void mdm_FileManager::loadM0Map(const std::string &M0path)
{
  try {
    mdm_Image3D M0_map = mdm_AnalyzeFormat::readImage3D(M0path, false);
    M0_map.setType(mdm_Image3D::ImageType::TYPE_M0MAP);
    
    //If image successfully read, add it to the T1 mapper object
    volumeAnalysis_.T1Mapper().setM0(M0_map);
  }
  catch (mdm_exception &e)
  {
    e.append("Error loading M0 map");
    throw;
  }

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read M0 map from " + M0path);
}

//
MDM_API void mdm_FileManager::loadStDataMaps(const std::string &dynBasePath,
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
				mdm_ProgramLogger::logProgramWarning(__func__,
					"Reached maximum number of images " + std::to_string(MAX_DYN_IMAGES));
			break;
		}

		nDyn++;

		std::string dynPath;
		makeSequenceFilename(dynBasePath, dynPrefix, nDyn, dynPath, indexPattern);

		if (!boost::filesystem::exists(dynPath))
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
      try {
        mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(dynPath, true);
        img.setType(mdm_Image3D::ImageType::TYPE_T1DYNAMIC);

        //If successfully loaded, add image to the volume analysis
        volumeAnalysis_.addStDataMap(img);
      }
      catch (mdm_exception &e)
      {
        e.append("Failed to read dynamic image " +
          std::to_string(nDyn) + " from " + dynPath);
        throw;
      }

			mdm_ProgramLogger::logProgramMessage(
				"Successfully read dynamic image " +
				std::to_string(nDyn) + " from " + dynPath);			
		}
	}
}

//
MDM_API void mdm_FileManager::loadCtDataMaps(const std::string &CtBasePath,
	const std::string &CtPrefix, int nDyns, const std::string &indexPattern)
{
	bool CtFilesExist = true;
	int nCt = 0;

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

	while (CtFilesExist)
	{
		if (nCt == nDyns)
		{
			if (warnIfMax)
        mdm_ProgramLogger::logProgramWarning(__func__,
          "Reached maximum number of images " + std::to_string(MAX_DYN_IMAGES));

			break;
		}
		nCt++;

		/* This sets various globals (see function header comment) */
		std::string CtPath;
		makeSequenceFilename(CtBasePath, CtPrefix, nCt, CtPath, indexPattern);

		if (!boost::filesystem::exists(CtPath))
		{
			//If nDyns was set, we expect to load in that many images, so error and return false
			//if any of them don't exist
			if (errorIfMissing)
        throw mdm_exception(__func__, CtPath + " does not exist.");
			
			//However if nDyns wasn't set, this is expected behaviour - we keep loading images
			//until we don't find them - just set CtFilesExist to false so we break the loop
			CtFilesExist = false;
		}
		else
		{
      try {
        mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(CtPath, true);
        img.setType(mdm_Image3D::ImageType::TYPE_CAMAP);

        //If successfully loaded, add image to the volume analysis
        volumeAnalysis_.addCtDataMap(img);        
      }
      catch (mdm_exception &e)
      {
        e.append("Failed to read concentration image " +
          std::to_string(nCt) + " from " + CtPath);
        throw;
      }

			mdm_ProgramLogger::logProgramMessage(
				"Successfully read concentration image  " +
				std::to_string(nCt) + " from " + CtPath);		
		}
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

MDM_API void mdm_FileManager::setSparseWrite(bool b)
{
  sparseWrite_ = b;
}

//---------------------------------------------------------------------------
//Private functions
//---------------------------------------------------------------------------

void mdm_FileManager::loadT1InputImage(const std::string& filePath, int nVFA)
{
  try {
    mdm_Image3D img = mdm_AnalyzeFormat::readImage3D(filePath, true);
    img.setType(mdm_Image3D::ImageType::TYPE_T1WTSPGR);

    //If image successfully read, add it to the T1 mapper object
    volumeAnalysis_.T1Mapper().addInputImage(img);
  }
  catch (mdm_exception &e)
  {
    e.append("Error loading T1 input " + filePath);
    throw;
  }

	mdm_ProgramLogger::logProgramMessage(
		"Successfully read T1 input image " + std::to_string(nVFA) + " from " + filePath);
}

void mdm_FileManager::saveOutputMap(
  const std::string &mapName, const std::string &outputDir, bool writeXtr/* = true*/)
{
	const mdm_Image3D &img = volumeAnalysis_.DCEMap(mapName);
  if (img.numVoxels())
    saveOutputMap(mapName, img, outputDir, writeXtr);
}

void mdm_FileManager::saveOutputMap(const std::string &mapName, const mdm_Image3D &img, 
  const std::string &outputDir, bool writeXtr/* = true*/,
  const mdm_AnalyzeFormat::Data_type format /*= mdm_AnalyzeFormat::DT_FLOAT*/)
{
	std::string saveName = outputDir + "/" + mapName;

	mdm_AnalyzeFormat::XTR_type xtr = (writeXtr ? mdm_AnalyzeFormat::XTR_type::NEW_XTR : mdm_AnalyzeFormat::XTR_type::NO_XTR);


  try {
    mdm_AnalyzeFormat::writeImage3D(saveName, img,
      format, xtr, sparseWrite_);
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
	if (volumeAnalysis_.T1Mapper().T1().numVoxels())
		saveMapSummaryStats(volumeAnalysis_.MAP_NAME_T1, 
      volumeAnalysis_.T1Mapper().T1(), stats);

	if (volumeAnalysis_.T1Mapper().M0().numVoxels())
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
void mdm_FileManager::makeSequenceFilename(const std::string &path, const std::string &prefix,
	const int fileNumber, std::string &filePath, const std::string &fileNumberFormat)
{
	auto formattedFilenumber = boost::format(fileNumberFormat.c_str()) % fileNumber;
	auto imageName = boost::format("%1%%2%.img") % prefix % formattedFilenumber;
	filePath = (fs::path(path) / imageName.str()).string();
}

