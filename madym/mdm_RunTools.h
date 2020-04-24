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
#include <madym/mdm_FileManager.h>
#include <madym/mdm_DCEVolumeAnalysis.h>
#include <madym/mdm_AIF.h>
#include <madym/mdm_T1VolumeAnalysis.h>
#include <madym/mdm_ErrorTracker.h>

#include <madym/mdm_DCEModelBase.h>

#include <string>
#include <vector>

/**
*  @brief   Defines options and their default values for all inputs to DCE analysis and T1 mapping tools
*/
class mdm_ToolsOptions {
public:
  //:All madym  options
  //--------------------------------------------------------------------
	
	const static std::string empty_str;
	MDM_API bool to_stream(std::ostream &str) const;
	MDM_API bool to_file(const std::string &filename) const;
	MDM_API bool from_file(const std::string &filename);

	//Generic input options
	std::string configFile = "";
	std::string dataDir = "";
	std::string inputDataFile = "";
	std::string roiName = "";

  //DCE input options
	bool inputCt = false;
  std::string dynDir = "";
  std::string dynName = "dyn_";
  std::string dynFormat = "%01u";
  std::string dynTimesFile = "";
	int nDyns = 0;

  //T1 calculation options
  std::string T1method = "VFA";
  std::vector<std::string> T1inputNames = {};
	double T1noiseThresh = 0.0;
 
  //Signal to concentration options
  double r1Const = 3.4;
  int injectionImage = 8;
  double dose = 0.1;
  double hct = 0.42;
	double TR = 0;
	double FA = 0;
  
  //Baseline T1 options
  std::string T1Name = "";
  std::string S0Name = "";
  bool useRatio = true;

  //Logging options
  std::string errorCodesName = "error_codes";
	std::string programLogName = "ProgramLog.txt";
	std::string outputConfigFileName = "config.txt";
  std::string auditLogBaseName = "AuditLog.txt";
  std::string auditLogDir = "audit_logs/";

  //AIF options
  std::string aifName = "";
  std::string pifName = "";
  int aifSlice = 0;

  //Output options
  std::string outputDir = "";
	std::string outputName = "madym_analysis.dat";
  bool overwrite = false;
  bool outputCt = false;
  bool outputCm = false;
	bool sparseWrite = false;
  std::vector<double> IAUCTimes = {60.0,90.0,120.0};

  //Model options
  std::string model = "";
  std::vector<double> initParams = {};
	std::string initMapsDir = "";
	std::vector<int> initMapParams = {};
	std::string initParamsFile = "";
	std::vector<std::string> paramNames = {};
  std::vector<int> fixedParams = {};
  std::vector<double> fixedValues = {};
	std::vector<int> relativeLimitParams = {};
	std::vector<double> relativeLimitValues = {};
  int firstImage = 0;
  int lastImage = 0;
  bool noOptimise = false;
  bool dynNoise = false;
	std::string dynNoiseFile = "";
  bool noEnhFlag = false;
  int maxIterations = 0;
  
  bool help = false;
  bool version = false;

  //:All madym  options - help strings
  //--------------------------------------------------------------------
	//Generic input options
	std::string configFileText = "Read input parameters from a configuration file";
	std::string dataDirText = "Set the working directory";
	std::string inputDataFileText = "Input data filename, see notes for options";
	std::string roiNameText = "Path to ROI map";

  //DCE input options
  std::string inputCtText = "Flag specifying input dynamic sequence are concentration (not signal) maps";
  std::string dceDirText = "Directory containing dynamic input volumes";
  std::string dynNameText = "Template name for dynamic sequences eg. dynamic/dyn_";
  std::string dynFormatText = "Number format for suffix specifying temporal index of dynamic volumes";
  std::string dynTimesFileText = "Time associated with each dynamic signal, see notes for options";
	std::string nDynsText = "Number of time-points in dynamic series";

  //T1 calculation options
  std::string T1methodText = "T1 method to use to fit, see notes for options, currently only VFA implemented";
  std::string T1inputNamesText = "Fielpaths to input signal volumes (eg from variable flip angles) file names, comma separated (no spaces)";
	std::string T1noiseThreshText = "Noise threshold for fitting baseline T1";

