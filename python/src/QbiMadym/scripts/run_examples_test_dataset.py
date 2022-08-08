''' ------------------------------------------------------------------------
# This script shows how to recreate the analysis test examples dataset 
# inlcuded with the main Madym C++ toolkit (described here:
# https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/example_scripts)
# using the python wrappers.
#
# It assumes you have already set your LOCAL_MADYM_ROOT, and that you have
# the examples dataset downloaded and copied to a location where you have 
# write access.
#
# We show:
# 1) How to run the examples using the config files in the dataset
# 2) How to override some of the config options
# 3) How to run the examples without the config files, setting all options
# via the python wrapper
 ------------------------------------------------------------------------'''
#%%
%load_ext autoreload

%autoreload 2

from QbiMadym import madym_DCE, madym_T1, madym_AIF

#Set path to the dataset - set this to the examples folder in which the
#test_dataset and config files are stored. Note if this path contains
#spaces in your installation. Alternatively leave data_dir empty and cd
#into your examples folder
data_dir = ''

#%% ------------------------------------------------------------------------
## 1) How to run the examples using the config files in the dataset

## 1.1 Fit T1
madym_T1.run(
    config_file = data_dir +'madym_T1_config.txt',
    working_directory = data_dir +'test_dataset')
#%% 1.2 Detect AIF
madym_AIF.run(
    config_file = data_dir +'madym_AIF_config.txt',
    working_directory = data_dir +'test_dataset')

#%% 1.3 Fit ETM
madym_DCE.run(
    config_file = data_dir +'madym_ETM_auto_config.txt',
    working_directory = data_dir +'test_dataset')

#%% ------------------------------------------------------------------------
## 2) How to override some of the config options
# We now show how to run using a config, but overriding one of the options,
# for example, fitting the 2CXM instead of the extended-Tofts model. We can
# do this for as many of the options as we like

#%% 2.1 fit 2CXM instead of ETM, also updating the output dir
madym_DCE.run(
    config_file = data_dir +'madym_ETM_auto_config.txt',
    working_directory = data_dir +'test_dataset',
    model = '2CXM',
    output_dir = 'madym_output/2CXM_auto')

#%% 2.2 Fit the ETM, but fix Vp (the 3rd parameter) to 0, to fit a TM
madym_DCE.run(
    config_file = data_dir +'madym_ETM_auto_config.txt',
    working_directory = data_dir +'test_dataset',
    fixed_params = [3],
    fixed_values = [0],
    output_dir = 'madym_output/TM_auto')

#%% ------------------------------------------------------------------------
## 3) How to run the examples without the config files
# In this example we show how to run the analysis that would be performed
# by calling the madym_ETM_map_config.txt config file, but instead sets all
# options via the python wrapper
madym_DCE.run(
    working_directory = data_dir +'test_dataset',
    model = 'ETM',
    output_dir = 'madym_output/ETM_map',
    output_Ct_mod = 1,
    output_Ct_sig = 1,
    M0_ratio = 1,
    T1_name = 'madym_output/T1/T1.hdr',
    aif_map = 'madym_output/AIF/slice_0-4_Auto_AIF.hdr',
    dynamic_basename = 'dynamic/dyn_',
    img_fmt_r = 'ANALYZE',
    img_fmt_w = 'ANALYZE',
    injection_image = 7,
    r1_const = 3.4,
    roi_name = 'roi/tumour.hdr',
    error_name = 'madym_output/T1/error_tracker.hdr',
    no_audit = 1,
    overwrite = 1)