/*!
 *  @file    mdm_ParamSummaryStats.h
 *  @brief   Class that holds summary statistics for an output parameter map
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_PARAMSUMMARYSTATS_HDR
#define MDM_PARAMSUMMARYSTATS_HDR
#include <madym/mdm_api.h>

#include <madym/mdm_Image3D.h>

#include <iostream>

//! Class to compute and store summary stats for an output parameter map
class mdm_ParamSummaryStats {
public:

	//!Helper class to hold set of basic summary stats (mean, median etc)
	struct SummaryStats {
		std::string paramName_; //!< Name of parameter
		double mean_; //!< Mean
		double stddev_; //!< Standard deviation
		double median_; //!< Median
		double lowerQ_; //!< Lower quartile (ie 25th percentile)
		double upperQ_; //!< Upper quartile (ie 75th percentile)
		double iqr_; //!< Inter-quartile range
		int validVoxels_; //!< Number of valid voxels
		int invalidVoxels_; //!< Number of invalid voxels

		//!Reset all values to zero
		void reset() {
			mean_ = 0;
			stddev_ = 0;
			median_ = 0;
			lowerQ_ = 0;
			upperQ_ = 0;
			iqr_ = 0;
			validVoxels_ = 0;
			invalidVoxels_ = 0;
		}
	};

	//!Default constructor
	MDM_API mdm_ParamSummaryStats();

	//!Default destructor
	MDM_API ~mdm_ParamSummaryStats();

	//!Set ROI
	/*!
	\param roi ROI image mask
	*/
	MDM_API void setROI(const mdm_Image3D& roi);

	//!Make output stats for an image
	/*!
	\param img Parameter to image to compute stats for
  \param paramName name of parameter
	\param scale scaling values to apply to parameter values (default 1.0)
	\param invert flag to invert the parameter values (default false). If true, negative values are ignored
	*/
	MDM_API void makeStats(const mdm_Image3D& img, const std::string &paramName, 
		const double scale = 1.0, bool invert = false);

	//!Return the current stats object
	/*!
	\return last computed summary stats
	*/
	MDM_API const SummaryStats& stats() const;

	//!Make ROI summary
	/*!
	\param roiFile filename to write ROI summary to
	\return true if file successfully written, false otherwise
	*/
	MDM_API void writeROISummary(const std::string &roiFile);

	//!Open file stream and write stats headers
	/*!
	\param statsFile filename to write summary stats to
	\return true if file successfully opened, false otherwise
	*/
	MDM_API void openNewStatsFile(const std::string &statsFile);

	//!Close file stream
	/*!
	\return true if file successfully closed, false otherwise
	*/
	MDM_API void closeNewStatsFile();

	//!Write out current stats to new line in output stream
	/*!
	\return true if stats successfully written, false otherwise
	*/
	MDM_API void writeStats();

	//!Open file stream and write stats headers
	/*!
	\param statsFile filename to write summary stats to
	\return true if file successfully opened, false otherwise
	*/
	MDM_API void openStatsFile(const std::string &statsFile);

	//!Close file stream
	/*!
	\return true if file successfully closed, false otherwise
	*/
	MDM_API void closeStatsFile();

	//!Write out current stats to new line in output stream
	/*!
	\return true if stats successfully written, false otherwise
	*/
	MDM_API void readStats();

private:
	//! Check if roiIdx set, if not, use all voxels
	void checkIdx(const mdm_Image3D& img);

	std::vector<int> roiIdx_;

	SummaryStats stats_;

	std::ofstream statsOStream_;
	std::ifstream statsIStream_;

	double xmm_, ymm_, zmm_;

	static const std::vector<std::string> headers_;

};

#endif /* MDM_PARAMSUMMARYSTATS_HDR */
