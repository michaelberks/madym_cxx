/**
*  @file    mdm_RunTools_madym_AIF.cxx
*  @brief   Implementation of mdm_RunTools_madym_AIF class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_AIF.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

#include <algorithm>
#include <numeric>

#include <boost/format.hpp>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_AIF::mdm_RunTools_madym_AIF()
{
}


MDM_API mdm_RunTools_madym_AIF::~mdm_RunTools_madym_AIF()
{
}

//
MDM_API void mdm_RunTools_madym_AIF::run()
{
  //Check required inputs
  checkRequiredInputs();

  //Set curent working dir
  set_up_cwd();

  //Set parameters from user inputs
  setFileManagerParams();

  setVolumeAnalysisParams();

  //Create output folder/check overwrite
  set_up_output_folder();

  //Set up logging trail
  set_up_logging();

  //Load existing error image if it exists
  loadErrorTracker();

  //Load ROI
  loadROI();

  //Load dynamic volumes
  if (options_.inputCt())
    loadCt();
  else
    loadSt();

  //set dyn times in AIF
  AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());

  if (!options_.T1Name().empty())
    //supplied an existing T1 map
    loadT1();

  else
    //mapping T1 from input signal volumes
    mapT1();

  //See if B1 correction map to load
  loadB1(options_.B1Correction());

  //Load AIF map
  if (!options_.aifMap().empty())
  {
    //Load AIF map
    fileManager_.loadAIFmap(fs::absolute(options_.aifMap()).string());

    //Try and set these base values in the AIF
    saveAIF("AIFmap");

  }
  else
    //Otherwise, try and auto-fit the AIF
    computeAutoAIF();

  //Save the error tracker
  fileManager_.saveErrorTracker(outputPath_.string(),
    volumeAnalysis_.MAP_NAME_ERROR_TRACKER);

  //Save an ROI if used
  fileManager_.saveROI(outputPath_.string(),
    volumeAnalysis_.MAP_NAME_ROI);

  //Reset the volume analysis
  volumeAnalysis_.reset();
}

//
MDM_API int mdm_RunTools_madym_AIF::parseInputs(int argc, const char *argv[])
{
  po::options_description cmdline_options("madym_AIF options");
  po::options_description config_options("madym_AIF config options");

  options_parser_.add_option(cmdline_options, options_.configFile);
  options_parser_.add_option(cmdline_options, options_.dataDir);;

		//DCE input options_
  options_parser_.add_option(config_options, options_.inputCt);
  options_parser_.add_option(config_options, options_.dynName);
  options_parser_.add_option(config_options, options_.dynDir);
  options_parser_.add_option(config_options, options_.sequenceFormat);
  options_parser_.add_option(config_options, options_.nDyns);
  options_parser_.add_option(config_options, options_.injectionImage);
  options_parser_.add_option(config_options, options_.roiName);
  options_parser_.add_option(config_options, options_.errorTrackerName);

  //T1 mapping options
  options_parser_.add_option(config_options, options_.T1method);
  options_parser_.add_option(config_options, options_.T1inputNames);
  options_parser_.add_option(config_options, options_.T1noiseThresh);
  options_parser_.add_option(config_options, options_.B1Scaling);
  options_parser_.add_option(config_options, options_.B1Name);

	//Signal to concentration options_
  options_parser_.add_option(config_options, options_.M0Ratio);
  options_parser_.add_option(config_options, options_.T1Name);
  options_parser_.add_option(config_options, options_.M0Name);
  options_parser_.add_option(config_options, options_.r1Const);
  options_parser_.add_option(config_options, options_.B1Correction);

  //AIF auto options
  options_parser_.add_option(config_options, options_.aifMap);
  options_parser_.add_option(config_options, options_.aifSlices);
  options_parser_.add_option(config_options, options_.aifXrange);
  options_parser_.add_option(config_options, options_.aifYrange);
  options_parser_.add_option(config_options, options_.minT1Blood);
  options_parser_.add_option(config_options, options_.peakTime);
  options_parser_.add_option(config_options, options_.prebolusNoise);
  options_parser_.add_option(config_options, options_.prebolusMinImages);
  options_parser_.add_option(config_options, options_.selectPct);

  //General output options_
  options_parser_.add_option(config_options, options_.outputRoot);
  options_parser_.add_option(config_options, options_.outputDir);
  options_parser_.add_option(config_options, options_.overwrite);

  //Image format options
  options_parser_.add_option(config_options, options_.imageReadFormat);
  options_parser_.add_option(config_options, options_.imageWriteFormat);

  //Logging options_
  options_parser_.add_option(config_options, options_.noLog);
  options_parser_.add_option(config_options, options_.noAudit);
  options_parser_.add_option(config_options, options_.quiet);
  options_parser_.add_option(config_options, options_.programLogName);
  options_parser_.add_option(config_options, options_.outputConfigFileName);
  options_parser_.add_option(config_options, options_.auditLogBaseName);
  options_parser_.add_option(config_options, options_.auditLogDir);

  return options_parser_.parseInputs(
    cmdline_options,
    config_options,
    options_.configFile(),
    who(),
    argc, argv);
}

MDM_API std::string mdm_RunTools_madym_AIF::who() const
{
	return "madym_AIF";
}
//*******************************************************************************
// Private:
//*******************************************************************************

//
void mdm_RunTools_madym_AIF::checkRequiredInputs()
{
  if (!options_.T1Name().empty() && options_.T1Name().at(0) == '-')
    throw mdm_exception(__func__, "No value associated with T1 map name from command-line");

  if (!options_.M0Name().empty() && options_.M0Name().at(0) == '-')
    throw mdm_exception(__func__, "No value associated with M0 map name from command-line");

  if (!options_.dynName().empty() && options_.dynName().at(0) == '-')
    throw mdm_exception(__func__, "No value associated with dynamic series file name from command-line");

  if (options_.aifSlices().empty() && options_.aifMap().empty())
    throw mdm_exception(__func__, "You must specify either --aif_slices or --aif_map");
}

//
void mdm_RunTools_madym_AIF::setVolumeAnalysisParams()
{
  volumeAnalysis_.setComputeCt(!options_.inputCt());
  volumeAnalysis_.setPrebolusImage(options_.injectionImage());
  volumeAnalysis_.setR1Const(options_.r1Const());
}

//
void mdm_RunTools_madym_AIF::computeAutoAIF()
{
  //We'll want to know which voxels were identified as suitable for AIF estimation
  mdm_Image3D AIFmap;
  AIFmap.copy(volumeAnalysis_.T1Mapper().T1());
  AIFmap.setType(mdm_Image3D::ImageType::TYPE_AIFVOXELMAP);

  //Get candidate voxels, saving their max signal, and associated timepoint
  std::vector<size_t> allCandidateVoxels;
  std::vector<double> allCandidateMaxSignals;
  processSlices(options_.aifSlices()[0], AIFmap, allCandidateVoxels, allCandidateMaxSignals);

  if (allCandidateVoxels.empty())
  {
    mdm_ProgramLogger::logProgramWarning(__func__,
      "No suitable voxels found to define AIF across all slices");
    return;
  }
  mdm_ProgramLogger::logProgramMessage(
    (boost::format("Found %1% candidate voxels across all slices")
      % allCandidateVoxels.size()).str());

  //Select from candidates
  selectVoxelsFromCandidates(AIFmap, allCandidateVoxels, allCandidateMaxSignals);

  //Use volume analysis to compute mean of voxels with flag set to 2, and set these
  //values in the AIF
  auto sliceName = (boost::format("slice_%1%-%2%_Auto_AIF")
    % options_.aifSlices().front()
    % options_.aifSlices().back()).str();

  saveAIF(sliceName);
}

//
void mdm_RunTools_madym_AIF::computeAutoAIFSlice(
  const size_t slice,
  mdm_Image3D &AIFmap,
  const std::vector<size_t> &xRange,
  const std::vector<size_t> &yRange,
  std::vector<size_t> &candidateVoxels,
  std::vector<double> &candidateMaxSignals)
{
  mdm_Image3D AIFSliceMap;
  AIFSliceMap.copy(volumeAnalysis_.T1Mapper().T1());
  AIFSliceMap.setType(mdm_Image3D::ImageType::TYPE_AIFVOXELMAP);

  //Get candidate voxels for this slice
  getSliceCandidateVoxels(
    slice,
    xRange,
    yRange,
    AIFmap,
    AIFSliceMap,
    candidateVoxels,
    candidateMaxSignals);

  if (candidateVoxels.empty())
  {
    mdm_ProgramLogger::logProgramWarning(__func__,
      (boost::format("warning: no suitable voxels found to define AIF for slice %1%") 
        % slice).str() );
    return;
  }
  mdm_ProgramLogger::logProgramMessage(
    (boost::format("Found %1% candidate voxels in slice %2%")
      % candidateVoxels.size() % slice).str());

  //Select from candidates
  selectVoxelsFromCandidates(AIFSliceMap, candidateVoxels, candidateMaxSignals);

  //Use volume analysis to compute mean of voxels with flag set to 2, and set these
  //values in the AIF
  auto sliceName = (boost::format("slice_%1%_Auto_AIF") % slice).str();
  saveAIF(sliceName);
}

//
void mdm_RunTools_madym_AIF::processSlices(
  const size_t slice,
  mdm_Image3D &AIFmap,
  std::vector<size_t> &candidateVoxels,
  std::vector<double> &candidateMaxSignals)
{
  candidateVoxels.clear();
  candidateMaxSignals.clear();

  //Scan T1 map for pixels that might be blood
  size_t nX, nY, nZ;
  AIFmap.getDimensions(nX, nY, nZ);

  //Get ranges for X and Y from inputs if they're set
  std::vector<size_t> xRange, yRange;
  if (options_.aifXrange().empty())
  {
    xRange.resize(nX);
    std::iota(xRange.begin(), xRange.end(), 0);
  }
  else
    for (const auto x: options_.aifXrange())
      xRange.push_back(size_t(x));

  if (options_.aifYrange().empty())
  {
    yRange.resize(nY);
    std::iota(yRange.begin(), yRange.end(), 0);
  }
  else
    for (const auto y : options_.aifYrange())
      yRange.push_back(size_t(y));

  for (const auto slice : options_.aifSlices())
  {
    std::vector<size_t> sliceCandidateVoxels;
    std::vector<double> sliceCandidateMaxSignals;
    computeAutoAIFSlice(
      slice,
      AIFmap,
      xRange,
      yRange,
      sliceCandidateVoxels,
      sliceCandidateMaxSignals);

    //Insert the slice candidates in to the main list
    candidateVoxels.insert(candidateVoxels.end(),
      sliceCandidateVoxels.begin(), sliceCandidateVoxels.end());

    candidateMaxSignals.insert(candidateMaxSignals.end(),
      sliceCandidateMaxSignals.begin(), sliceCandidateMaxSignals.end());
  }
}

// 
void mdm_RunTools_madym_AIF::getSliceCandidateVoxels(
  const size_t slice,
  const std::vector<size_t> &xRange,
  const std::vector<size_t> &yRange,
  mdm_Image3D &AIFmap,
  mdm_Image3D &AIFmapSlice,
  std::vector<size_t> &candidateVoxels,
  std::vector<double> &candidateMaxSignals)
{
  const std::vector<mdm_Image3D> &dynImages = options_.inputCt() ?
    volumeAnalysis_.CtDataMaps() : volumeAnalysis_.StDataMaps();

  const auto &T1 = volumeAnalysis_.T1Mapper().T1();
  const auto &errorMap = volumeAnalysis_.errorTracker().errorImage();

  bool useROI = (bool)volumeAnalysis_.ROI();

  for (const auto ix : xRange)
  {
    for (const auto iy : yRange)
    {
      auto voxelIndex = T1.sub2ind(ix, iy, slice);

      //Skip if using ROI and voxel not in ROI
      if (useROI && !volumeAnalysis_.ROI().voxel(voxelIndex))
        continue;

      //Also skip if bad value set in error tracker
      if (errorMap.voxel(voxelIndex) != mdm_ErrorTracker::OK)
        continue;

      // assume pre-contrast T1 of blood is around 1500 ms
      if (T1.voxel(voxelIndex) > options_.minT1Blood())
      {
        // Check to see if time course is a valid candidate, if so, save its
        //max signal and voxel index
        double maxSignal;
        if (validCandidate(dynImages, AIFmap, AIFmapSlice, voxelIndex, maxSignal))
        {
          //Flag this voxel in map as valid and above threshold
          candidateMaxSignals.push_back(maxSignal);
          candidateVoxels.push_back(voxelIndex);
        }
      }
    }
  }
}

//
void mdm_RunTools_madym_AIF::selectVoxelsFromCandidates(
  mdm_Image3D &AIFmap,
  const std::vector<size_t> &candidateVoxels,
  const std::vector<double> &candidateMaxSignals)
{
  // sort max conc array and return sorted indices
  auto nMax = candidateMaxSignals.size();
  std::vector<size_t> indices(nMax);
  std::iota(indices.begin(), indices.end(), 0); //returns 0, 1, 2,... etc
  std::sort(indices.begin(), indices.end(),
    [&candidateMaxSignals](size_t i1, size_t i2) {return candidateMaxSignals[i1] > candidateMaxSignals[i2]; });

  // Keep the indices of the top 5%
  size_t threshIdx = (size_t)(options_.selectPct() * (double)(nMax)/100.0);

  //Get all voxel indexes with concentration above threshold and save in AIF map
  for (size_t idx = 0; idx < threshIdx; idx++)
    AIFmap.setVoxel(candidateVoxels[indices[idx]], mdm_AIF::AIFmapVoxel::SELECTED);

  mdm_ProgramLogger::logProgramMessage(
    (boost::format("Selected %1% voxels to use in AIF")
      % threshIdx).str());
    
  volumeAnalysis_.setAIFmap(AIFmap);
}

//
void mdm_RunTools_madym_AIF::saveAIF(const std::string &sliceName)
{
  std::vector<double> baseAIF(volumeAnalysis_.AIFfromMap());
  AIF_.setBaseAIF(baseAIF);

  //Write the AIF and save the AIF map
  AIF_.writeAIF(outputPath_.string() + "/" + sliceName + ".txt");

  //Write out AIF map
  fileManager_.saveAIFmap(outputPath_.string(), sliceName);

  mdm_ProgramLogger::logProgramMessage(
    (boost::format("Saved AIF and voxel map to %1%.hdr/txt")
      % sliceName).str());
}

bool mdm_RunTools_madym_AIF::validCandidate(
  const std::vector<mdm_Image3D> &dynImages,
  mdm_Image3D &AIFmap,
  mdm_Image3D &AIFmapSlice,
  size_t voxelIndex, double &maxSignal)
{
  //Aim of this function is to:
  // Check if voxel valid:
  // - Has max signal at time t, where prebolus < t < prebolus + 1 minute;
  // - Has no negative values after t
  // - Has max signal distinguishable from noise
  auto nTimes = dynImages.size();

  std::vector<double> signalData(nTimes);
  for (size_t it = 0; it < nTimes; it++)
    signalData[it] = dynImages[it].voxel(voxelIndex);

  //Convenient to alias these
  const auto &prebolusImg = options_.injectionImage();
  const auto &times = AIF_.AIFTimes();
  const auto &bolusTime = times[prebolusImg];

  // Find image that defines onset - Algorithm for following from 'MRIW'
  // - see Parker et al, JMRI 7, 564, 1997 and Parker et al, Radiographics 18, 497, 1998.
  //
  // Get max and min signals in time series
  double minSignal;
  size_t maxImg;
  getMinMaxSignal(signalData, minSignal, maxSignal, maxImg);

  //First check if the max signal arrives in peak window post injection
  //If it doesn't return
  if (maxImg <= prebolusImg)
  {
    AIFmapSlice.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::PEAK_TOO_EARLY);
    AIFmap.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::PEAK_TOO_EARLY);
    return false;
  }
  else if (times[maxImg] - bolusTime > options_.peakTime())
  {
    AIFmapSlice.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::PEAK_TOO_LATE);
    AIFmap.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::PEAK_TOO_LATE);
    return false;
  }
  

  // Find arrival image as image which first exceed 10% 
  // from min to max signal after bolus arrival
  size_t arrivalImg = 0;
  double lowerThreshold = minSignal + 0.1 * (maxSignal - minSignal);
  for (size_t it = prebolusImg; it < maxImg; it++)
  {
    if (!arrivalImg && signalData[it] > lowerThreshold)
      arrivalImg = it;

    // if it dips down again it must be noise...
    if (signalData[it] < lowerThreshold && arrivalImg)
    {
      AIFmapSlice.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::DOUBLE_DIP);
      AIFmap.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::DOUBLE_DIP);
      return false;
    }
      
  }

  //Finally, work out standard deviation in pre-arrival period
  //and check if max signal is distinguishable from noise
  if (maxSignal < prebolusNoiseThresh(signalData, arrivalImg))
  {
    AIFmapSlice.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::BELOW_NOISE_THRESH);
    AIFmap.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::BELOW_NOISE_THRESH);
    return false;
  }
    
  AIFmapSlice.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::CANDIDATE);
  AIFmap.setVoxel(voxelIndex, mdm_AIF::AIFmapVoxel::CANDIDATE);
  return true;
}

//
void mdm_RunTools_madym_AIF::getMinMaxSignal(const std::vector<double> &signalData,
  double &minSignal, double&maxSignal, size_t &maxImg)
{
  maxSignal = signalData[0];
  minSignal = maxSignal;
  maxImg = 0;
  for (size_t it = 1; it < signalData.size(); it++)
  {
    if (signalData[it] > maxSignal)
    {
      maxSignal = signalData[it];
      maxImg = it;
    }
    if (signalData[it] < minSignal)
      minSignal = signalData[it];
  }
}

//
double mdm_RunTools_madym_AIF::prebolusNoiseThresh(
  const std::vector<double> &signalData,
  const size_t arrivalImg)
{
  //Compute mean and standard deviation in prebolu signal
  double sum = 0.0;
  double sumsq = 0.0;
  for (size_t it = 0; it <= arrivalImg; it++)
  {
    sum += signalData[it];
    sumsq += signalData[it] * signalData[it];
  }

  double nT = double(arrivalImg) + 1;
  double mean = sum / nT;
  double std = nT >= options_.prebolusMinImages() ?
    sqrt((sumsq - sum * sum / nT) / (nT - 1)) : options_.prebolusNoise();

  //Set threshold as 3 standard deviations from the mean
  return mean + 3*std;
}