'''
--------------------------------------------------------------------------
 This script provides a quick demonstration of fitting signals generated 
 for diffusion IVIM model using the Matlab wrapper to Madym's DWI tools

 The example shows running a fit at several noise levels for a fixed set
 of IVIM ground truth parameters, however this basic simulation may easily
 be extended to larger sets of more variable inputs, or with different 
 B-values etc.
'''
#%%
import numpy as np
import matplotlib.pyplot as plt

from QbiMadym import madym_DWI_lite

#%%
def IVIM_simulation(n_samples, B_vals, S0, D, f, D_star, sigma):

    #Generate simulated IVIM test data with Rician noise added
    S0s = np.full((n_samples,1), S0)
    Ds = np.full((n_samples,1), D)
    fs = np.full((n_samples,1), f)
    D_stars = np.full((n_samples,1), D_star)

    signals = madym_DWI_lite.IVIM_model(B_vals, S0s, Ds, fs, D_stars)
    signals_n = madym_DWI_lite.add_rician_noise(signals, sigma)
    print('Shape of signals = ', signals_n.shape)

    #Fit IVIM using Madym
    Bvals_thresh = [40.0,60.0,100.0,150.0]
    model_params = madym_DWI_lite.run(
        'IVIM', signals_n, B_vals,
        Bvals_thresh=Bvals_thresh)[0]

    #Plot the output
    param_names = ['S_0', 'D', 'f', 'D^*']
    param_units = ['a.u.', 'mm^2/s', 'no units', 'mm^2/s']
    gt = [S0, D, f, D_star]

    n_bins = round(n_samples / 100)

    fig, ax = plt.subplots(2, 2, figsize=[12,12], constrained_layout = True)
    plt.suptitle(f'Rician noise sigma = {sigma}')
    
    for i_p in range(4):
        row = i_p // 2
        col = i_p % 2

        [counts, bins] = np.histogram(model_params[:,i_p], n_bins)
        bin_width = bins[1] - bins[0]
        med_p = np.median(model_params[:,i_p])
        max_count = np.max(counts)
        ax[row, col].bar(bins[0:-1], counts, width = 0.9*bin_width)
        ax[row, col].plot([gt[i_p], gt[i_p]], [0, max_count], 'g-', linewidth=2)
        ax[row, col].plot([med_p, med_p], [0, max_count], 'r--', linewidth=2)

        ax[row, col].legend([
            'Distribution of fitted parameters',
            'Ground truth',
            'Median of fitted parameters'])

        ax[row, col].set_xlabel(f'{param_names[i_p]},  {param_units[i_p]}')
        ax[row, col].set_ylabel(f'Histogram counts per {n_samples}')
        ax[row, col].set_title(f'Fitting {param_names[i_p]} = {gt[i_p]:5.4f}: median fit = {med_p:5.4f}')

#%%
#Set B-values at which to simulate signal inputs
B_vals = np.array([0.0, 20.0, 40.0, 60.0, 80.0, 100.0, 300.0, 500.0, 800.0]).reshape((1,-1))

#Set ground truth IVIM parameters
S0 = 100
D = 0.8e-3
f = 0.2
D_star = 15e-3
n_samples = int(1e4)
#%%
for sigma in [0.1, 1, 2, 4, 10]:
    IVIM_simulation(n_samples, B_vals, S0, D, f, D_star, sigma)



    

    
    
    
    
    

# %%
