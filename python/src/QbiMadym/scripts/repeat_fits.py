'''
This script tests the fetaure introduce in  Madym 4.21.0 to allow repeat fits
of parameter in madym_DCE PK model fitting.

The PK model is fitted at each of the repeat values specified, with the
fit that returns the lowest residual retained, allowing a grid search over
a single parameter.

This is particularly useful for performing grid-searches over the
arterial delay parameter when using linear-least-squares fitting of
the extended-Tofts model, and this is the use case we demonstrate here.
'''
#%%
%load_ext autoreload

%autoreload 2

import os
import numpy as np
import matplotlib.pyplot as plt

from QbiPy.dce_models import data_io
from QbiPy.image_io.analyze_format import read_analyze
from QbiMadym import madym_DCE_lite


#%%
#Set path to the dataset - set this to the examples folder in which the
#test_dataset and config files are stored. Note if this path contains
#spaces in your installation.
config_dir = ''

#%% ------------------------------------------------------------------------
## 1) How to run the examples using the config files in the dataset
data_dir = os.path.join(config_dir, 'test_dataset')
root_path = os.path.join(data_dir, 'dynamic', 'dyn_')
roi_path = os.path.join(data_dir, 'roi', 'tumour.hdr')
T1_path = os.path.join(data_dir, 'madym_output', 'T1', 'T1.hdr')

S_t = data_io.get_dyn_vals(root_path, 75, roi_path)

t = np.linspace(0, 6, 75) 
TR = 4.0
FA = 20.0
#%%
roi = read_analyze(roi_path)[0] > 0
T1 = read_analyze(T1_path)[0][roi]
#%%
delay_times = [0, 0.1, 0.2, 0.3, 0.4, 0.5]
n_samples = T1.size
best_params = np.zeros((n_samples, 4))
best_model_fits = np.full((n_samples), np.Inf)
for delay_time in delay_times:
    model_params, model_fit = madym_DCE_lite.run(
        model = 'ETM',
        input_data = S_t,
        input_Ct = 0,
        dyn_times = t,
        T1 = T1,
        FA = FA,
        TR = TR,
        M0_ratio = 1,
        injection_image = 7,
        r1_const = 3.4,
        opt_type='LLS',
        fixed_params=[4],
        fixed_values=[delay_time]
        )[0:2]

    swap = model_fit < best_model_fits
    best_params[swap,:] = model_params[swap,:]
    best_model_fits[swap] = model_fit[swap]

#%%
model_params, model_fit = madym_DCE_lite.run(
    model = 'ETM',
    input_data = S_t,
    input_Ct = 0,
    dyn_times = t,
    T1 = T1,
    FA = FA,
    TR = TR,
    M0_ratio = 1,
    injection_image = 7,
    r1_const = 3.4,
    opt_type='LLS',
    repeat_param = 4,
    repeat_values = delay_times
    )[0:2]

# %%
plt.figure()
plt.plot(t, np.mean(S_t, axis=0))
# %%
