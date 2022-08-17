'''
Madym-lite tools are written in C++ in the main Madym toolbox, but designed to be used
in python (or Matlab), operating directly on numpy arrays and returning parameters
a numpy arrays (unlike the main Madym tools that operate on images saved to disk).

We demonstrate the use here of the 3 lite tools:

- madym_T1 (note both the main and lite versions of madym_T1 use the same wrapper)
- madym_DCE_lite
- madym_DWI_lite

'''
import numpy as np
import matplotlib.pyplot as plt

import madym_T1
import madym_DCE_lite
import madym_DWI_lite

#%% -----------------------------------------------------------------
# Madym T1
from QbiPy.t1_mapping.VFA import signal_from_T1

#Generate some signals from sample FA, TR, T1 and M0 values
T1 = np.array([500, 1000, 1500, 500, 1000, 1500])
M0 = np.array([1000, 1000, 1000, 2000, 2000, 2000])
TR = 3.5
FAs = np.array([2, 10, 18])

#will be 6 x 3
signals = signal_from_T1(T1, M0, FAs, TR)

#First run this in data mode using calculate_T1_lite:    
T1_fit, M0_fit,_,_ = run(
    ScannerParams=FAs, 
    signals=signals,
    TR=TR, 
    method='VFA')
signals_fit = signal_from_T1(T1_fit, M0_fit, FAs, TR)

plt.figure(figsize=(16,8))
for i_sample in range(6):
    plt.subplot(2,3,i_sample+1)
    plt.plot(FAs, signals[i_sample,:], 'go')
    plt.plot(FAs, signals_fit[i_sample,:], 'r*')
    plt.plot(FAs, signals_fit[i_sample,:], 'b-')
    plt.title('Parameter estimates (actual,fit)\n'
        f' T1: ({T1[i_sample]}, {T1_fit[i_sample]:4.1f}),\n'
        f' M0: ({M0[i_sample]}, {M0_fit[i_sample]:4.1f})')
    
    if not i_sample:
        plt.legend(['Signals', 'Fit to signals',' '])
    
    plt.xlabel('Flip angle (degrees)')
    plt.ylabel('Signal intensity') 
plt.tight_layout()
plt.show() 

print('Parameter estimates (actual,fit)')
for i_sample in range(6):
    print(f' T1: ({T1[i_sample]}, {T1_fit[i_sample]:4.1f})')
    print(f' M0: ({M0[i_sample]}, {M0_fit[i_sample]:4.1f})')


#%% -----------------------------------------------------------------
# Madym DCE
from QbiPy.dce_models.dce_aif import Aif
from QbiPy.dce_models.tissue_concentration import signal_to_concentration, concentration_to_signal
from QbiPy.dce_models import tofts_model

#Generate an concentration time-series using the ETM
nDyns = 100
ktrans = 0.25
ve = 0.2
vp = 0.1
tau = 0
injection_img = 8
t = np.linspace(0, 5, nDyns)
aif = Aif(times=t, prebolus=injection_img, hct=0.42)
C_t = tofts_model.concentration_from_model(aif, ktrans, ve, vp, tau)

#Add some noise and rescale so baseline mean is 0
C_tn = C_t + np.random.randn(1,nDyns)/100
C_tn = C_tn - np.mean(C_tn[:,0:injection_img])

#Use madym lite to fit this data
model_params_C, model_fit_C, _, _, CmC_t,_ = madym_DCE_lite.run(
    model='ETM', input_data=C_tn, dyn_times=t)

#Convert the concentrations to signals with some notional T1 values and
#refit using signals as input
FA = 20
TR = 3.5
T1_0 = 1000
r1_const = 3.4
S_t0 = 100
S_tn = concentration_to_signal(
    C_tn, T1_0, S_t0, FA, TR, r1_const, injection_img)

model_params_S, model_fit_S, _,_,CmS_t,Cm_t = run(
    model='ETM',
    input_data=S_tn,
    dyn_times=t,
    input_Ct=0,
    T1=T1_0,
    TR=TR,
    FA=FA,
    r1_const=r1_const,
    injection_image=injection_img)

#Convert the modelled concentrations back to signal space
Sm_t = concentration_to_signal(
    CmS_t, T1_0, S_t0, FA, TR, r1_const, injection_img)

print(f'Parameter estimation (actual, fitted concentration, fitted signal)')
print(f'Ktrans: ({ktrans:3.2f}, {model_params_C[0,0]:3.2f}, {model_params_S[0,0]:3.2f})')
print(f'Ve: ({ve:3.2f}, {model_params_C[0,1]:3.2f}, {model_params_S[0,1]:3.2f})')
print(f'Vp: ({vp:3.2f}, {model_params_C[0,2]:3.2f}, {model_params_S[0,2]:3.2f})')
print(f'Tau: ({tau:3.2f}, {model_params_C[0,3]:3.2f}, {model_params_S[0,3]:3.2f})')
print('Model fit residuals (should be < 0.01)')
print(f'Input concentration: {model_fit_C[0]:4.3f}')
print(f'Input signal: {model_fit_S[0]:4.3f}')

