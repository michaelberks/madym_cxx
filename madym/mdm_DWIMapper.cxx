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
	noiseThreshold_(0),
	method_(mdm_DWIMethodGenerator::DWIMethods::UNDEFINED)
{}

//
MDM_API mdm_DWIMapper::~mdm_DWIMapper()
{}

//
MDM_API void mdm_DWIMapper::reset()
{
  inputImages_.clear();
	for (auto &map : model_maps_)
		map.reset(); 
  
}

//
MDM_API void mdm_DWIMapper::addInputImage(mdm_Image3D img)
{
  errorTracker_.checkOrSetDimension(img, "T1 input");
	inputImages_.push_back(img);
	auto msg = boost::format(
		"Acquisition parameters for T1 mapping input image %1% set from %2%:\n"
		"    TR = %3% ms\n"
		"    FA = %4% deg (only required for VFA method)\n"
		"    TI = %5% ms (only required for inversion recovery method)")
		% inputImages_.size()
		% img.info().xtrSource 
		% img.info().TR.value() 
		% img.info().flipAngle.value()
		% img.info().TI.value();
	mdm_ProgramLogger::logProgramMessage(msg.str());
}

//
MDM_API void  mdm_DWIMapper::mapDWI(mdm_DWIMethodGenerator::DWIMethods method)
{
	//
	auto nSignals = inputImages_.size();

	//Instantiate T1 fitter object of required method type
	auto T1Fitter = mdm_DWIMethodGenerator::createFitter(method, inputImages_);

	mdm_ErrorTracker::ErrorCode errCode = mdm_ErrorTracker::OK;

	/*
	T1_.copy(inputImages_[0]);
	T1_.setType(mdm_Image3D::ImageType::TYPE_T1BASELINE);

	M0_.copy(inputImages_[0]);
	M0_.setType(mdm_Image3D::ImageType::TYPE_M0MAP);

  bool useROI = (bool)ROI_;
  bool useB1_ = (bool)B1_ && method == mdm_T1MethodGenerator::VFA_B1;

	int numFitted = 0;
	int numErrors = 0;
	auto fit_start = std::chrono::system_clock::now();
	for (size_t voxelIndex = 0, n = M0_.numVoxels(); voxelIndex < n; voxelIndex++)
	{
		if (useROI && !ROI_.voxel(voxelIndex))
			continue;

		//Get signals at this voxel
		std::vector<double> signal(nSignals);
		for (size_t i_f = 0; i_f < nSignals; i_f++)
			signal[i_f] = inputImages_[i_f].voxel(voxelIndex);

		//TODO - MB, why only check the first signal?				
		if (signal[0] > noiseThreshold_)
		{
      //If using B1 correction, add this to the inputs
      if (useB1_)
      {
        auto B1 = B1_.voxel(voxelIndex);
        if (B1 > 0)
          signal.push_back(B1);
        else
        {
          errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::B1_INVALID);
          numErrors++;
          continue;
        }
      }
        

			//Compute T1 and M0
			double T1, M0;
			T1Fitter->setInputs(signal);
			errCode = T1Fitter->fitT1(T1, M0);

			//Check for errors
			if (errCode != mdm_ErrorTracker::OK)
			{
				errorTracker_.updateVoxel(voxelIndex, errCode);
				numErrors++;
			}

			//Fill the image maps.
			T1_.setVoxel(voxelIndex, T1);
			M0_.setVoxel(voxelIndex, M0);
		}
		else
		{
			errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::VFA_THRESH_FAIL);
			numErrors++;
		}
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
	*/
}

//
MDM_API void mdm_DWIMapper::mapDWI()
{
	//Just call the overload mapT1 with the method- member variable
	mapDWI(method_);
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
	return {};
}

//
MDM_API const mdm_Image3D& mdm_DWIMapper::model_map(const std::string& map_name) const
{
	return model_maps_[0];
}

//
MDM_API double mdm_DWIMapper::model_map(const std::string& map_name, size_t voxel) const
{
	return model_maps_[0].voxel(voxel);
}

//
MDM_API mdm_DWIMethodGenerator::DWIMethods  mdm_DWIMapper::method() const
{
	return method_;
}

//
MDM_API void  mdm_DWIMapper::setMethod(mdm_DWIMethodGenerator::DWIMethods method)
{
	method_ = method;
}

//
MDM_API double  mdm_DWIMapper::noiseThreshold() const
{
	return noiseThreshold_;
}

//
MDM_API void  mdm_DWIMapper::setNoiseThreshold(double t)
{
	noiseThreshold_ = t;
}

//******************************************************************
//Private methods
//******************************************************************