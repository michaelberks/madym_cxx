/**
*  @file    mdm_T1Mapper.cxx
*  @brief   implementation of mdm_T1Mapper class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1Mapper.h"

#include <cassert>
#include <chrono>  // chrono::system_clock

#include <madym/mdm_ErrorTracker.h>
#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>
#include <boost/format.hpp>

//
MDM_API mdm_T1Mapper::mdm_T1Mapper(mdm_ErrorTracker &errorTracker, mdm_Image3D &ROI)
	:inputImages_(0),
	errorTracker_(errorTracker),
  ROI_(ROI),
	noiseThreshold_(0),
	method_(mdm_T1MethodGenerator::T1Methods::VFA)
{}

//
MDM_API mdm_T1Mapper::~mdm_T1Mapper()
{}

//
MDM_API void mdm_T1Mapper::reset()
{
  inputImages_.clear();
  T1_.reset(); 
  M0_.reset();
}

//
MDM_API void mdm_T1Mapper::addInputImage(mdm_Image3D img)
{
  checkOrSetDimension(img);
	inputImages_.push_back(img);
}

//
MDM_API void mdm_T1Mapper::setT1(mdm_Image3D T1)
{
  checkOrSetDimension(T1);
	T1_ = T1;
}

//
MDM_API void mdm_T1Mapper::setM0(mdm_Image3D M0)
{
  checkOrSetDimension(M0);
	M0_ = M0;
}

//
MDM_API void mdm_T1Mapper::setB1(mdm_Image3D B1)
{
  checkOrSetDimension(B1);
  B1_ = B1;
}

//
MDM_API void  mdm_T1Mapper::mapT1(mdm_T1MethodGenerator::T1Methods method)
{
	//
	auto nSignals = inputImages_.size();

	//Instantiate T1 fitter object of required method type
	auto T1Fitter = mdm_T1MethodGenerator::createFitter(method, inputImages_);

	mdm_ErrorTracker::ErrorCode errCode = mdm_ErrorTracker::OK;

	//
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
			signal[i_f] = inputImages_[i_f].voxel(voxelIndex);   /* sig FA_1 */

		//TODO - MB, why only check the first signal?				
		if (signal[0] > noiseThreshold_)
		{
      //If using B1 correction, add this to the inputs
      if (useB1_)
        signal.push_back(B1_.voxel(voxelIndex));

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
}

//
MDM_API void mdm_T1Mapper::mapT1()
{
	//Just call the overload mapT1 with the method- member variable
	mapT1(method_);
}

//
MDM_API const std::vector<mdm_Image3D>& mdm_T1Mapper::inputImages() const
{
	return inputImages_;
}

//
MDM_API const mdm_Image3D& mdm_T1Mapper::inputImage(size_t i) const
{
  if (i >= inputImages_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access input image %1% when there are %2% input images")
      % i % inputImages_.size());

  return inputImages_[i];
}

//
MDM_API const mdm_Image3D& mdm_T1Mapper::T1() const
{
	return T1_;
}

//
MDM_API const mdm_Image3D& mdm_T1Mapper::M0() const
{
	return M0_;
}

//
MDM_API const mdm_Image3D& mdm_T1Mapper::B1() const
{
  return B1_;
}

//
MDM_API double mdm_T1Mapper::T1(size_t voxel) const
{
	return T1_.voxel(voxel);
}

//
MDM_API double mdm_T1Mapper::M0(size_t voxel) const
{
	return M0_.voxel(voxel);
}

//
MDM_API double mdm_T1Mapper::B1(size_t voxel) const
{
  return B1_.voxel(voxel);
}

//
MDM_API void mdm_T1Mapper::zeroVoxel(size_t voxel)
{
	T1_.setVoxel(voxel, 0);
	M0_.setVoxel(voxel, 0);
}

//
MDM_API mdm_T1MethodGenerator::T1Methods  mdm_T1Mapper::method() const
{
	return method_;
}

//
MDM_API void  mdm_T1Mapper::setMethod(mdm_T1MethodGenerator::T1Methods method)
{
	method_ = method;
}

MDM_API double  mdm_T1Mapper::noiseThreshold() const
{
	return noiseThreshold_;
}
MDM_API void  mdm_T1Mapper::setNoiseThreshold(double t)
{
	noiseThreshold_ = t;
}
//******************************************************************
//Private methods
//******************************************************************

void mdm_T1Mapper::checkOrSetDimension(const mdm_Image3D &img)
{
  if (!errorTracker_.errorImage())
    errorTracker_.initErrorImage(img);

  else if (!img.dimensionsMatch(errorTracker_.errorImage()))
    throw mdm_dimension_mismatch(__func__, errorTracker_.errorImage(), img);
}