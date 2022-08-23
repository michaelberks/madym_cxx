'''
This script shows a comparison between linera and non-linear least-squares 
fitting of the extended-Tofts and 2-compartent exchange models.

This also shows how to use the tracer-kinetic model modules in QbiPy
to create simulated test data for Monte-Carlo type experiments.

For the ETM we test both with and without fitting the arterial delay parameter.

Note the NLLS algorithm Madym uses is Alglib's BLEIC:

This script can either be run interactively or as regular python script
from the command line.

'''
#%%
# Some iPython automagic commands below. Comment out if not using
# as an interactive script
#%load_ext autoreload
#
#%autoreload 2
#%%
import numpy as np
import matplotlib.pyplot as plt
import time
#%%
from QbiMadym import madym_DCE_lite
from QbiPy.dce_models import dce_aif,tofts_model,two_cxm_model

# %%
def display_fit_errors(
    model, param_name, noise_std, param_gt, param_idx, fitted_params, fit_time):
    '''
    Create scatter plots of estimates parameters vs ground truth for both
    linear and non-linear fits of a TK model

    Inputs:
        model:str 
            Name of the TK model as defined in Madym (eg ETM or 2CXM)  
        param_name:str
            Names of each parameter
        noise_std:float
            Sigma of random Gaussian noise added to Ct
        param_gt:np.array (n_samples,)
            Array of ground truth parameters, 1 sample per row 
        param_idx:int
            Column index of parameter to slice fitted parameters array
        fitted_params:dict
            Fields {'LLS', 'BLEIC'}, each containing (n_samples, n_params) array
            of fitted parameters. For each fit_type, 
            fitted_params[fit_type][:,param_idx] is compared with param_gt
        fit_time:dict
            Fields {'LLS', 'BLEIC'}, the total fit time for each method in seconds
    '''

    #Get fitted parameters and fit time for linear and non-linear fits
    param_lls = fitted_params['LLS'][:,param_idx]
    param_nlls = fitted_params['BLEIC'][:,param_idx]
    LLS_t = fit_time['LLS']
    NLLS_t = fit_time['BLEIC']
    
    #Compute the median absolute error between the fitted and ground-truth
    #parameters
    LLS_err = np.median(np.abs(param_gt - param_lls))
    NLLS_err = np.median(np.abs(param_gt - param_nlls))

    #Get parameter limits of ground truth to set plot limits
    min_param = np.min(param_gt)
    max_param = np.max(param_gt)

    #Create scatter plots of ground-truth vs estimates for each fit type
    plt.figure(figsize=(12,6))
    plt.suptitle(f'{model}: {param_name}, noise = {noise_std}')
    plt.subplot(1,2,1)
    plt.plot(param_gt, param_lls, 'r.')
    plt.plot([min_param,max_param],[min_param,max_param], 'k--')
    plt.axis('equal')
    plt.axis((min_param,max_param,min_param,max_param))
    plt.title(f'LLS err = {LLS_err:5.4f}, fitting time = {LLS_t:5.4f}')

    plt.subplot(1,2,2)
    plt.plot(param_gt, param_nlls, 'r.')
    plt.plot([min_param,max_param],[min_param,max_param], 'k--')
    plt.axis('equal')
    plt.axis((min_param,max_param,min_param,max_param))
    plt.title(f'NLLS err = {NLLS_err:5.4f}, fitting time = {NLLS_t:5.4f}')
    plt.show()

# %%
def run_MC(model, params_gt, param_names, Ct, t, noise_std, 
    fixed_params = None, fixed_values = None,
    repeat_param = None, repeat_values = None):
    '''
    Given simulated contrast-agent concentration time-series with
    fit a TK model using both linear and non-linear least squares fitting 
    and compare the estimated TK parameters with
    the known ground truth.

    Inputs:
        model:str 
            Name of the TK model as defined in Madym (eg ETM or 2CXM)  
        params_gt:np.array (n_samples, n_params)
            Array of ground truth parameters, 1 sample per row, 1 param per column 
        param_names:list(str)
            Names of each parameter for display
        Ct:np.array (n_samples, n_times)
            Noise free concentration time-series, 1 time-series per row
        t:np.array (n_times,)
            Array of times (in minutes) associated with each timepoint in time-series
        noise_std:float
            Sigma of random Gaussian noise added to Ct (Rician noise would be better,
            but this just an example simulation to show functionality)
        fixed_params:list(int) = None
            Indices of parameters fixed (ie not-optimised) in the model 
        fixed_values:list(float) = None,
            Values of each fixed parameter, 1 value per fixed parameter
        repeat_param:int = None
            Index of parameter fitted at multiple fixed values 
        repeat_values:list(float) = None
            Set of values at which repeat param is fixed. The best fit will be selected.

    Outputs:
        None, instead the scatter plots of ground truth vs estimated are shown
        for each parameter and fit type
    '''

    #Add Gaussian noise at given sigma to the concentration time-series
    Ct_n = Ct + np.random.randn(*Ct.shape)*noise_std
    
    #Use madym_DCE_lite to fit model for either non-linear (BLEIC) or linear (LLS)
    # fitting 
    fitted_params = dict()
    fit_time = dict()
    for opt_type in {'BLEIC', 'LLS'}:

        #Only apply repeat parameter fitting for LLS
        if opt_type == 'BLEIC':
            rp = None
            rv = None
        else:
            rp = repeat_param
            rv = repeat_values
    
        #Start timer then call madym_DCE_lite.run, we only want the first output
        #which is the fitted parameters
        start_time = time.time()
        fitted_params[opt_type] = madym_DCE_lite.run(
            model=model, #Name of TK model
            input_data = Ct_n, #Noisy concentration time-series
            dyn_times = t, #Dynamic times
            hct = 0, #Don't apply haematocrit correction
            fixed_params = fixed_params, 
            fixed_values = fixed_values,
            repeat_param = rp, 
            repeat_values = rv, 
            quiet = 1,
            opt_type = opt_type)[0]

        #Get total compute time
        elapsed_time = time.time() - start_time
        fit_time[opt_type] = elapsed_time

    #Display the fit errors for ecah parameter
    for i_p, (param_gt, param_name) in enumerate(zip(params_gt, param_names)):
        display_fit_errors(model, param_name, noise_std, param_gt, 
            i_p, fitted_params, fit_time)

