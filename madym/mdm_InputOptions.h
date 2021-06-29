/**
*  @file    mdm_InputOptions.h
*  @brief   Header only class defines default input options for T1 mapping and DCE analysis
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_INPUT_OPTIONS_HDR
#define MDM_INPUT_OPTIONS_HDR

#include <mdm_InputTypes.h>

//! Input options structure defining default input options for T1 mapping and DCE analysis
struct mdm_InputOptions {

	//Generic input options applied to all command-line tools
	mdm_input_string configFile = mdm_input_string(
		mdm_input_str(""), "config", "c", 
    "Read input parameters from a configuration file"); //!< See initial value
	mdm_input_string dataDir = mdm_input_string(
		mdm_input_str(""), "cwd", "", "Set the working directory"); //!< See initial value

	//DCE input options
	mdm_input_bool inputCt = mdm_input_bool(
		false, "Ct", "",
		"Flag specifying input dynamic sequence are concentration (not signal) maps"); //!< See initial value
	mdm_input_string dynName = mdm_input_string(
		mdm_input_str("dyn_"), "dyn", "d",
		"Root name for dynamic volumes"); //!< See initial value
	mdm_input_string dynDir = mdm_input_string(
		mdm_input_str(""), "dyn_dir", "",
		"Folder containing dynamic volumes, can be left empty if already included in option --dyn"); //!< See initial value
	mdm_input_int nDyns = mdm_input_int(
		0, "n_dyns", "n",
		"Number of DCE volumes, if 0 uses all images matching file pattern"); //!< See initial value
	mdm_input_int injectionImage = mdm_input_int(
		8, "inj", "i", "Injection image"); //!< See initial value

  //Generic naming conventions
  mdm_input_string sequenceFormat = mdm_input_string(
    mdm_input_str("%01u"), "sequence_format", "",
    "Number format for suffix specifying temporal index of volumes in a sequence"); //!< See initial value
  mdm_input_int sequenceStart = mdm_input_int(
   1, "sequence_start", "",
    "Start index volumes in a sequence"); //!< See initial value
  mdm_input_int sequenceStep = mdm_input_int(
    1, "sequence_step", "",
    "Start index volumes in a sequence"); //!< See initial value

	//ROI options
	mdm_input_string roiName = mdm_input_string(
		mdm_input_str(""), "roi", "", "Path to ROI map"); //!< See initial value
  mdm_input_string errorTrackerName = mdm_input_string(
    mdm_input_str(""), "err", "E",
    "Path to existing error tracker map, if empty a new map created"); //!< See initial value

	//T1 calculation options
	mdm_input_string T1method = mdm_input_string(
		mdm_input_str("VFA"), "T1_method", "T",
		"Method used for baseline T1 mapping"); //!< See initial value
  mdm_input_string T1Dir = mdm_input_string(
    mdm_input_str(""), "T1_dir", "",
    "Folder containing T1 input volumes, can be left empty if already included in option --T1_vols"); //!< See initial value
	mdm_input_strings T1inputNames = mdm_input_strings(
		mdm_input_string_list(std::vector<std::string>{}), "T1_vols", "",
		"Filepaths to input signal volumes (eg from variable flip angles)"); //!< See initial value
	mdm_input_double T1noiseThresh = mdm_input_double(
		0.0, "T1_noise", "",
		"Noise threshold for fitting baseline T1"); //!< See initial value
	mdm_input_int nT1Inputs = mdm_input_int(
		0, "n_T1", "",
		"Number of input signals for baseline T1 mapping"); //!< See initial value
  mdm_input_string B1Name = mdm_input_string(
    mdm_input_str(""), "B1", "",
    "Path to B1 correction map"); //!< See initial value
  mdm_input_double B1Scaling = mdm_input_double(
    1000.0, "B1_scaling", "",
    "Scaling value appplied to B1 map"); //!< See initial value

	//Signal to concentration options
	mdm_input_bool M0Ratio = mdm_input_bool(
		true, "M0_ratio", "",
		"Flag to use ratio method to scale signal instead of precomputed M0"); //!< See initial value
	mdm_input_string T1Name = mdm_input_string(
		mdm_input_str(""), "T1", "",
		"Path to precomputed T1 map"); //!< See initial value
	mdm_input_string M0Name = mdm_input_string(
		mdm_input_str(""), "M0", "",
		"Path to precomputed M0 map"); //!< See initial value
	mdm_input_double r1Const = mdm_input_double(
		3.4, "r1", "", "Relaxivity constant of concentration in tissue"); //!< See initial value
  mdm_input_bool B1Correction = mdm_input_bool(
    false, "B1_correction", "",
    "Flag to use B1 correction on FA values to scale signal instead of precomputed M0"); //!< See initial value

	//AIF options
  mdm_input_int aifType = mdm_input_int(
    0, "aif_type", "",
    "AIF type, overriden by --aif and --aif_map"); //!< See initial value
	mdm_input_string aifName = mdm_input_string(
		mdm_input_str(""), "aif", "",
		"Path to precomputed AIF, if not set uses population AIF"); //!< See initial value
  mdm_input_string aifMap = mdm_input_string(
    mdm_input_str(""), "aif_map", "", "Map of voxels to average in AIF computation"); //!< See initial value
	mdm_input_string pifName = mdm_input_string(
		mdm_input_str(""), "pif", "",
		"Path to precomputed AIF, if not set derives from AIF"); //!< See initial value
	mdm_input_double dose = mdm_input_double(
		0.1, "dose", "D", "Contrast-agent dose"); //!< See initial value
	mdm_input_double hct = mdm_input_double(
		0.42, "hct", "H", "Haematocrit level"); //!< See initial value

	//Model options
	mdm_input_string model = mdm_input_string(
		mdm_input_str(""), "model", "m",
		"Tracer-kinetic model"); //!< See initial value
	mdm_input_doubles initialParams = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{}), "init_params", "",
		"Initial values for model parameters to be optimised"); //!< See initial value
	mdm_input_string initMapsDir = mdm_input_string(
		mdm_input_str(""), "init_maps", "",
		"Path to folder containing parameter maps for per-voxel initialisation"); //!< See initial value
  mdm_input_string modelResiduals = mdm_input_string(
    mdm_input_str(""), "residuals", "",
    "Path to model residuals map as a target threshold for new fits"); //!< See initial value
	mdm_input_ints initMapParams = mdm_input_ints(
		mdm_input_int_list(std::vector<int>{}), "init_map_params", "",
		"Index of parameters sampled from maps"); //!< See initial value
	mdm_input_strings paramNames = mdm_input_strings(
		mdm_input_string_list(std::vector<std::string>{}), "param_names", "",
		"Names of model parameters, used to override default output map names"); //!< See initial value
	mdm_input_ints fixedParams = mdm_input_ints(
		mdm_input_int_list(std::vector<int>{}), "fixed_params", "",
		"Index of parameters fixed to their initial values"); //!< See initial value
	mdm_input_doubles fixedValues = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{}), "fixed_values", "",
		"Values for fixed parameters"); //!< See initial value
	mdm_input_ints relativeLimitParams = mdm_input_ints(
		mdm_input_int_list(std::vector<int>{}), "relative_limit_params", "",
		"Index of parameters to which relative limits are applied"); //!< See initial value
	mdm_input_doubles relativeLimitValues = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{}), "relative_limit_values", "",
		"Values for relative limits - optimiser bounded to range initParam +/- relLimit"); //!< See initial value
	mdm_input_int firstImage = mdm_input_int(
		0, "first", "",
		"First image used in model fit cost function"); //!< See initial value
	mdm_input_int lastImage = mdm_input_int(
		0, "last", "",
		"Last image used in model fit cost function"); //!< See initial value

	mdm_input_bool noOptimise = mdm_input_bool(
		false, "no_opt", "",
		"Flag to turn-off optimisation, default false"); //!< See initial value
	mdm_input_bool dynNoise = mdm_input_bool(
		false, "dyn_noise", "",
		"Flag to use varying temporal noise in model fit, default false"); //!< See initial value
	mdm_input_bool testEnhancement = mdm_input_bool(
		true, "test_enh", "",
		"Flag to test for enhancement before fitting model, default true"); //!< See initial value
	mdm_input_int maxIterations = mdm_input_int(
		0, "max_iter", "",
		"Max iterations per voxel in optimisation - 0 for no limit"); //!< See initial value

	//DCE only output options
	mdm_input_bool outputCt_sig = mdm_input_bool(
		false, "Ct_sig", "",
		"Flag to save signal-derived dynamic concentration maps"); //!< See initial value
	mdm_input_bool outputCt_mod = mdm_input_bool(
		false, "Ct_mod", "",
		"Flag to save modelled dynamic concentration maps"); //!< See initial value
	mdm_input_doubles IAUCTimes = mdm_input_doubles(
		mdm_input_double_list(std::vector<double>{ 60.0,90.0,120.0 }), "iauc", "I",
		"Times (in s, post-bolus injection) at which to compute IAUC"); //!< See initial value
	mdm_input_bool IAUCAtPeak = mdm_input_bool(
		false, "iauc_peak", "",
		"Flag to compute IAUC at peak signal"); //!< See initial value

  //AIF detection
  mdm_input_ints aifSlices = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "aif_slices", "",
    "Slices used to automatically measure AIF"); //!< See initial value
  mdm_input_ints aifXrange = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "aif_x_range", "",
    "Range of voxels to consider as AIF candidates in x-axis"); //!< See initial value
  mdm_input_ints aifYrange = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "aif_y_range", "",
    "Range of voxels to consider as AIF candidates in y-axis"); //!< See initial value
  mdm_input_double minT1Blood = mdm_input_double(
    1000, "min_T1_blood", "", "Minimum T1 to be considered as potential blood voxel"); //!< See initial value
  mdm_input_double peakTime = mdm_input_double(
    1.0, "peak_time", "", "Time window post bolus for peak to arrive"); //!< See initial value
  mdm_input_double prebolusNoise = mdm_input_double(
    0, "prebolus_noise", "", "Estimate of noise on the prebolus signal used when unable to compute"); //!< See initial value
  mdm_input_int prebolusMinImages = mdm_input_int(
    5, "prebolus_min_images", "", "Minimum number of images required to estimate prebolus noise"); //!< See initial value
  mdm_input_double selectPct = mdm_input_double(
    5, "select_pct", "", "Percentage of candidates to select"); //!< See initial value

	//General output options
	mdm_input_string outputRoot = mdm_input_string(
		mdm_input_str(""), "output_root", "", "Optional base for output folder"); //!< See initial value
	mdm_input_string outputDir = mdm_input_string(
		mdm_input_str(""), "output", "o", "Output folder"); //!< See initial value
	mdm_input_bool overwrite = mdm_input_bool(
		false, "overwrite", "",
		"Flag to overwrite existing analysis in output dir, default false"); //!< See initial value

  //Image format options
  mdm_input_string imageReadFormat = mdm_input_string(
    mdm_input_str("NIFTI"), "img_fmt_r", "",
    "Image format for reading input"); //!< See initial value
  mdm_input_string imageWriteFormat = mdm_input_string(
    mdm_input_str("NIFTI"), "img_fmt_w", "",
    "Image format for writing output"); //!< See initial value
  mdm_input_int imageDataType = mdm_input_int(
    16, "img_dt_type", "", "Data type of output images - see Analyze format type specifiers"); //!< See initial value

	//Logging options
  mdm_input_bool voxelSizeWarnOnly = mdm_input_bool(
    false, "voxel_size_warn_only", "",
    "Only throw a warning (instead of error) if input image voxel sizes do not match"); //!< See initial value
  mdm_input_bool noLog = mdm_input_bool(
    false, "no_log", "",
    "Switch off program logging"); //!< See initial value
  mdm_input_bool noAudit = mdm_input_bool(
    false, "no_audit", "",
    "Switch off audit logging"); //!< See initial value
  mdm_input_bool quiet = mdm_input_bool(
    false, "quiet", "q",
    "Do not display logging messages in cout"); //!< See initial value
	mdm_input_string programLogName = mdm_input_string(
		mdm_input_str("ProgramLog.txt"), "program_log", "",
		"Filename of program log, will be appended with datetime"); //!< See initial value
	mdm_input_string outputConfigFileName = mdm_input_string(
		mdm_input_str("config.txt"), "config_out", "",
		"Filename of output config file, will be appended with datetime"); //!< See initial value
	mdm_input_string auditLogBaseName = mdm_input_string(
		mdm_input_str("AuditLog.txt"), "audit", "",
		"Filename of audit log, will be appended with datetime"); //!< See initial value
	mdm_input_string auditLogDir = mdm_input_string(
		mdm_input_str("audit_logs/"), "audit_dir", "",
		"Folder in which audit output is saved"); //!< See initial value

	//Madym-lite only options
	mdm_input_string dynTimesFile = mdm_input_string(
		mdm_input_str(""), "times", "t",
		"Path to file containing dynamic times"); //!< See initial value
	mdm_input_string initParamsFile = mdm_input_string(
		mdm_input_str(""), "init_params_file", "",
		"Path to data file containing maps of parameters to initialise fit"); //!< See initial value
	mdm_input_string dynNoiseFile = mdm_input_string(
		mdm_input_str(""), "dyn_noise_file", "", ""); //!< See initial value

	//Lite options
	mdm_input_string inputDataFile = mdm_input_string(
		mdm_input_str(""), "data", "", "Input data filename, see notes for options"); //!< See initial value
	mdm_input_double FA = mdm_input_double(
		0, "FA", "", "FA of dynamic series"); //!< See initial value
  mdm_input_doubles VFAs = mdm_input_doubles(
    mdm_input_double_list(std::vector<double>{}), "VFAs", "", "FA of dynamic series"); //!< See initial value
	mdm_input_double TR = mdm_input_double(
		0, "TR", "", "TR of dynamic series"); //!< See initial value
	mdm_input_string outputName = mdm_input_string(
		mdm_input_str("madym_analysis.dat"), "output_name", "O", "Name of output data file"); //!< See initial value

  //Dicom options - data locations
  mdm_input_string dicomDir = mdm_input_string(
    mdm_input_str(""), "dicom_dir", "",
    "Folder containing DICOM data"); //!< See initial value
  mdm_input_string dicomSeriesFile = mdm_input_string(
    mdm_input_str("DCM_series"), "dicom_series", "", 
    "Filename to/from which dicom series data is written/read"); //!< See initial value
  
  mdm_input_ints T1inputSeries = mdm_input_ints(
    mdm_input_int_list(std::vector<int>{}), "T1_series", "",
    "Indices of the dicom series for each T1 input"); //!< See initial value
  mdm_input_int dynSeries = mdm_input_int(
    0, "dyn_series", "",
    "Index of the dicom series for the dynamic DCE time-series"); //!< See initial value
  mdm_input_int singleSeries = mdm_input_int(
    0, "single_series", "",
    "Index of the dicom series for converting a generic single volume"); //!< See initial value
  mdm_input_string dicomFileFilter = mdm_input_string(
    mdm_input_str(""), "dicom_filter", "",
    "File filter for dicom sort (eg IM_)"); //!< See initial value
  mdm_input_string volumeName = mdm_input_string(
    mdm_input_str(""), "vol_name", "",
    "Output filename for converting a single dicom volume"); //!< See initial value

  //Dicom options -flags
  mdm_input_bool dicomSort = mdm_input_bool(
    false, "sort", "",
    "Sort the files in Dicom dir into separate series, writing out the series information"); //!< See initial value
  mdm_input_bool makeT1Inputs = mdm_input_bool(
    false, "make_t1", "",
    "Make T1 input images from dicom series"); //!< See initial value
  mdm_input_bool makeSingle = mdm_input_bool(
    false, "make_single", "",
    "Make single 3D image from dicom series"); //!< See initial value
  mdm_input_bool makeDyn = mdm_input_bool(
    false, "make_dyn", "",
    "Make dynamic images from dynamic series"); //!< See initial value
  mdm_input_bool makeT1Means = mdm_input_bool(
    false, "make_t1_means", "",
    "Make mean of each set of T1 input repeats"); //!< See initial value
  mdm_input_bool makeDynMean = mdm_input_bool(
    false, "make_dyn_mean", "",
    "Make temporal mean of dynamic images"); //!< See initial value
  mdm_input_bool flipX = mdm_input_bool(
    false, "flip_x", "",
    "Flip dicom slices horizontally before copying into 3D image volume"); //!< See initial value
  mdm_input_bool flipY = mdm_input_bool(
    true, "flip_y", "",
    "Flip dicom slices vertically before copying into 3D image volume"); //!< See initial value

  //Dicom options - scaling
  mdm_input_string autoScaleTag = mdm_input_string(
    mdm_input_str("0x2005,0x100e"), 
    "scale_tag", "",
    "Dicom tag key (group,element) for rescale slope, in hexideciaml form - for Philips this is (0x2005, 0x100e)"); //!< See initial value
  mdm_input_string autoOffsetTag = mdm_input_string(
    mdm_input_str("0x2005,0x100d"),
    "offset_tag", "",
    "Dicom tag key (group,element) for rescale intercept, in hexideciaml form - for Philips this is (0x2005, 0x100d)"); //!< See initial value

  mdm_input_double dicomScale = mdm_input_double(
    1.0, "dicom_scale", "", "Additional scaling factor applied to the dicom data"); //!< See initial value
  mdm_input_double dicomOffset = mdm_input_double(
    0, "dicom_offset", "", "Additional offset factor applied to the dicom data"); //!< See initial value

  //DICOM -acquisition time
  mdm_input_string dynTimeTag = mdm_input_string(
    mdm_input_str(""),
    "acquisition_time_tag", "",
    "Dicom tag key (group,element) for acquisition time, if empty uses DCM_AcquisitionTime"); //!< See initial value
  mdm_input_double temporalResolution = mdm_input_double(
    0, "temp_res", "", 
    "Time in seconds between volumes in the DCE sequence, used to fill acquisition time not set in dynTimeTag"); //!< See initial value

  //DICOM - naming
  mdm_input_string repeatPrefix = mdm_input_string(
    mdm_input_str("rpt_"),
    "repeat_prefix", "",
    "Prefix of image name for repeats in DICOM series, appended with sequence_format index and stored in series name folder"); //!< See initial value

  mdm_input_string meanSuffix = mdm_input_string(
    mdm_input_str("_mean"),
    "mean_suffix", "",
    "Suffix of image name for mean of repeats in DICOM series, appended to series name"); //!< See initial value


};

#endif /* mdm_RunTools_HDR */
