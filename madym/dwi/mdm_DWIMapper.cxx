/**
*  @file    mdm_DWIMapper.cxx
*  @brief   implementation of mdm_DWIMapper class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DWIMapper.h"

#include <cassert>
#include <chrono>  // chrono::system_clock

#include <madym/mdm_ErrorTracker.h>
#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>
#include <boost/format.hpp>

//
MDM_API mdm_DWIMapper::mdm_DWIMapper(mdm_ErrorTracker &errorTracker, mdm_Image3D &ROI)
	:inputImages_(0),
	errorTracker_(errorTracker),
  ROI_(ROI),
	model_(mdm_DWImodelGenerator::DWImodels::UNDEFINED)
{}

//
MDM_API mdm_DWIMapper::~mdm_DWIMapper()
{}

//
MDM_API void mdm_DWIMapper::reset()
{
  inputImages_.clear();
	for (auto &map : modelMaps_)
		map.reset(); 
  
}

//
MDM_API void mdm_DWIMapper::addInputImage(mdm_Image3D img)
{
  errorTracker_.checkOrSetDimension(img, "DWI input");
	inputImages_.push_back(img);
	auto msg = boost::format(
		"Acquisition parameters for DWI mapping input image %1% set from %2%:\n"
		"    B = %3% ms\n")
		% inputImages_.size()
		% img.info().xtrSource 
		% img.info().B.value();
	mdm_ProgramLogger::logProgramMessage(msg.str());
}

//
MDM_API void  mdm_DWIMapper::mapDWI(mdm_DWImodelGenerator::DWImodels method)
{
	//
	auto nSignals = inputImages_.size();

	//Instantiate DWI fitter object of required method type
	auto DWIFitter = mdm_DWImodelGenerator::createFitter(method, inputImages_, BvalsThresh_);
	auto nParams = DWIFitter->nParams();

	//Initialise maps
	modelMaps_.resize(nParams);
	paramNames_ = DWIFitter->paramNames();

	for (auto& map : modelMaps_)
	{
		map.copy(inputImages_[0]);
		map.setType(mdm_Image3D::ImageType::TYPE_ADCMAP);
	}
	SSR_.copy(inputImages_[0]);
	SSR_.setType(mdm_Image3D::ImageType::TYPE_DWI);

  bool useROI = (bool)ROI_;

	auto numVoxels = inputImages_[0].numVoxels();
	int numFitted = 0;
	int numErrors = 0;
	auto fit_start = std::chrono::system_clock::now();
	for (size_t voxelIndex = 0, n = numVoxels; voxelIndex < n; voxelIndex++)
	{
		if (useROI && !ROI_.voxel(voxelIndex))
			continue;

		//Get signals at this voxel
		std::vector<double> signal(nSignals);
		for (size_t i_f = 0; i_f < nSignals; i_f++)
			signal[i_f] = inputImages_[i_f].voxel(voxelIndex);

        
		//Compute T1 and M0
		std::vector<double> params;
		double ssr;
		DWIFitter->setSignals(signal);
		auto errCode = DWIFitter->fitModel(params, ssr);

		//Check for errors
		if (errCode != mdm_ErrorTracker::OK)
		{
			errorTracker_.updateVoxel(voxelIndex, errCode);
			numErrors++;
		}

		//Fill the image maps.
		for (size_t i_p = 0; i_p < nParams; i_p++)
			modelMaps_[i_p].setVoxel(voxelIndex, params[i_p]);
		SSR_.setVoxel(voxelIndex, ssr);
		numFitted++;
	}

	// Get end time and log results
	auto fit_end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = fit_end - fit_start;

	mdm_ProgramLogger::logProgramMessage("Fitted " +
    std::to_string(numFitted) + " voxels in " + std::to_string(elapsed_seconds.count()) + "s");
  if (numErrors)
    mdm_ProgramLogger::logProgramWarning(__func__, 
      std::to_string(numErrors) + " voxels returned fit errors");
}

//
MDM_API void mdm_DWIMapper::mapDWI()
{
	//Just call the overload mapT1 with the method- member variable
	mapDWI(model_);
}

//
MDM_API const std::vector<mdm_Image3D>& mdm_DWIMapper::inputImages() const
{
	return inputImages_;
}

//
MDM_API const mdm_Image3D& mdm_DWIMapper::inputImage(size_t i) const
{
  if (i >= inputImages_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access input image %1% when there are %2% input images")
      % i % inputImages_.size());

  return inputImages_[i];
}

//
MDM_API std::vector<std::string> mdm_DWIMapper::paramNames() const
{
	return paramNames_;
}

//
MDM_API const mdm_Image3D& mdm_DWIMapper::model_map(const std::string& map_name) const
{
	for (size_t i_p = 0; i_p < paramNames_.size(); i_p++)
		if (paramNames_[i_p] == map_name)
			return modelMaps_[i_p];

	throw mdm_exception(__func__, boost::format(
		"Map name %1% not found in DWI model paramter names")
		% map_name % inputImages_.size());
}

//
MDM_API double mdm_DWIMapper::model_map(const std::string& map_name, size_t voxel) const
{
	for (size_t i_p = 0; i_p < paramNames_.size(); i_p++)
		if (paramNames_[i_p] == map_name)
			return modelMaps_[i_p].voxel(voxel);

	throw mdm_exception(__func__, boost::format(
		"Map name %1% not found in DWI model paramter names")
		% map_name % inputImages_.size());
}

//
MDM_API mdm_DWImodelGenerator::DWImodels  mdm_DWIMapper::model() const
{
	return model_;
}

//
MDM_API void  mdm_DWIMapper::setModel(mdm_DWImodelGenerator::DWImodels model)
{
	model_ = model;
}

MDM_API void  mdm_DWIMapper::setBvalsThresh(const std::vector<double>& BvalsThresh)
{
	BvalsThresh_ = BvalsThresh;
}

//******************************************************************
//Private methods
//******************************************************************