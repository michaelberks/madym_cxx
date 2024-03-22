'''
This script is an accompaniment to the wiki blogbpost
on using Madym to perform DCE-MRI Monte-Carlo simulations.

It is still a work in progress...
'''
#%%
# Some iPython automagic commands below. Comment out if not using
# as an interactive script
%load_ext autoreload
#
%autoreload 2
#%%
import numpy as np
import matplotlib.pyplot as plt
import time
#%%
from QbiMadym import madym_DCE_lite
#%%
#Randomly sample a set of TK parameters with uniform distribution, U(a,b)
n_samples = int(1e3)
Ktrans = np.random.rand(n_samples)*0.2 + 0.1 # U(0.1, 0.3)
v_e = np.random.rand(n_samples)*0.2 + 0.1 # U(0.1, 0.3)
v_p = np.random.rand(n_samples)*0.1 + 0.05 # U(0.05, 0.15)
tau_a = np.random.rand(n_samples)*0.5 # U(0.0, 0.5)

#%%
#Create dynamic time-points
n_t = 60
temp_res_mins = 5 / 60
t = np.arange(60)*temp_res_mins
#%%
#Create some dummy input time-series
Ct_in = np.zeros((n_samples,n_t))

#Concatenate the model parameters to be used as initial params into an n_samples x n_t array
init_params = np.concatenate(
    (Ktrans.reshape(-1,1), 
     v_e.reshape(-1,1), 
     v_p.reshape(-1,1), 
     tau_a.reshape(-1,1)), axis=1
)
# %%
#Call madym_DCE_lite to generate C_t time-series for each
#sample using the initial parameters array
C_t = madym_DCE_lite.run(
    model='ETM', #Name of TK model
    input_data = Ct_in, #Dummy data not used
    dyn_times = t, #Dynamic times
    no_optimise=True, #Don't fit after the initial intialisation
    init_params=init_params # Per sample parameter initialisation
    )[4] #Modelled C_t is the 5th output

# %%
from QbiPy.dce_models import dce_aif,tofts_model

#Create a population AIF using QbiPy's Aif class
aif = dce_aif.Aif(times = t, hct = 0.42)

#Use QbiPy tofts_model to get concentration time-series from
#model parameters
C_tp = tofts_model.concentration_from_model(aif, Ktrans, v_e, v_p, tau_a)

