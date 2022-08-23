'''
This script shows how to use the data loading helper functions in QbiPy
to load in time-series for all voxels in an ROI, then uses 
madym_DCE_lite to fit to the average time-series.

This script can either be run interactively or as regular python script
from the command line, in which case the first argument should be
path to the folder containing the Madym examples data.

IMPORTANT: To run this script, please first run the run_examples_test_dataset.py
script. This will check Madym is installed properly on your system, and we 
will use the outputs of that script here.
'''
#%%
import os
import sys
import numpy as np
import matplotlib.pyplot as plt

from QbiPy.dce_models import data_io
from QbiMadym import madym_DCE_lite


#%%
#Set path to the dataset - set this to the examples folder in which the
#test_dataset and config files are stored. If you call this script from the command
#line you can pass this as the first argument
if len(sys.argv) > 0:
    config_dir = sys.argv[1]
else:
    config_dir = ''

#%% ------------------------------------------------------------------------
##
#Setup paths to data, we're loading signal-derived concentration time-series
#from the output of the examples script ETM fit using an auto-generated
#AIF. An alternative would be to load in the signal time-series and
#convert these to signal (see eg using_madym_lite.py for using 
# QbiPy to convert between signal and concentration), but for simplicity
# we'll using concentration time-series here 
data_dir = os.path.join(config_dir, 'test_dataset')
root_path = os.path.join(data_dir, 'madym_output', 'ETM_auto', 'Ct_sig', 'Ct_sig')
roi_path = os.path.join(data_dir, 'roi', 'tumour.hdr')
n_images = 75

#Use get_dyn_vals to load signal time series from roi into a 2D array
C_t = data_io.get_dyn_vals(root_path, n_images, roi_path)

#Compute the mean over the samples
C_t_avg = np.mean(C_t, axis = 0)

#Set-up an array of dynamic times - we could read this from the dynamic series
#xtr files, but for this script we'll just set them up using linpsace.
t = np.linspace(0, 6.13698, n_images) #units:minutes

params, model_fit, _, _, C_t_mod, _ = madym_DCE_lite.run(
    model = 'ETM',
    input_data = C_t_avg,
    input_Ct = 1,
    dyn_times = t,
    injection_image = 7)

plt.figure()
plt.plot(t, C_t_avg, 'b-', linewidth=3)
plt.plot(t, C_t_mod[0,:], 'r', linewidth=3)
plt.xlabel('Time (mins)')
plt.ylabel('CA concentration')
plt.title('ETM fit to average time-series of examples dataset')
plt.legend(['Avg. C(t)', 'Model fit'])

#%%
print(f'Model residual = {model_fit[0]:4.3f}')
for i_p, param_name in enumerate(['Ktrans', 'v_e', 'v_p', 'tau_a']):
    print(f'{param_name} = {params[0,i_p]:4.3f}')
# %%