  //Signal to concentration options
  std::string injectionImageText = "Injection image";
  std::string r1ConstText = "Relaxivity constant of concentration in tissue";
  std::string doseText = "Concentration dose";
  std::string hctText = "Haematocrit correction";
	std::string TRText = "TR of dynamic series";
	std::string FAText = "Flip angle of dynamic series";

  //Baseline T1 options
  std::string T1NameText = "Path to T1 map";
  std::string S0NameText = "Path to S0 map";
  std::string useRatioText = "Flag to use ratio method to scale signal instead of supplying S0";

  //Logging options
  std::string programLogNameText = "Program log filename";
	std::string outputConfigFileNameText = "Config file of  filename";
	std::string auditLogBaseNameText = "Base of audit log filename, will be appended with date and time";
  std::string auditLogDirText = "Folder in which audit output is saved";
  std::string errorCodesNameText = "Error codes image filename";

  //AIF options
  std::string aifNameText = "Path to precomputed AIF if not using population AIF";
  std::string pifNameText = "Path to precomputed PIF if not deriving from AIF";
  std::string aifSliceText = "Slice used to automatically derive AIF";

  //Output options
  std::string outputDirText = "Output path";
	std::string outputNameText = "Output path";
	std::string overwriteText = "Set overwrite existing analysis in output dir ON";
  std::string outputCtText = "Flag requesting concentration (derived from signal) are saved to output";
  std::string outputCmText = "Flag requesting modelled concentration maps are saved to output";
	std::string sparseWriteText = "Flag requesting output files are written in Analyze sparse format";

  //Model options
  std::string modelText = "Model to fit, see notes for options";
  std::string IAUCTimesText = "Times (in s) at which to compute IAUC maps";
  std::string initParamsText = "Initial values for model parameters to be optimised";
  std::string initMapsDirText = "Path to directory containing maps of parameters to initialise fit (overrides init_params)";
	std::string initMapParamsText = "Index of parameters sampled from maps. If empty and initMapsDir set, takes all params from input maps";
	std::string initParamsFileText = "Path to data file containing maps of parameters to initialise fit (overrides init_params)";
	std::string paramNamesText = "Names of model parameters to be optimised, used to name the output parameter maps, comma separated (no spaces)";
  std::string fixedParamsText = "Index of parameters fixed to their initial values (ie not optimised)";
  std::string fixedValuesText = "Values for fixed parameters (overrides default initial parameter values)";
	std::string relativeLimitParamsText = "Index of parameters to which relative limits are applied";
	std::string relativeLimitValuesText = "Values for relative limits - optimiser capped to range initParam +/- relLimit";
	std::string firstImageText = "First image";
  std::string lastImageText = "Last image";
  std::string dynNoiseText = "Set to use varying temporal noise in model fit OFF";
	std::string dynNoiseFileText = "File to set varying temporal noise in model fit";
  std::string noOptimiseText = "Flag to switch off optimising, will just fit initial parameters values for model";
  std::string noEnhFlagText = "Set test-for-enhancement flag OFF";
  std::string maxIterationsText = "Max iterations per voxel in optimisation - set as 0 for no limit";

  //Other stuff 
  std::string helpText = "Show this usage and quit";
  std::string versionText = "Print version to stdout and quit";
};

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
	MDM_API mdm_RunTools(mdm_ToolsOptions &options);
		
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
	MDM_API int run_DCEFit(const std::string &exe_args, const std::string &exeString);

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_DCEFit_lite(const std::string &exe_args, const std::string &exeString);
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_CalculateT1(const std::string &exe_args, const std::string &exeString);

	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_CalculateT1_lite(const std::string &exe_args, const std::string &exeString,
		const int nSignals);
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run_AIFFit(const std::string &exe_args, const std::string &exeString);

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
		const bool&noEnhFlag,
		const bool&useRatio,
		const std::vector<double> &IAUCTimes,
		const bool &outputCm,
		const bool &outputCt,
		const bool &optimiseModel);

  //Variables:
  mdm_ToolsOptions options_;

  mdm_ErrorTracker errorTracker_;
  mdm_AIF AIF_;
  mdm_T1VolumeAnalysis T1Mapper_;
  mdm_DCEVolumeAnalysis volumeAnalysis_;
  mdm_DCEModelBase *model_;

  mdm_FileManager fileManager_;

  std::string exeString_;
	


};

#endif /* mdm_RunTools_HDR */