#%% ----------------------------------------------------------------------
'''
Sample parameters for the extended-Tofts Model
'''

#First create an array of dynamic times (here 51 times from 0 to 5 minutes
# inclusive, giving a temporal resolution of 6s)
t = np.linspace(0, 5, 51)

#Create a population AIF using QbiPy's Aif class
aif = dce_aif.Aif(times = t)

#Randomly sample a set of TK parameters with uniform distribution, U(a,b)
n_samples = int(1e3)
Ktrans = np.random.rand(n_samples)*0.2 + 0.1 # U(0.1, 0.3)
v_e = np.random.rand(n_samples)*0.2 + 0.1 # U(0.1, 0.3)
v_p = np.random.rand(n_samples)*0.1 + 0.05 # U(0.05, 0.15)
tau_a = np.random.rand(n_samples)*0.5 # U(0.0, 0.5)

#%% ----------------------------------------------------------------------
'''
For the first test, create ETM time-series with zero arterial delay.
Note that the fits are very, very similar, but LLS is an order of magnitude
faster to fit.
'''

#Use QbiPy tofts_model to get concentration time-series from
#model parameters
Ct = tofts_model.concentration_from_model(aif, Ktrans, v_e, v_p, 0)

#Perform fits at a set of increasing noise values
for noise in [1/100, 1/50, 1/25, 1/10]:
    run_MC('ETM', (Ktrans, v_e, v_p), ('Ktrans', 'v_e', 'v_p'), 
        Ct, t, noise,
        fixed_params = [4], #Fix tau_a to zero for both LLS + NLLS fits
        fixed_values = [0])

#%% ----------------------------------------------------------------------
'''
For the second test, create ETM time-series with the randomly sampled
arterial delay times tau_a. For the LLS, we will fit tau_a at a set of
discretely sampled repeat values, choosing the best fit. For NLLS, we
fit tau_a as a free parameter.

Note how even with the additional overhead of repeating the fit 6 times,
LLS fitting is still an order magnitude faster as NLLS now has to fit 4
parameters.

The fits errors are similar, with some parameters fitting better with
LLS and some better with NLLS, depending on the noise-level.

Ultimately more sophisticated Monte-Carlo simulations are required to
determine which provides optimal fit (where fit time isn't a consideration),
but for fitting volumetric data (where we fit millions of voxels and time
very much is a consideration), these data would suggest an LLS fit with a
grid search over tau_a provides adequate accuracy at a fraction of the
computational cost.
'''
Ct = tofts_model.concentration_from_model(aif, Ktrans, v_e, v_p, tau_a)

for noise in [1/100, 1/50, 1/25, 1/10]:
    run_MC('ETM', (Ktrans, v_e, v_p, tau_a), ('Ktrans', 'v_e', 'v_p', 'tau_a'), 
        Ct, t, noise,
        repeat_param = 4, repeat_values = [0.0,0.1,0.2,0.3,0.4,0.5])

#%%
'''
For the third test we compare fits of the two-compartment exchange model.
This has an extra parameter, separating flow rates from exchange rates.

Even with tau_a fixed, fits are generally much worse than the ETM, and
LLS fitting is noticeably worse than NLLS fitting. Again, formal
Monte-Carlo studies should be performed to properly establish the
magnitude of the difference in fit quality, but if you are fitting
the 2CXM (or similar bi-exponential models), we would suggest non-linear
fitting is the only safe option, despite the even greater computational
cost involved.
'''
#Sample new random parameters
F_p = np.random.rand(n_samples)*0.4 + 0.8 # U(0.8, 1.2)
PS = np.random.rand(n_samples)*0.2 + 0.1 # U(0.1, 0.3)
v_e = np.random.rand(n_samples)*0.2 + 0.1 # U(0.1, 0.3)
v_p = np.random.rand(n_samples)*0.1 + 0.05 # U(0.05, 0.15)

#%%
Ct = two_cxm_model.concentration_from_model(aif, F_p, PS, v_e, v_p, 0)

for noise in [1/1e6, 1/1000, 1/100, 1/50, 1/25]:
    run_MC('2CXM', (F_p, PS, v_e, v_p), ('F_p', 'PS', 'v_e', 'v_p'), 
        Ct, t, noise,
        fixed_params = [5], fixed_values = [0])

