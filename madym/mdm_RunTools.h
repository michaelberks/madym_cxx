/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_HDR
#define MDM_RUNTOOLS_HDR
#include "mdm_api.h"
#include <mdm_version.h>
#include <mdm_InputOptions.h>
#include <madym/mdm_FileManager.h>
#include <madym/mdm_DCEVolumeAnalysis.h>
#include <madym/mdm_AIF.h>
#include <madym/mdm_T1VolumeAnalysis.h>
#include <madym/mdm_ErrorTracker.h>

#include <madym/mdm_DCEModelBase.h>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunTools {

public:
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunTools(mdm_DefaultValues &options, mdm_InputOptions &options_parser_);
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_RunTools();
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_DCEFit();

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_DCEFit_lite();
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_CalculateT1();

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_CalculateT1_lite();
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_AIFFit();

protected:
  

private:
  //Methods:
	int mdm_progExit();
  void mdm_progAbort(const std::string &err_str);

	std::string timeNow();

  void setModel(const std::string &model_name, bool auto_aif, bool auto_pif,
    const std::vector<std::string> &paramNames,
    const std::vector<double> &initParams,
    const std::vector<int> fixedParams,
    const std::vector<double> fixedValues,
		const std::vector<int> relativeLimitParams,
		const std::vector<double> relativeLimitValues);

  bool setT1Method(const std::string &method);

	void fit_series(std::ostream &outputData, 
		const std::vector<double> &ts, const bool &inputCt,
		const std::vector<double> &noiseVar,
		const double &t10, const double &s0,
		const double &r1,
		const double &TR,
		const double & FA,
		const int &firstImage,
		const int &lastImage,
		const bool&testEnhancement,
		const bool&useRatio,
		const std::vector<double> &IAUCTimes,
		const bool &outputCt_mod,
		const bool &outputCt_sig,
		const bool &optimiseModel);

	void set_up_logging(boost::filesystem::path outputPath);

  //Variables:
	mdm_DefaultValues options_;
	mdm_InputOptions options_parser_;

  mdm_ErrorTracker errorTracker_;
  mdm_AIF AIF_;
  mdm_T1VolumeAnalysis T1Mapper_;
  mdm_DCEVolumeAnalysis volumeAnalysis_;
  mdm_DCEModelBase *model_;

  mdm_FileManager fileManager_;
	
};

#endif /* mdm_RunTools_HDR */
