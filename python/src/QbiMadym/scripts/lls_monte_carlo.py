#%%
# Some iPython automagic commands below. Comment out if not using
# as an interactive scripts
%load_ext autoreload

%autoreload 2

%config Completer.use_jedi = False
#%%
import numpy as np
import matplotlib.pyplot as plt
import time
#%%
from QbiMadym import madym_DCE_lite
from QbiPy.dce_models import dce_aif,tofts_model,two_cxm_model

# %%
def print_err(param_name, param_gt, param_idx, fitted_params, fit_time):

    param_lls = fitted_params['LLS'][:,param_idx]
    param_nlls = fitted_params['BLEIC'][:,param_idx]
    LLS_err = np.median(np.abs(param_gt - param_lls))
    NLLS_err = np.median(np.abs(param_gt - param_nlls))
    LLS_t = fit_time['LLS']
    NLLS_t = fit_time['BLEIC']

    min_param = np.min(param_gt)
    max_param = np.max(param_gt)
    plt.figure(figsize=(12,6))
    plt.suptitle(f'{param_name}')
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

# %%
def run_MC(model, params_gt, param_names, Ct, noise_std, 
    fixed_params = None, fixed_values = None,
    repeat_param = None, repeat_values = None):
    Ct_n = Ct + np.random.randn(*Ct.shape)*noise_std
    
    fitted_params = dict()
    fit_time = dict()
    for opt_type in {'BLEIC', 'LLS'}:

        if opt_type == 'BLEIC':
            rp = None
            rv = None
        else:
            rp = repeat_param
            rv = repeat_values
    
        start_time = time.time()
        fitted_params[opt_type] = madym_DCE_lite.run(
            model=model, 
            input_data=Ct_n, 
            dyn_times=t, 
            hct = 0,
            fixed_params = fixed_params, 
            fixed_values = fixed_values,
            repeat_param = rp, 
            repeat_values = rv, 
            quiet = 1,
            opt_type = opt_type)[0]

        elapsed_time = time.time() - start_time
        fit_time[opt_type] = elapsed_time

    for i_p, (param_gt, param_name) in enumerate(zip(params_gt, param_names)):
        print_err(f'{param_name}, noise = {noise_std}', param_gt, 
            i_p, fitted_params, fit_time)

#%%
t = np.linspace(0, 5, 51)
aif = dce_aif.Aif(times = t)

Ca_t = aif.resample_AIF(0)[0,]

n_samples = int(1e3)
Ktrans = np.random.rand(n_samples)*0.2 + 0.1
v_e = np.random.rand(n_samples)*0.2 + 0.5
v_p = np.random.rand(n_samples)*0.1 + 0.05
tau_a = np.random.rand(n_samples)*0.5

#%%
Ct = tofts_model.concentration_from_model(aif, Ktrans, v_e, v_p, 0)

for noise in [1/100, 1/50, 1/25, 1/10]:
    run_MC('ETM', (Ktrans, v_e, v_p), ('Ktrans', 'v_e', 'v_p'), 
        Ct, noise,
        fixed_params = [4], fixed_values = [0])

#%%
Ct = tofts_model.concentration_from_model(aif, Ktrans, v_e, v_p, tau_a)

for noise in [1/100, 1/50, 1/25, 1/10]:
    run_MC('ETM', (Ktrans, v_e, v_p, tau_a), ('Ktrans', 'v_e', 'v_p', 'tau_a'), 
        Ct, noise,
        repeat_param = 4, repeat_values = [0.0,0.1,0.2,0.3,0.4,0.5])

#%%
F_p = np.random.rand(n_samples)*0.4 + 0.8
PS = np.random.rand(n_samples)*0.2 + 0.1
v_e = np.random.rand(n_samples)*0.2 + 0.5
v_p = np.random.rand(n_samples)*0.1 + 0.05

#%%
Ct = two_cxm_model.concentration_from_model(aif, F_p, PS, v_e, v_p, 0)

for noise in [1/1000, 1/100, 1/50, 1/25, 1/10]:
    run_MC('2CXM', (F_p, PS, v_e, v_p), ('F_p', 'PS', 'v_e', 'v_p'), 
        Ct, noise,
        fixed_params = [5], fixed_values = [0])
# %%
