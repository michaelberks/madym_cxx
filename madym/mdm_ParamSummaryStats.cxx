/**
*  @file    mdm_ParamSummaryStats.cxx
*  @brief   Implementation of mdm_ParamSummaryStats class
*
*  Original author MA Berks 13 Nov 2020
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_ParamSummaryStats.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <boost/format.hpp>
#include <madym/mdm_exception.h>

const std::vector<std::string> mdm_ParamSummaryStats::headers_ = {
	"param",
	"n_valid",
	"n_invalid",
	"mean",
	"stddev",
	"median",
	"lowerQ",
	"upperQ",
	"iqr"
};

//!Helper function to compute percentile of vector of values
double percentile(const std::vector<double> &A, const double &prct)
{
	//We only ever call this function for 25,50 and 100, but for completeness
	assert(prct >= 0 && prct <= 100.0);
	if (prct == 0)
		return A[0];
	if (prct == 100)
		return A.back();

	//See https://en.wikipedia.org/wiki/Quartile#Method_4
	double n1 = A.size() + 1;
	double pn1 = n1 * prct / 100.00;
	double k = std::floor(pn1);

	//If size A < 3, then k = 0 for prctile(25) and k = 3 for prctile(75)
	if (!k)
		return A[0];
	else if (k == A.size())
		return A.back();

	double alpha = pn1 - k;
	return A[k - 1] + alpha * (A[k] - A[k - 1]);//Remember indexing starts at 0 in cxx

	/*double x = (double)A.size() * prct / 100.00;
	int i2 = std::floor(x); 
	int i1 = i2 - 1;
	double m1 = 0.5 - (x - (double)i2);
	double m2 = 1.0 - m1;
	return A[i1] * m1 + A[i2] * m2;*/
}

//
MDM_API mdm_ParamSummaryStats::mdm_ParamSummaryStats()
	:
	roiIdx_(0)
{

}

//
MDM_API mdm_ParamSummaryStats::~mdm_ParamSummaryStats()
{
	closeNewStatsFile();
}

//!Set ROI
MDM_API void mdm_ParamSummaryStats::setROI(const mdm_Image3D& roi)
{
	//Don't store the image, just save the non-zero IDX
	roiIdx_.clear();
	for (int i = 0; i < roi.numVoxels(); i++)
	{
		if (roi.voxel(i))
			roiIdx_.push_back(i);
	}
		
	xmm_ = roi.info().Xmm.value();
	ymm_ = roi.info().Ymm.value();
	zmm_ = roi.info().Zmm.value();
}

//!Make output stats for an image given an ROI
MDM_API void mdm_ParamSummaryStats::makeStats(const mdm_Image3D& img, const std::string &paramName, 
	const double scale, bool invert)
{
	//Set param name
	stats_.paramName_ = paramName;

	//Check ROI idx are set
	checkIdx(img);

	//Reset the stats
	stats_.reset();

	// loop image extracting parameter values and taking running sum
	double ROI_sum = 0.0;
	double ROI_sumsq = 0.0;

	std::vector<double>  paramVals;
	for (const auto &idx : roiIdx_)
	{
		// Get value from image
		double voxValue = scale * img.voxel(idx);

		if (std::isnan(voxValue))
		{
			stats_.invalidVoxels_++;
			continue;
		}

		if (invert)
		{
			//Can't invert negative values, just skip
			if (voxValue <= 0.0)
			{
				stats_.invalidVoxels_++;
				continue;
			}
				

			else
				voxValue = 1 / voxValue;
		}

		//Increment sums and save value
		ROI_sum += voxValue;
		ROI_sumsq += voxValue * voxValue;
		paramVals.push_back(voxValue);
		stats_.validVoxels_++;
	}

	//If we haven't got any voxels, return
	if (!stats_.validVoxels_)
		return;

	else if (stats_.validVoxels_ == 1)
	{
		stats_.mean_ = paramVals[0];
		stats_.median_ = paramVals[0];
		stats_.lowerQ_ = paramVals[0];
		stats_.upperQ_ = paramVals[0];

		//std and iqr are 0
		return;
	}

	//Compute summary stats from data values
	//Sort the data arrays
	std::sort(paramVals.begin(), paramVals.end());

	//Compute mean and std
	double validVoxels = (double)stats_.validVoxels_;
	stats_.mean_ = ROI_sum / validVoxels;

	// This is the unbiased estimator for the sd of the parent distribution ... i.e. strong gaussian assumption
	stats_.stddev_ = sqrt((ROI_sumsq - ROI_sum * ROI_sum /
		validVoxels) / (validVoxels - 1));

	//Compute median and IQR
	stats_.median_ = percentile(paramVals, 50);
	stats_.lowerQ_ = percentile(paramVals, 25);
	stats_.upperQ_ = percentile(paramVals, 75);
	stats_.iqr_ = stats_.upperQ_ - stats_.lowerQ_;
}