#%%
#We can view a couple of these time-series to make sure this
# has worked as expected
fig,axs = plt.subplots(2,2)
fig.suptitle('C(t) generated my madym_DCE_lite and QbiPy')
for sample in range(4):
    ax = axs[sample//2][sample%2]
    ax.plot(t, C_t[sample,:], 'g', label='madym')
    ax.plot(t, C_tp[sample,:], 'r--', label = 'QbiPy')
    ax.set_xlabel('Time (mins)')
    ax.set_ylabel('C(t)')
    ax.set_title(f'Voxel {sample+1}')
    if not sample:
        ax.legend()
fig.tight_layout()

print(f'Median difference between time series = {np.median(np.abs(C_t - C_tp))}')
# %%
#If we want to use custom AIF values, we create an AIF with type
#ARRAY and set the base_aif values directly.
custom_aif_vals = np.random.rand(n_t)
custom_aif = dce_aif.Aif(
    aif_type=dce_aif.AifType.ARRAY, 
    times = t,
    base_aif=custom_aif_vals)

aif.write_AIF('C:/isbe/temp_AIF.txt')

# %%
from QbiPy.dce_models import tissue_concentration
#Sample T_1 in ms
T1_0 = np.random.rand(n_samples)*100.0 + 1000.0 # U(900, 1100)
r1_const = 3.4 # Default for Omniscan
FA = 20
TR = 4.0
S_t0 = np.ones(n_samples)*100
S_t = tissue_concentration.concentration_to_signal(
    C_t = C_t, 
    T1_0 = T1_0, 
    M0 = S_t0, #Will use ratio generate M0
    FA = FA, 
    TR = TR, 
    relax_coeff = r1_const, 
    use_M0_ratio = 8 #The bolus injection image
    )

# %%
#Add noise to our signals
from QbiMadym.madym_DWI_lite import add_rician_noise

sigma = 10.0
S_tn = add_rician_noise(S_t, sigma)

# View some voxels
fig,axs = plt.subplots(2,2)
fig.suptitle('Single time-series S(t)')
for sample in range(4):
    ax = axs[sample//2][sample%2]
    ax.plot(t, S_t[sample,:], 'g', label='Original')
    ax.plot(t, S_tn[sample,:], 'r--', label='+ Rician noise')
    ax.set_xlabel('Time (mins)')
    ax.set_ylabel('S(t)')
    ax.set_title(f'Voxel {sample+1}')
    if not sample:
        ax.legend()
fig.tight_layout()
plt.savefig('C:/isbe/code/manchester_qbi/public/madym_cxx.wiki/monte_carlo_signals.png')
# %%
#Create a dict to store our results
fit_results = dict()
models = ['TOFTS', 'ETM', '2CXM']
for model in models[:1]:
    if model == 'TOFTS':
        #For (non-extended) Tofts, fix v_p = 0
        fixed_params = [3]
        fixed_values = [0.0]
    else:
        fixed_params = None
        fixed_values = None

    fit_results[model] = madym_DCE_lite.run(
        model=model, #Name of TK model
        input_data = S_tn, #Noisy signal data
        dyn_times = t, #Dynamic times
        input_Ct=False, #Input is signal not concentration
        T1=T1_0, #Below are all values we used in our ground truth
        FA=FA,
        r1_const=r1_const,
        TR=TR,
        dose=0.1,
        injection_image=8,
        hct=0.42,
        fixed_params=fixed_params,
        fixed_values=fixed_values
        )
# %%
# We can display the results for each model for a few time-series,
# using the output signal-converted and modelled C(t)
fig,axs = plt.subplots(2,2)
fig.suptitle('Example fits for each model')
for sample in range(4):
    ax = axs[sample//2][sample%2]

    # Signal-derived Ct is the 6th output of madym_DCE_lite.
    # All the models should generate the same C(t), so we'll 
    # take here from the ETM results
    C_tni = fit_results['ETM'][5][sample,:]
    ax.plot(t, C_tni, 'k--', label='Noisy input')

    for model in models:
        #Get the modelled Ct and plot
        Cm_tni = fit_results[model][4][sample,:]
        ax.plot(t, Cm_tni, '--', label=model)
    
    ax.set_xlabel('Time (mins)')
    ax.set_ylabel('C(t)')
    ax.set_title(f'Voxel {sample+1}')
    if not sample:
        ax.legend()
fig.tight_layout()
plt.savefig(f'C:/isbe/code/manchester_qbi/public/madym_cxx.wiki/monte_carlo_voxel_fits.png')

# %%
# Next we can look at how the fitted parameters measure up to the ground truth.
# These are the first output of `madym_DCE_lite`.

def param_scatter(ax, gt, fit, name):
    '''Helper function to make scatter plots of fitted
    parameters

    Parameters
    ----------
    ax : pyplot axes on which to plot
    gt : 1D array
        ground truth parameters values
    fit : 1D array
        fitted parameters
    name : str
        Name of the parameter
    '''
    #Set limits based on the gorund-truth - this keeps
    #consistent axes for all models, but does mean
    #we won't see outliers
    min_val = np.min(gt)
    max_val = np.max(gt)
    ax.plot(gt, fit, 'r.')
    ax.plot([min_val,max_val], [min_val,max_val], 'k--')
    ax.axis('equal')
    ax.set_xlim([min_val, max_val])
    ax.set_ylim([min_val, max_val])
    ax.set_title(name)
    ax.set_xlabel('Ground truth')
    ax.set_ylabel('Fitted')

#Show scatter plots of each param, for each model
for model in models:
    fig,axs = plt.subplots(2, 2, figsize=(8,8))
    fig.suptitle(f'Paramter scatter plots for {model}')

    model_params = fit_results[model][0]

    #For the 2CXM, convert first params to Ktrans
    if model == '2CXM':
        F_p_fit,PS_fit,v_e_fit,v_p_fit,tau_a_fit= model_params.T
        Krans_fit = (F_p_fit*PS_fit)/(F_p_fit+PS_fit)
    else:
        Krans_fit,v_e_fit,v_p_fit,tau_a_fit = model_params.T

    
    param_scatter(axs[0][0], Ktrans, Krans_fit, 'Ktrans')
    param_scatter(axs[0][1], v_e, v_e_fit, 'v_e')
    param_scatter(axs[1][0], v_p, v_p_fit, 'v_p')
    param_scatter(axs[1][1], tau_a, tau_a_fit, 'tau_a')

    fig.tight_layout()
    plt.savefig(f'C:/isbe/code/manchester_qbi/public/madym_cxx.wiki/monte_carlo_params_scatter_{model}.png')

# %%
