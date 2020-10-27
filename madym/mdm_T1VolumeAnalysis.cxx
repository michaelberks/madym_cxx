/**
 *  @file    mdm_T1VolumeAnalysis.c
 *  @brief   T1 image calculation stuff for MaDyM
 *
 *  Original Author GJM Parker 2001-2002
 *  Moved to this file and structurally modified by GA Buonaccorsi
 *  (c) Copyright ISBE, University of Manchester 2002
 *
 *  Last edited GJMP 7/10/03
 *  Last edited GAB  15 July 2005
 *
 *  GAB mods:
 *  26 April 2004
 *  -  Moved all T1-related stuff here (part of cmd-line code generation)
 *  16-17 Sep 2004
 *  -  Altered T1_find() to return when maxIterations is exceeded or when the values
 *     are not in a reasonble range
 *  -  Changed K&R header for T1Function() to ANSI
 *     (Also changed headers of mrqmin() and mrqcof() in mdm_NRUtils.c to match
 *  15 July 2005
 *  -  Renamed functions to indicate variable flip angle method - in preparation for modularisation
 *  -  Removed references to unused x_ and y_pos in function header of T1_calcVarFlipAngle() (was T1_find())
 *  -  Added asserts to document assumptions and give better debug feedback
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1VolumeAnalysis.h"

#include <cassert>
#include <chrono>  // chrono::system_clock
#include <sstream> // stringstream

#include "mdm_T1Voxel.h"
#include "mdm_ErrorTracker.h"
#include "mdm_ProgramLogger.h"

MDM_API mdm_T1VolumeAnalysis::mdm_T1VolumeAnalysis(mdm_ErrorTracker &errorTracker)
	:FA_images_(0),
	errorTracker_(errorTracker),
	noiseThreshold_(100)
{

}

MDM_API mdm_T1VolumeAnalysis::~mdm_T1VolumeAnalysis()
{

}

MDM_API void mdm_T1VolumeAnalysis::addFlipAngleImage(mdm_Image3D FA_img)
{
	/* MB - Copied this Gio comment from elsewhere - here would be an obvious place to enforce this check!
	* Note that there is an assumption throughout MaDyM that all images used in the analysis have the same
	* dimensions.  This is why we intialise with all the values from first_image.  However, this
	* assumption is never tested.  It would robustify the code to include tests at the file-reading stage.
	*/
	FA_images_.push_back(FA_img);
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

/**
 * Pre-conditions:
 * -  first_image, second_image & third_image loaded and holding the respective FA_* images
 * -  noise_threshold holds a valid value for the estimated noise level
 *
 * Post-conditions:
 * -  T1 holds map of T1 values for the current slice, calculated from the three FA_* images
 * -  M0 holds map of M0 values for the current slice, calculated from the three FA_* images
 *
 * Uses mdm_permFit.c file-scope static:
 * -  noise_threshold                           (input only - via its accessor function)
 *
 * Uses madym.h globals:
 * -  first_image, second_image, third_image    (input only)
 * -  T1value, M0value                          (output - values set)
 * -  T1, M0 maps                               (output - values set)
 *
 * Note:  NOT a stand-alone fn - see pre- and post-conditions, and it uses madym.h globals
 *
 * @author   GJM Parker with structural mods by Gio Buonaccorsi
 * @brief    Calculate T1 and M0 maps from the three pre-contrast flip angle image volumes (FA_*)
 * @version  madym 1.20
 */
MDM_API void mdm_T1VolumeAnalysis::T1_mapVarFlipAngle()
{
	mdm_ErrorTracker::ErrorCode errCode = mdm_ErrorTracker::OK;

  /*
   * To allow variable number of flip angles, these are now stored in a vector container
	 * We don't need to mess around with the null memory checking stuff. If the images have been
	 * successfully loaded they'll be in the vector. So just check we have at least the minimum
	 * required number for fitting
   *
   * It would be kinder to prompt the user to load them, but that's for the calling program to
   * deal with.  We can handle that behaviour better with exceptions, but at least the asserts
   * provide useful messages for debug, and they document the assumption of existence.
   */
	int numFAs = FA_images_.size();
  assert(numFAs >= mdm_T1Voxel::MINIMUM_FAS);

  /*
   * Note that there is an assumption throughout MaDyM that all images used in the analysis have the same
   * dimensions.  This is why we intialise with all the values from first_image.  However, this
   * assumption is never tested.  It would robustify the code to include tests at the file-reading stage.
   */

	//MB T1_ and M0_ are no longer globals - they're member variables and must be accessed as such from other
	//calling functions
	T1_.copy(FA_images_[0]);
	T1_.setType(mdm_Image3D::ImageType::TYPE_T1BASELINE);
  
  M0_.copy(FA_images_[0]);
  M0_.setType(mdm_Image3D::ImageType::TYPE_M0MAP);

  /* Loop through images having fun ... */
	int nX, nY, nZ;
	M0_.getDimensions(nX, nY, nZ);

	//Get flip angles (and convert to radians) of the images we've got
	std::vector<double> FAs;
    const auto PI = acos(-1.0);
	for (int i_f = 0; i_f < numFAs; i_f++)
		FAs.push_back(FA_images_[i_f].info_.flipAngle.value()  * PI / 180);
  const std::vector<double> sigma(numFAs, 0.1);

	bool useROI = ROI_.numVoxels() > 0;

	//Get tr value from first FA image - assume same for all images?
	double tr = FA_images_[0].info_.TR.value();
	int numFitted = 0;
	int numErrors = 0;

	//Make T1 calculator object
	mdm_T1Voxel T1Calculator(FAs, tr);

	auto fit_start = std::chrono::system_clock::now();
  for (int x = 0; x < nX; x++)
  {
    for (int y = 0; y < nY; y++)
    {
      for (int z = 0; z < nZ; z++)
      {
        int voxelIndex = x + (y * nX) + (z * nX * nY);
        
				if (useROI && !ROI_.voxel(voxelIndex))
					continue;

				//Get signals at this voxel
				std::vector<double> signal(numFAs);
				for (int i_f = 0; i_f < numFAs; i_f++)
					signal[i_f] = FA_images_[i_f].voxel(voxelIndex);   /* sig FA_1 */
        
				//TODO - MB, why only check the first signal?				
        if (signal[0] > noiseThreshold_)
        {
          double T1, M0;
          T1Calculator.setSignals(signal);
					errCode = T1Calculator.fitT1_VFA(T1, M0);

          if (errCode != mdm_ErrorTracker::OK)
          {
            errorTracker_.updateVoxel(voxelIndex, errCode);
						numErrors++;
          }
          /* ... and use them to fill the image maps. */
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
    }
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

MDM_API mdm_Image3D mdm_T1VolumeAnalysis::FAImage(int i) const
{
	assert(i >= 0 && i < FA_images_.size());
	return FA_images_[i];
}
MDM_API mdm_Image3D mdm_T1VolumeAnalysis::T1Map() const
{
	return T1_;
}
MDM_API mdm_Image3D mdm_T1VolumeAnalysis::M0Map() const
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

MDM_API double  mdm_T1VolumeAnalysis::noiseThreshold() const
{
	return noiseThreshold_;
}
MDM_API void  mdm_T1VolumeAnalysis::setNoiseThreshold(double t)
{
	noiseThreshold_ = t;
}
