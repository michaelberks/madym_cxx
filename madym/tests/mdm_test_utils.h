/**
*  @file    mdm_test_utils.h
*  @brief   Header only class provides some utility methods for unit testing
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/
#ifndef MDM_TEST_UTILS_H
#define MDM_TEST_UTILS_H

#include <boost/filesystem.hpp>
#include "mdm_version.h"
#include <random>

//! Macro to make checking vectors are equal easier
#define BOOST_CHECK_VECTORS(v1, v2) \
	BOOST_CHECK_EQUAL_COLLECTIONS(v1.begin(), v1.end(), v2.begin(), v2.end());

/**
*  @brief   Header only class provides some utility methods for unit testing
*/
class mdm_test_utils {

public:
	//! Return string path to system temporary directory
	/*!
	\return path to temporary directory
	*/
	static std::string temp_dir()
	{
		return boost::filesystem::temp_directory_path().string();
	};

	//! Return string path to directory containing calibration data
	/*!
	Uses MACRO defined during CMake generation to map to calibration
	data copied to binary build dir during project generation

	\return path to calibration data folder
	*/
	static std::string calibration_dir()
	{
		//Auto-generated in mdm_version to be relative to users binary dir
		return std::string(MDM_TEST_CALIBRATION_DIR);
	};

	//!   Return string path to directory containing madym tools executable
	/*! 
	Uses MACRO defined during CMake generation to map to build location of tools
	*/
	static std::string tools_exe_dir()
	{
		//Auto-generated in mdm_version to be relative to users binary dir
		return std::string(MDM_EXE_DIR);
	};

	
	//! Tests if all members of a numeric vector a equal to some tolerance
	/*!
	* \param v1 first vector, must match size of v2
	* \param v2 second vector, must match size of v1
	* \param tol tolerance
	* \return true if |v1[i] - v2[i]| < tol for all i
	*/
	static bool vectors_near_equal(std::vector<double> v1, std::vector<double> v2, double tol)
	{
		// Comparator to compare two vectors are element-wise equal to some tolerance
		std::function< bool(const double&, const double&) > near_to =
			[tol](const double &left, const double & right) {
			return std::abs(left - right) < tol; };

		// Compare two vectors of strings in case insensitive manner
		bool matched = std::equal(v1.begin(), v1.end(), v2.begin(), near_to);
		return matched;
	}

	//! Tests if all members of a numeric vector a equal to some relative tolerance
	/*!
	* \param v1 first vector, must match size of v2
	* \param v2 second vector (typically ground truth), must match size of v1
	* \param tol tolerance
	* \return true if |v1[i] - v2[i]| / |v2[i]| < tol for all i
	*/
	static bool vectors_near_equal_rel(std::vector<double> v1, std::vector<double> v2, double tol)
	{
		// Comparator to compare two vectors are element-wise equal to some tolerance
		//as a fraction of the element from the (target) second vector. If target is 0,
		//check first vector element is less than tol
		std::function< bool(const double&, const double&) > near_to_rel =
			[tol](const double &left, const double & right) {
			if (right)
				return (std::abs(left - right) / std::abs(right))< tol; 
			else
				return std::abs(left) < tol;
		};

		// Compare two vectors of strings in case insensitive manner
		bool matched = std::equal(v1.begin(), v1.end(), v2.begin(), near_to_rel);
		return matched;
	}

	//! Add random Gaussian noise to time-series data
	/*!
	* \param time_series reference to vector of data to which noise will be added
	* \param sigma standard deviation of Gaussian noise added (i.i.d, zero-mean)
	*/
	static void add_noise(std::vector<double> &time_series, const double sigma)
	{
		std::random_device r;
		std::seed_seq seed2{ r(), r(), r(), r(), r(), r(), r(), r() };
		std::mt19937 e2(seed2);
		std::normal_distribution<double> normal_dist(0.0, sigma);
		for (auto &t : time_series)
		{
			double n = normal_dist(e2);
			t += n;
		}
	}
};
#endif //MDM_TEST_UTILS_H