MDM_API const mdm_ParamSummaryStats::SummaryStats& mdm_ParamSummaryStats::stats() const
{
	return stats_;
}

//
MDM_API void mdm_ParamSummaryStats::writeROISummary(const std::string &roiFile)
{
	std::ofstream roiStream(roiFile);
	if (!roiStream)
	  throw mdm_exception(__func__, boost::format("Failed to open stats file %1%") % roiFile);
		

	double nVoxels = (double)roiIdx_.size();
	roiStream <<
		"number_of_voxels = " << nVoxels <<
		" volume = " << nVoxels * xmm_ * ymm_ * zmm_;
}

//
MDM_API void mdm_ParamSummaryStats::openNewStatsFile(const std::string &statsFile)
{
	statsOStream_.open(statsFile, std::ios::out);
	if (!statsOStream_)
    throw mdm_exception(__func__, boost::format("Failed to open stats file %1%") % statsFile);

	//Write stats headers
	for (const auto hdr : headers_)
		statsOStream_ << hdr << ",";
	
	statsOStream_ << "\n";

}

//
MDM_API void mdm_ParamSummaryStats::closeNewStatsFile()
{
	if (statsOStream_)
		statsOStream_.close();

}

//
MDM_API void mdm_ParamSummaryStats::writeStats()
{
	if (!statsOStream_.is_open())
    throw mdm_exception(__func__, 
      "Tried to write stats, but no stats file open");

	statsOStream_ <<
		stats_.paramName_ << "," <<
		stats_.validVoxels_ << "," <<
		stats_.invalidVoxels_ << "," <<
		stats_.mean_ << "," <<
		stats_.stddev_ << "," <<
		stats_.median_ << ","<<
		stats_.lowerQ_ << "," <<
		stats_.upperQ_ << "," <<
		stats_.iqr_ << ",\n";

}

//
MDM_API void mdm_ParamSummaryStats::openStatsFile(const std::string &statsFile)
{
	statsIStream_.open(statsFile, std::ios::in);
	if (!statsIStream_)
    throw mdm_exception(__func__, boost::format("Failed to open stats file %1%") % statsFile);

	//Write stats headers
	std::string hdr_in;
	for (const auto hdr : headers_)
	{
		std::getline(statsIStream_, hdr_in, ',');
		if (hdr_in != hdr)
      throw mdm_exception(__func__, boost::format("Incorrect headers in %1%, cannot open") % statsFile);
	}
	//Get rid of final comma to \n part
	std::getline(statsIStream_, hdr_in);
}

//
MDM_API void mdm_ParamSummaryStats::closeStatsFile()
{
	if (statsIStream_)
		statsIStream_.close();
}

//
MDM_API void mdm_ParamSummaryStats::readStats()
{
	if (!statsIStream_.is_open())
    throw mdm_exception(__func__,
      "Tried to read stats, but no stats file open");

	std::getline(statsIStream_, stats_.paramName_, ',');

	std::string val;
	std::getline(statsIStream_, val, ',');
	stats_.validVoxels_ = std::stoi(val);

	std::getline(statsIStream_, val, ',');
	stats_.invalidVoxels_ = std::stoi(val);

	std::getline(statsIStream_, val, ',');
	stats_.mean_ = std::stod(val);

	std::getline(statsIStream_, val, ',');
	stats_.stddev_ = std::stod(val);

	std::getline(statsIStream_, val, ',');
	stats_.median_ = std::stod(val);

	std::getline(statsIStream_, val, ',');
	stats_.lowerQ_ = std::stod(val);

	std::getline(statsIStream_, val, ',');
	stats_.upperQ_ = std::stod(val);

	std::getline(statsIStream_, val, ',');
	stats_.iqr_ = std::stod(val);

	//Get rid of final comma to \n part
	std::getline(statsIStream_, val);
}

//------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------
void mdm_ParamSummaryStats::checkIdx(const mdm_Image3D& img)
{
	if (roiIdx_.empty())
	{
		for (int i = 0; i < img.numVoxels(); i++)
			roiIdx_.push_back(i);
	}

	xmm_ = img.info().Xmm.value();
	ymm_ = img.info().Ymm.value();
	zmm_ = img.info().Zmm.value();
}
