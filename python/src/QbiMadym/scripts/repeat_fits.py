'''
This script tests the fetaure introduce in  Madym 4.21.0 to allow repeat fits
of parameter in madym_DCE PK model fitting.

The PK model is fitted at each of the repeat values specified, with the
fit that returns the lowest residual retained, allowing a grid search over
a single parameter.

This is particularly useful for performing grid-searches over the
arterial delay parameter when using linear-least-squares fitting of
the extended-Tofts model, and this is the use case we demonstrate here.

IMPORTANT: To run this script, please first run the run_examples_test_dataset.py
script. This will check Madym is installed properly on your system, and we 
will use some of the outputs of that script here.
'''
#%%
import os
import numpy as np
import matplotlib.pyplot as plt

from QbiPy.dce_models import data_io
from QbiPy.image_io.analyze_format import read_analyze
from QbiMadym import madym_DCE_lite


#%%
#Set path to the dataset - set this to the examples folder in which the
#test_dataset and config files are stored.
config_dir = ''

#%% ------------------------------------------------------------------------
## We're going to use madym_DCE_lite to compute model fits directly on numpy
# arrays. So first we're going to use some of the helper functions in QbiPy
# to load in the data we want to fit.

#Setup paths to data
data_dir = os.path.join(config_dir, 'test_dataset')
root_path = os.path.join(data_dir, 'dynamic', 'dyn_')
roi_path = os.path.join(data_dir, 'roi', 'tumour.hdr')
T1_path = os.path.join(data_dir, 'madym_output', 'T1', 'T1.hdr')
n_images = 75

#Use get_dyn_vals to load signal time series from roi into a 2D array
S_t = data_io.get_dyn_vals(root_path, n_images, roi_path)

#Read in T1 values (computed from running run_examples_test_dataset.py)
roi = read_analyze(roi_path)[0] > 0
T1 = read_analyze(T1_path)[0][roi]

#Set-up an array of dynamic times - we could read this from the dynamic series
#xtr files, but for this script we'll just set them up using linpsace.
t = np.linspace(0, 6.13698, n_images) #units:minutes

#Specify other scanner parameters
TR = 4.0
FA = 20.0
injection_image = 7,
r1_const = 3.4

#%% ------------------------------------------------------------------------
# Next we'll perform linear-least squares of an extended-Tofts model
# at multiple fixed values of the arterial delay parameter tau, using
# python code to choose the best
delay_times = [0, 0.1, 0.2, 0.3, 0.4, 0.5] #units: minutes
n_samples = T1.size
n_delays = len(delay_times)

#Set up containers to store the parameters and model fits (we could also
# implement this by storing just the best fits/parameters
# and swapping in fits/params for samples that improve  on each iteration)
all_model_params = np.zeros((n_samples, 4, n_delays))
all_model_fits = np.full((n_samples, n_delays), np.Inf)

#Call madym_DCE_lite.run at each delay time - we only want the first two ouputs:
#the model parameters (n_samples, 4) and model fits (n_samples,)

for i_d,delay_time in enumerate(delay_times):
    all_model_params[:,:,i_d], all_model_fits[:,i_d] = madym_DCE_lite.run(
        model = 'ETM',
        input_data = S_t,
        input_Ct = 0,
        dyn_times = t,
        T1 = T1,
        FA = FA,
        TR = TR,
        M0_ratio = 1,
        injection_image = injection_image,
        r1_const = r1_const,
        opt_type='LLS', #Selects linear least-square sfitting
        fixed_params=[4], #tau is the 4th parameter in the ETM
        fixed_values=[delay_time] #fix to the given delay time
        )[0:2]

#Compute the best fits using the argmin of the model fits 
best_model_fits = np.min(all_model_fits, axis=1)
best_idx = np.argmin(all_model_fits, axis = 1)
best_model_params = np.empty((n_samples,4))
for i, best_delay in enumerate(best_idx):
    best_model_params[i, :] = all_model_params[i,:,best_delay]

#%% ------------------------------------------------------------------------
# Now we'll use the new feature (since Madym v4.21.0) that allows to loop
# over values of a single parameter directly in a single call to madym
model_params, model_fit = madym_DCE_lite.run(
    model = 'ETM',
    input_data = S_t,
    input_Ct = 0,
    dyn_times = t,
    T1 = T1,
    FA = FA,
    TR = TR,
    M0_ratio = 1,
    injection_image = injection_image,
    r1_const = r1_const,
    opt_type='LLS',
    repeat_param = 4, #specify tau as the repeat parameter
    repeat_values = delay_times #Pass in the array of delay times
    )[0:2]

#Confirm that we obtain the same fits, max_diff should be zero
max_diff = np.max(np.abs(model_params - best_model_params))
print(f'Max difference between python loop and --repeat_params option = {max_diff}')

# %%
# Plot the fitted parameters
param_names = ['$K^{trans}$', '$v_e$', '$v_p$', '$\\tau$']
fig, ax = plt.subplots(2,2)
fig.set_size_inches((8,8))
fig.set_tight_layout(True)
for i_p in range(4):
    row = i_p // 2
    col = i_p % 2
    ax[row,col].plot(best_model_params[:,i_p], model_params[:,i_p], 'r.')
    ax[row,col].set_aspect('equal')
    ax[row,col].set_xlabel('Fitting using python loop')
    ax[row,col].set_ylabel('Fitting using --repeat_param option')
    ax[row,col].set_title(param_names[i_p])

# %%
