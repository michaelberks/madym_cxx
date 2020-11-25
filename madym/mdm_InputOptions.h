/**
*  @file    mdm_InputOptions.h
*  @brief   Header only class defines default input options for T1 mapping and DCE analysis
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_INPUT_OPTIONS_HDR
#define MDM_INPUT_OPTIONS_HDR

#include <mdm_version.h>
#include <mdm_InputTypes.h>

//! Input options structure defining default input options for T1 mapping and DCE analysis
struct mdm_InputOptions {

	//Generic input options applied to all command-line tools
	mdm_input_string configFile = mdm_input_string(
		mdm_input_str(""), "config", "c", "Read input parameters from a configuration file");
	mdm_input_string dataDir = mdm_input_string(
		mdm_input_str(""), "cwd", "", "Set the working directory");

	//DCE input options
	mdm_input_bool inputCt = mdm_input_bool(
		false, "Ct", "",
		"Flag specifying input dynamic sequence are concentration (not signal) maps");
	mdm_input_string dynName = mdm_input_string(
		mdm_input_str("dyn_"), "dyn", "d",
		"Root name for dynamic volumes");
	mdm_input_string dynDir = mdm_input_string(
		mdm_input_str(""), "dyn_dir", "",
		"Folder containing dynamic volumes, can be left empty if already included in option --dyn");
	mdm_input_string dynFormat = mdm_input_string(
		mdm_input_str("%01u"), "dyn_name_format", "",
		"Number format for suffix specifying temporal index of dynamic volumes");
	mdm_input_int nDyns = mdm_input_int(
		0, "n_dyns", "n",
		"Number of DCE volumes, if 0 uses all images matching file pattern");
	mdm_input_int injectionImage = mdm_input_int(
		8, "inj", "i", "Injection image");

	//ROI options
	mdm_input_string roiName = mdm_input_string(
		mdm_input_str(""), "roi", "", "Path to ROI map");

	//T1 calculation options
	mdm_input_string T1method = mdm_input_string(
		mdm_input_str("VFA"), "T1_method", "T",
		"Method used for baseline T1 mapping");
	mdm_input_strings T1inputNames = mdm_input_strings(
		mdm_input_string_list(std::vector<std::string>{}), "T1_vols", "",
		"Filepaths to input signal volumes (eg from variable flip angles)");
	mdm_input_double T1noiseThresh = mdm_input_double(
		0.0, "T1_noise", "",
		"Noise threshold for fitting baseline T1");
	mdm_input_int nT1Inputs = mdm_input_int(
		0, "n_T1", "",
		"Number of input signals for baseline T1 mapping");

	//Signal to concentration options
	mdm_input_bool M0Ratio = mdm_input_bool(
		true, "M0_ratio", "",
		"Flag to use ratio method to scale signal instead of precomputed M0");
	mdm_input_string T1Name = mdm_input_string(
		mdm_input_str(""), "T1", "",
		"Path to precomputed T1 map");
	mdm_input_string M0Name = mdm_input_string(
		mdm_input_str(""), "M0", "",
		"Path to precomputed M0 map");
	mdm_input_double r1Const = mdm_input_double(
		3.4, "r1", "", "Relaxivity constant of concentration in tissue");

	//AIF options
  mdm_input_int aifType = mdm_input_int(
    0, "aif_type", "",
    "AIF type, overriden by --aif and --aif_map");
	mdm_input_string aifName = mdm_input_string(
		mdm_input_str(""), "aif", "",
		"Path to precomputed AIF, if not set uses population AIF");
  mdm_input_string aifMap = mdm_input_string(
    mdm_input_str(""), "aif_map", "", "Map of voxels to average in AIF computation");
	mdm_input_string pifName = mdm_input_string(
		mdm_input_str(""), "pif", "",
		"Path to precomputed AIF, if not set derives from AIF");
	mdm_input_double dose = mdm_input_double(
		0.1, "dose", "D", "Contrast-agent dose");
	mdm_input_double hct = mdm_input_double(
		0.42, "hct", "H", "Haematocrit level");

	//Model options
	mdm_input_string model = mdm_input_string(
		mdm_input_str(""), "model", "m",
		"Tracer-kinetic model");
	mdm_input_doubles initialParams = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{}), "init_params", "",
		"Initial values for model parameters to be optimised");
	mdm_input_string initMapsDir = mdm_input_string(
		mdm_input_str(""), "init_maps", "",
		"Path to folder containing parameter maps for per-voxel initialisation");
	mdm_input_ints initMapParams = mdm_input_ints(
		mdm_input_int_list(std::vector<int>{}), "init_map_params", "",
		"Index of parameters sampled from maps");
	mdm_input_strings paramNames = mdm_input_strings(
		mdm_input_string_list(std::vector<std::string>{}), "param_names", "",
		"Names of model parameters, used to override default output map names");
	mdm_input_ints fixedParams = mdm_input_ints(
		mdm_input_int_list(std::vector<int>{}), "fixed_params", "",
		"Index of parameters fixed to their initial values");
	mdm_input_doubles fixedValues = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{}), "fixed_values", "",
		"Values for fixed parameters");
	mdm_input_ints relativeLimitParams = mdm_input_ints(
		mdm_input_int_list(std::vector<int>{}), "relative_limit_params", "",
		"Index of parameters to which relative limits are applied");
	mdm_input_doubles relativeLimitValues = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{}), "relative_limit_values", "",
		"Values for relative limits - optimiser bounded to range initParam +/- relLimit");
	mdm_input_int firstImage = mdm_input_int(
		0, "first", "",
		"First image used in model fit cost function");
	mdm_input_int lastImage = mdm_input_int(
		0, "last", "",
		"Last image used in model fit cost function");

	mdm_input_bool noOptimise = mdm_input_bool(
		false, "no_opt", "",
		"Flag to turn-off optimisation, default false");
	mdm_input_bool dynNoise = mdm_input_bool(
		false, "dyn_noise", "",
		"Flag to use varying temporal noise in model fit, default false");
	mdm_input_bool testEnhancement = mdm_input_bool(
		true, "test_enh", "",
		"Flag to test for enhancement before fitting model, default true");
	mdm_input_int maxIterations = mdm_input_int(
		0, "max_iter", "",
		"Max iterations per voxel in optimisation - 0 for no limit");

	//DCE only output options
	mdm_input_bool outputCt_sig = mdm_input_bool(
		false, "Ct_sig", "",
		"Flag to save signal-derived dynamic concentration maps");
	mdm_input_bool outputCt_mod = mdm_input_bool(
		false, "Ct_mod", "",
		"Flag to save modelled dynamic concentration maps");
	mdm_input_doubles IAUCTimes = mdm_input_doubles(
		mdm_input_double_list({ 60.0,90.0,120.0 }), "iauc", "I",
		"Times (in s, post-bolus injection) at which to compute IAUC");

  //AIF detection
  mdm_input_ints aifSlices = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "aif_slices", "",
    "Slices used to automatically measure AIF");
  mdm_input_ints aifXrange = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "aif_x_range", "",
    "Range of voxels to consider as AIF candidates in x-axis");
  mdm_input_ints aifYrange = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "aif_y_range", "",
    "Range of voxels to consider as AIF candidates in y-axis");
  mdm_input_double minT1Blood = mdm_input_double(
    1000, "min_T1_blood", "", "Minimum T1 to be considered as potential blood voxel");
  mdm_input_double peakTime = mdm_input_double(
    1.0, "peak_time", "", "Time window post bolus for peak to arrive");
  mdm_input_double prebolusNoise = mdm_input_double(
    0, "prebolus_noise", "", "Estimate of noise on the prebolus signal used when unable to compute");
  mdm_input_int prebolusMinImages = mdm_input_int(
    5, "prebolus_min_images", "", "Minimum number of images required to estimate prebolus noise");
  mdm_input_double selectPct = mdm_input_double(
    0, "select_pct", "", "Percentage of candidates to select");

	//General output options
	mdm_input_string outputRoot = mdm_input_string(
		mdm_input_str(""), "output_root", "", "Optional base for output folder");
	mdm_input_string outputDir = mdm_input_string(
		mdm_input_str(""), "output", "o", "Output folder");
	mdm_input_bool overwrite = mdm_input_bool(
		false, "overwrite", "",
		"Flag to overwrite existing analysis in output dir, default false");
	mdm_input_bool sparseWrite = mdm_input_bool(
		false, "sparse", "",
		"Flag to write output in sparse Analyze format");

	//Logging options
  mdm_input_bool noLog = mdm_input_bool(
    false, "no_log", "",
    "Switch off program logging");
  mdm_input_bool noAudit = mdm_input_bool(
    false, "no_audit", "",
    "Switch off audit logging");
  mdm_input_bool quiet = mdm_input_bool(
    false, "quiet", "q",
    "Do not display logging messages in cout");
	mdm_input_string errorCodesName = mdm_input_string(
		mdm_input_str("error_codes"), "err", "E",
		"Filename of error codes map");
	mdm_input_string programLogName = mdm_input_string(
		mdm_input_str("ProgramLog.txt"), "program_log", "",
		"Filename of program log, will be appended with datetime");
	mdm_input_string outputConfigFileName = mdm_input_string(
		mdm_input_str("config.txt"), "config_out", "",
		"Filename of output config file, will be appended with datetime");
	mdm_input_string auditLogBaseName = mdm_input_string(
		mdm_input_str("AuditLog.txt"), "audit", "",
		"Filename of audit log, will be appended with datetime");
	mdm_input_string auditLogDir = mdm_input_string(
		mdm_input_str("audit_logs/"), "audit_dir", "",
		"Folder in which audit output is saved");

	//Madym-lite only options
	mdm_input_string dynTimesFile = mdm_input_string(
		mdm_input_str(""), "times", "t",
		"Path to file containing dynamic times");
	mdm_input_string initParamsFile = mdm_input_string(
		mdm_input_str(""), "init_params_file", "",
		"Path to data file containing maps of parameters to initialise fit");
	mdm_input_string dynNoiseFile = mdm_input_string(
		mdm_input_str(""), "dyn_noise_file", "", "");

	//Lite options
	mdm_input_string inputDataFile = mdm_input_string(
		mdm_input_str(""), "data", "", "Input data filename, see notes for options");
	mdm_input_double FA = mdm_input_double(
		0, "FA", "", "FA of dynamic series");
	mdm_input_double TR = mdm_input_double(
		0, "TR", "", "TR of dynamic series");
	mdm_input_string outputName = mdm_input_string(
		mdm_input_str("madym_analysis.dat"), "output_name", "O", "Name of output data file");

};

#endif /* mdm_RunTools_HDR */
