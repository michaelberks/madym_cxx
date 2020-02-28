/**
*  @file    mdm_T1Voxel.h
*  @brief   Class for estimating T1 (and S0) in a single voxel
*  @details Currently only variable flip angle method supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_T1VOXEL_HDR
#define MDM_T1VOXEL_HDR
#include "mdm_api.h"

#include <vector>
#include "opt/optimization.h"

/**
*  @brief   Estimating T1 (and S0) in a single voxel
*  @details Currently only variable flip angle method supported
*/
class mdm_T1Voxel {

public:
    /**
	* @brief
    */
	const static int MINIMUM_FAS;
    
    /**
	* @brief
    */
	const static int MAXIMUM_FAS;
	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_T1Voxel(const std::vector<double> &FAs, const double TR);

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_T1Voxel();

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setFAs(const std::vector<double> &FAs);

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setSignals(const std::vector<double> &signals);

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setTR(const double TR);

  /**
	* @brief

	* @param
	* @return
	*/
	MDM_API int fitT1_VFA(double &T1value, double &S0value);

  /**
	* @brief

	* @param
	* @return
	*/
	MDM_API static double T1toSignal(
		const double T1, const double S0, const double FA, const double TR);

private:
	static void setErrorValuesAndTidyUp(const std::string msg, double &T1, double &S0);

	void computeSignalGradient(const double &T1, const double &S0,
		const double &cosFA, const double &sinFA,
		double &signal, double &signal_dT1, double &signal_dS0);

	void computeSSEGradient(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad);

	static void computeSSEGradientAlglib(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad,
		void *context) {
		static_cast<mdm_T1Voxel*>(context)->computeSSEGradient(
			x, func, grad);
	}

	void initFAs();

	std::vector<double> FAs_;
	std::vector<double> signals_;
	double TR_;
	double delta_;
	int maxIterations_;

	alglib::mincgstate state_;
	alglib::mincgreport rep_;

	//Convenient to cache these when FAs set
	int nFAs_;
	std::vector<double> cosFAs_;
	std::vector<double> sinFAs_;
	
};

#endif /* MDM_T1CALC_HDR */
