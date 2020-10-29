/**
 *  @file    mdm_T1VolumeAnalysis.cxx
 *  @brief   implementation of mdm_T1VolumeAnalysis class
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1VolumeAnalysis.h"

#include <cassert>
#include <chrono>  // chrono::system_clock
#include <sstream> // stringstream

#include "mdm_ErrorTracker.h"
#include "mdm_ProgramLogger.h"

MDM_API mdm_T1VolumeAnalysis::mdm_T1VolumeAnalysis(mdm_ErrorTracker &errorTracker)
	:inputImages_(0),
	errorTracker_(errorTracker),
	noiseThreshold_(0),
	method_(mdm_T1MethodGenerator::T1Methods::VFA)
{

}

MDM_API mdm_T1VolumeAnalysis::~mdm_T1VolumeAnalysis()
{
}

MDM_API void mdm_T1VolumeAnalysis::addInputImage(mdm_Image3D FA_img)
{
	/* MB - Copied this Gio comment from elsewhere - here would be an obvious place to enforce this check!
	* Note that there is an assumption throughout MaDyM that all images used in the analysis have the same
	* dimensions.  This is why we intialise with all the values from first_image.  However, this
	* assumption is never tested.  It would robustify the code to include tests at the file-reading stage.
	*/
	inputImages_.push_back(FA_img);
}

MDM_API void mdm_T1VolumeAnalysis::addT1Map(mdm_Image3D T1_img)
{
	T1_ = T1_img;
}

MDM_API void mdm_T1VolumeAnalysis::addM0Map(mdm_Image3D M0_img)
{
	M0_ = M0_img;
}

MDM_API void mdm_T1VolumeAnalysis::addROI(mdm_Image3D ROI)
{
	ROI_ = ROI;
}

//
MDM_API void  mdm_T1VolumeAnalysis::mapT1(mdm_T1MethodGenerator::T1Methods method)
{
	//
	int nSignals = inputImages_.size();

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
	for (int voxelIndex = 0, n = M0_.numVoxels(); voxelIndex < n; voxelIndex++)
	{
		if (useROI && !ROI_.voxel(voxelIndex))
			continue;

		//Get signals at this voxel
		std::vector<double> signal(nSignals);
		for (int i_f = 0; i_f < nSignals; i_f++)
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
	ss << "mdm_T1VolumeAnalysis: Fitted " <<
		numFitted << " voxels in " << elapsed_seconds.count() << "s.\n" <<
		numErrors << " voxels returned fit errors\n";
	mdm_ProgramLogger::logProgramMessage(ss.str());
}

//
MDM_API void mdm_T1VolumeAnalysis::mapT1()
{
	//Just call the overload mapT1 with the method- member variable
	mapT1(method_);
}

MDM_API const std::vector<mdm_Image3D>& mdm_T1VolumeAnalysis::inputImages() const
{
	return inputImages_;
}

MDM_API const mdm_Image3D& mdm_T1VolumeAnalysis::inputImage(int i) const
{
	assert(i >= 0 && i < inputImages_.size());
	return inputImages_[i];
}
MDM_API const mdm_Image3D& mdm_T1VolumeAnalysis::T1Map() const
{
	return T1_;
}
MDM_API const mdm_Image3D& mdm_T1VolumeAnalysis::M0Map() const
{
	return M0_;
}

MDM_API double mdm_T1VolumeAnalysis::T1atVoxel(int voxel) const
{
	return T1_.voxel(voxel);
}

MDM_API double mdm_T1VolumeAnalysis::M0atVoxel(int voxel) const
{
	return M0_.voxel(voxel);
}

MDM_API void mdm_T1VolumeAnalysis::zeroVoxel(int voxel)
{
	T1_.setVoxel(voxel, 0);
	M0_.setVoxel(voxel, 0);
}

//
MDM_API mdm_T1MethodGenerator::T1Methods  mdm_T1VolumeAnalysis::method() const
{
	return method_;
}

//
MDM_API void  mdm_T1VolumeAnalysis::setMethod(mdm_T1MethodGenerator::T1Methods method)
{
	method_ = method;
}

MDM_API double  mdm_T1VolumeAnalysis::noiseThreshold() const
{
	return noiseThreshold_;
}
MDM_API void  mdm_T1VolumeAnalysis::setNoiseThreshold(double t)
{
	noiseThreshold_ = t;
}
//******************************************************************
//Private methods
//******************************************************************