#Display plots of the fit
plt.figure(figsize=(16,8))
plt.suptitle('madym_lite test applied')
plt.subplot(2,2,(1 ,3))
plt.plot(t, C_tn.reshape(-1,1))
plt.plot(t, CmC_t.reshape(-1,1))
plt.legend(['C(t)', 'ETM model fit'])
plt.xlabel('Time (mins)')
plt.ylabel('Voxel concentration')
plt.title(f'Input C(t): Model fit SSE = {model_fit_C[0]:4.3f}')

plt.subplot(2,2,2)
plt.plot(t, C_tn.reshape(-1,1))
plt.plot(t, Cm_t.reshape(-1,1), '--')
plt.plot(t, CmS_t.reshape(-1,1))
plt.legend(['C(t)', 'C(t) (output from MaDym)', 'ETM model fit'])
plt.xlabel('Time (mins)')
plt.ylabel('Voxel concentration')
plt.title(f'Input S_t: Model fit SSE = {model_fit_S[0]:4.3f}')

plt.subplot(2,2,4)
plt.plot(t, S_tn.reshape(-1,1))
plt.plot(t, Sm_t.reshape(-1,1))
plt.legend(['S(t)', 'ETM model fit - converted to signal'])
plt.xlabel('Time (mins)')
plt.ylabel('Voxel signal')
plt.title(f'Input S(t): Signal SSE = {np.sum((S_tn-Sm_t)**2):4.3f}')
plt.tight_layout()
plt.show()

#%% -----------------------------------------------------------------
# Madym DWI
#Run tests for ADC and IVIM models
sigma = 1
B_vals = np.array([0.0, 20.0, 40.0, 60.0, 80.0, 100.0, 300.0, 500.0, 800.0])

#Generate ADC test data with Rician noise added
S0 = 100
ADC = 0.8e-3
signals = ADC_model(B_vals, S0, ADC)
signals_n = add_rician_noise(signals, sigma)

#Use madym lite to fit this data
model_params, _, _ = run(
    'ADC', signals_n, B_vals)
S0_f = model_params[0,0]
ADC_f = model_params[0,1]
signals_f = ADC_model(B_vals, S0_f, ADC_f )

#Display plots of the fit
plt.figure(figsize=(16,8))
plt.suptitle('madym_DWI_lite test applied')

plt.subplot(1,2,1)
plt.plot(B_vals, signals, 'k--', linewidth= 2)
plt.plot(B_vals, signals_n, 'rx', markersize = 10)
plt.plot(B_vals, signals_f, 'b-', linewidth = 2)
plt.plot(B_vals, signals_f, 'go', markersize = 10)
plt.legend(['Noise free signals',
    'Signals + Rician noise', 
    'Fitted ADC model signals',
    ''])
plt.xlabel('B-values (msecs)')
plt.ylabel('Signal')
plt.title('ADC: Parameter estimates (actual,fit)\n'
    f'S0: ({S0:4.3f}, {S0_f:4.3f}), '
    f'ADC: ({1e3*ADC:4.3f}, {1e3*ADC_f:4.3f})')

#Generate IVIM test data with Rician noise added
S0 = 100
D = 0.8e-3
f = 0.2
D_star = 15e-3
Bvals_thresh = [40.0,60.0,100.0,150.0]

signals = IVIM_model(B_vals, S0, D, f, D_star)
signals_n = add_rician_noise(signals, sigma)

#Use madym lite to fit this data
model_params, _, _ = run(
    'IVIM', signals_n, B_vals,
    Bvals_thresh=Bvals_thresh)
S0_f = model_params[0,0]
D_f = model_params[0,1]
f_f = model_params[0,2]
D_star_f = model_params[0,3]
signals_f = IVIM_model(B_vals, S0_f, D_f, f_f, D_star_f)


plt.subplot(1,2,2)
plt.plot(B_vals, signals, 'k--', linewidth= 2)
plt.plot(B_vals, signals_n, 'rx', markersize = 10)
plt.plot(B_vals, signals_f, 'b-', linewidth = 2)
plt.plot(B_vals, signals_f, 'go', markersize = 10)
plt.legend(['Noise free signals',
    'Signals + Rician noise', 
    'Fitted IVIM model signals',
    ''])
plt.xlabel('B-values (msecs)')
plt.ylabel('Signal')
plt.title('IVIM: Parameter estimates (actual,fit)\n'
    f'S0: ({S0:4.3f}, {S0_f:4.3f}), '
    f'D: ({1e3*D:4.3f}, {1e3*D_f:4.3f}), '
    f'f: ({f:4.3f}, {f_f:4.3f}), '
    f'D*: ({1e3*D_star:4.3f}, {1e3*D_star_f:4.3f})')

plt.tight_layout()
plt.show()


