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
#include <sstream> // stringstream

#include <madym/mdm_ErrorTracker.h>
#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>
#include <boost/format.hpp>

//
MDM_API mdm_T1Mapper::mdm_T1Mapper(mdm_ErrorTracker &errorTracker)
	:inputImages_(0),
	errorTracker_(errorTracker),
	noiseThreshold_(0),
	method_(mdm_T1MethodGenerator::T1Methods::VFA)
{}

//
MDM_API mdm_T1Mapper::~mdm_T1Mapper()
{}

//
MDM_API void mdm_T1Mapper::addInputImage(mdm_Image3D img)
{
	/* MB - Copied this Gio comment from elsewhere - here would be an obvious place to enforce this check!
	* Note that there is an assumption throughout MaDyM that all images used in the analysis have the same
	* dimensions.  This is why we intialise with all the values from first_image.  However, this
	* assumption is never tested.  It would robustify the code to include tests at the file-reading stage.
	*/
  if (inputImages_.empty())
    errorTracker_.initErrorImage(img); //If already set, returns silently

	inputImages_.push_back(img);
}

//
MDM_API void mdm_T1Mapper::setT1(mdm_Image3D T1_img)
{
	T1_ = T1_img;
}

//
MDM_API void mdm_T1Mapper::setM0(mdm_Image3D M0_img)
{
	M0_ = M0_img;
}

//
MDM_API void mdm_T1Mapper::setROI(mdm_Image3D ROI)
{
	ROI_ = ROI;
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

	bool useROI = ROI_.numVoxels() > 0;

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
			//Compute T1 and M0
			double T1, M0;
			T1Fitter->setInputSignals(signal);
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

	std::stringstream ss;
	ss << "mdm_T1Mapper: Fitted " <<
		numFitted << " voxels in " << elapsed_seconds.count() << "s.\n" <<
		numErrors << " voxels returned fit errors\n";
	mdm_ProgramLogger::logProgramMessage(ss.str());
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
  try { return inputImages_[i]; }
  catch (std::out_of_range &e)
  {
    mdm_exception em(__func__, e.what());
    em.append(boost::format(
      "Attempting to access input image %1% when there are %2% input images")
      % i % inputImages_.size());
    throw em;
  }
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