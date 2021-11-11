'''
Wrapper to C++ tool madym_DCE_lite
'''
import numpy as np
import warnings
from tempfile import TemporaryDirectory
import subprocess
import os
from QbiMadym.utils import local_madym_root

def run(model=None, input_data=None,
    cmd_exe:str = None,
    output_dir:str = None, 
    output_name:str = 'madym_analysis.dat',
    dyn_times:np.array = None,
    input_Ct:bool = True,
    output_Ct_sig:bool = True,
    output_Ct_mod:bool = True,
    no_optimise:bool = None,
    # The below are all only required if we're converting from signals
    T1:np.array = None, 
    M0:np.array = None,
    TR:float = None,
    FA:float = None,
    r1_const:float = None,
    M0_ratio:bool = None,
    B1_values:np.array = None,
    dose:float = None,
    injection_image:int = None,
    hct:float = None,
    first_image:int = None,
    last_image:int = None,
    #
    aif_name:str = None,
    pif_name:str = None,
    #
    IAUC_times:np.array = [60,90,120],
    IAUC_at_peak:bool = None,
    init_params:np.array = None,
    fixed_params:np.array = None,
    fixed_values:np.array = None,
    upper_bounds:np.array = None,
    lower_bounds:np.array = None,
    relative_limit_params:np.array = None,
    relative_limit_values:np.array = None,
    #
    max_iter: int = None,
    opt_type:str = None,
    dyn_noise_values:np.array = None,
    test_enhancement:bool = None,
    quiet:bool = None,
    dummy_run:bool = False
):
    '''
    MADYM_LITE wrapper function to call C++ tool Madym-lite. Fits
       tracer-kinetic models to DCE time-series, returning the model
       parameters and modelled concentration time-series
       [model_params, model_fit, error_codes, Ct_m, Ct_s] = 
           run_madym_lite(model, input_data, varargin)

    Note: This wrapper allows setting of all optional parameters the full C++ function takes.
    However rather than setting default values for the parameters in this wrapper (which python
    makes super easy and would be nice), the defaults are largely set to None (apart from boolean)
    flags, so that if they're not set in the wrapper they will use the C++ default. This is to avoid
    inconsistency between calling the C++ function from the wrapper vs directly (and also between
    the Matlab and python wrappers). If in time we find one wrapper in predominantly used, we may
    instead choose to set the defaults here (again doing this in python would be preferable due
    to the ease with which it deals with optional parameters)
    
    Mandatory inputs (if not set will run test module on synthetic data):
        model : str,
            Model to fit, specified by its name in CAPITALS,
                see notes for options
    
        input_data : np.array,
            2D array (Nsamples x Ntimes), Either signals or 
                concentration, 1 time-series per row
    
     Optional inputs:
        cmd_exe : str default None, 
            Allows specifying madym_lite executable, if empty uses local Madym installation
        output_dir : str default None, 
            Output path, will use temp dir if empty
        output_name : str default 'madym_analysis.dat',  
            Name of output file
        dyn_times : np.array default None,  
            Time associated with each dynamic signal (in mins), must be supplied if using population AIF
        input_Ct : bool default True,  
            Flag specifying input dynamic sequence are concentration (not signal) maps
        output_Ct_sig : bool default True, 
            Flag requesting concentration (derived from signal) are saved to output
        output_Ct_mod : bool default True, 
            Flag requesting modelled concentration maps are saved to output
        no_optimise : bool default False, 
            Flag to switch off optimising, will just fit initial parameters values for model
         
        The below are all only required if we're converting from signals
        T1 : np.array default None, 
            Baseline T1 values (in ms)
        M0 : np.array default None, 
            Baseline M0 values, required if not using ratio method
        TR : float default None,  
            TR of dynamic series (in ms), must be >0 if converting signals
        FA : float default None,  
            Flip angle of dynamic series (degrees), must be set if converting signals
        r1_const : float default None, 
            Relaxivity constant of concentration in tissue (in ms)
        M0_ratio : bool default True,  
            Flag to use ratio method to scale signal instead of supplying M0
        B1_values:np.array default False,
            B1 values to correct each FA, 1D array of length n_samples
        dose : float default None, 
            Concentration dose (mmole/kg)
        injection_image : int default None,  
            Injection image
        hct : float default None, 
            Haematocrit correction
        first_image : int default None,  
            First image used to compute model fit
        last_image : int default None,  
            Last image used to compute model fit
        
        aif_name : str default None,  
            Path to precomputed AIF if not using population AIF
        pif_name : str default None,  
            Path to precomputed PIF if not deriving from AIF
        
        IAUC_times : np.array default [60,90,120], 
            imes (in s) at which to compute IAUC values
        IAUC_at_peak : bool default False
            Flag requesting IAUC computed at peak signal   
        init_params : np.array default None, 
            Initial values for model parameters to be optimised, either as single vector, or 2D array NSamples x NParams
        fixed_params : np.array default None,
            Parameters fixed to their initial values (ie not optimised)
        fixed_values : np.array default None, 
            Values for fixed parameters (overrides default initial parameter values)"
        lower_bounds: np.array = None
		    Lower bounds for each parameter during optimisation
	    upper_bounds: np.array = None
		    Upper bounds for each parameter during optimisation
        relative_limit_params : np.array = None,
            Parameters with relative limits on their optimisation bounds
        relative_limit_values : np.array = None,
            Values for relative bounds, sets lower/upper bound as init param -/+ relative limit
        
        max_iter: int = None
            Maximum number of iterations to run model fit for
        opt_type: str = None
            Type of optimisation to run
        dyn_noise_values : np.array default None,
            Varying temporal noise in model fit
        test_enhancement : bool default False, 
            Set test-for-enhancement flag
        quiet : bool = False,
            Do not display logging messages in cout
        dummy_run : bool default False 
            Don't run any thing, just print the cmd we'll run to inspect)
    
     Outputs:
          model_params (2D array, Nsamples x Nparams) - each row contains 
           the estimated model parameters for the corresponding time-series 
           in the input_data. The number of columns depends on the model 
           parameters. The parameters are returned in the same order as the 
           defined in Madym. See notes for details for each model.
    
          model_fit (1D array, Nsamples x 1) - sum-of-squared model 
           residuals for each time-series
    
          error_codes (2D array, Nsamples x 2) - error codes returned by MaDym
           for fitting each sample. 0 implies no errors or warnings. For all
           non-zero values refer to Madym documentation for details
    
          Ct_m (2D array, Nsamples x Ntimes) - the modelled
           concentration time-series for each input
    
          Ct_s (2D array, Nsamples x 2) - the signal-derived
           concentration time-series for each input
    
     Examples:
       Fitting to concentration time-series. If using a population AIF, you
       must supply a vector of dynamic times. A population AIF (PIF) is used
       if the aif_name (pif_name) option is left empty.
       [model_params, model_fit, error_codes, Ct_m] = 
           run("2CXM", Ct_s, 'dyn_times', t)
    
       Fitting to concentration time-series using a patient specific AIF. The
       AIF should be defined in a text file with two columns containing the
       dynamic times and associated AIF value at each times respectively. Pass
       the full filepath as input
       [model_params, model_fit, error_codes, Ct_m] = 
           run("2CXM", Ct_s, 'aif_name', 'C:\DCE_data\pt_AIF.txt')
    
       Fitting to signals - Set input_Ct to false and use options to supply
       T1 values (and TR, FA, relax_coeff etc) to convert signals to
       concentration.
       [model_params, model_fit, error_codes, Ct_m, Ct_s] = 
           run("2CXM", dyn_signals, 'dyn_times', t,
               'input_Ct', 0, 'T1', T1_vals, 'TR', TR, 'FA', FA)
    
       Fixing values in a model - eg to fit a TM instead of ETM, set Vp (the
       3rd parameter in the ETM to 0)
       [model_params, model_fit, error_codes, Ct_m] = 
           run("ETM", Ct_s, 'dyn_times', t,
               'fixed_params', 3, 'fixed_values', 0.0)
    
     Notes:
       Tracer-kinetic models:
    
       All models available in the main MaDym and MaDym-Lite C++ tools are
       available to fit. See https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/dce_models for details.
    
     Created: 20-Feb-2019
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk 
     Phone : +44 (0)161 275 7669 
     Copyright: (C) University of Manchester 
    '''
    if model is None:
        test()
        return

    if cmd_exe is None:
        madym_root = local_madym_root()

        if not madym_root:
            print('MADYM_ROOT not set. This could be because the'
                ' madym tools were installed in a different python/conda environment.'
                ' Please check your installation. To run from a local folder (without requiring MADYM_ROOT)'
                ' you must set the cmd_exe argument')
            raise ValueError('cmd_exe not specified and MADYM_ROOT not found.')
        
        cmd_exe = os.path.join(madym_root,'madym_DCE_lite')

    #Get size number of dynamic values - if we've been given a single column
    #vector, transpose into a row. Should n_voxels x n_dyns
    input_data = np.atleast_2d(input_data)
    if input_data.shape[1]==1:
        input_data = np.transpose(input_data)

    n_samples, n_dyns = input_data.shape

    #If we're converting from signal to concentration, append T1 (and M0 if not
    #using ratio method) to pinput data
    if not input_Ct:
        if T1 is None:
            raise ValueError('T1 must be set if input data are signals not concentrations')
        input_data = np.concatenate(
            (input_data, np.atleast_1d(T1).reshape((n_samples,1))),
            1
        )
        
        if not M0_ratio:
            if M0 is None:
                raise ValueError('M0 if M0_ratio is set to false')
            input_data = np.concatenate(
                (input_data, np.atleast_1d(M0).reshape((n_samples,1))),
                1
            )

    #If B1 values supplied, append them to the signals and set B1_correction
    #flag (applied after cmd set up, see below)
    if B1_values is not None:
        input_data = np.concatenate(
            (input_data, np.atleast_1d(B1_values).reshape((n_samples,1))),
            1
        )

    #Check for bad samples, these can screw up Madym as the lite version
    #of the software doesn't do the full range of error checks Madym proper
    #does. So chuck them out now and warn the user
    discard_samples = np.any(np.isnan(input_data), 1)
    if any(discard_samples):
        warnings.warn('Samples with NaN values found,'
            ' these will be set to zero for model-fitting')
        input_data[discard_samples,:] = 0


    discard_samples = np.any(~np.isfinite(input_data), 1)
    if any(discard_samples):
        warnings.warn('Samples with non-finite values found,'
            ' these will be set to zero for model-fitting')
        input_data[discard_samples,:] = 0

    #Get a name for the temporary file we'll write input data to (we'll hold
    #off writing anything until we know this isn't a dummy run). For python
    #we'll do this differently to Matlab, using the tempdir function to create
    #an input directory we'll then cleanup at the end
    input_dir = TemporaryDirectory()
    input_file = os.path.join(input_dir.name, 'input_data.dat')
        
    delete_output = False
    if output_dir is None:
        output_temp_dir = TemporaryDirectory()
        output_dir = output_temp_dir.name
        delete_output = True

    full_out_path = os.path.join(output_dir, model + '_' +  output_name)

    #Set up initial string using all the values we can directly input args
    cmd_args = [cmd_exe, 
        '-m', model,#
        '--data', f'{input_file}',
        '-n', str(n_dyns),
        '-o', f'{output_dir}',
        '-O', output_name]

    #Now set any args that require option inputs
    if input_Ct is not None:
        cmd_args += ['--Ct', str(int(input_Ct))]
        
    else: #Below are only needed if input is signals
        if TR is not None:
            cmd_args += ['--TR', f'{TR:4.3f}']
        
        if FA is not None:
            cmd_args += ['--FA', f'{FA:4.3f}']
        
        if r1_const is not None:
            cmd_args += ['--r1', f'{r1_const:4.3f}']       

        if M0_ratio:
            cmd_args += ['--M0_ratio']
 
    if dose is not None:
        cmd_args += ['--dose', f'{dose:4.3f}']

    if hct is not None:
        cmd_args += ['--hct', f'{hct:4.3f}']

    if injection_image is not None:
        cmd_args += ['--inj', str(injection_image)]

    if first_image is not None:
        cmd_args += ['--first', str(first_image)]

    if last_image is not None:
        cmd_args += ['--last', str(last_image)]

    if output_Ct_sig is not None:
        cmd_args += ['--Ct_sig', str(int(output_Ct_sig))]

    if output_Ct_mod is not None:
        cmd_args += ['--Ct_mod', str(int(output_Ct_mod))]

    if no_optimise is not None:
        cmd_args += ['--no_opt', str(int(no_optimise))]

    if test_enhancement is not None:
        cmd_args += ['--test_enh', str(int(test_enhancement))]

    if aif_name is not None:
        cmd_args += ['--aif', aif_name]

    if pif_name is not None:
        cmd_args += ['--pif', pif_name]

    if dyn_times is not None:
        #Get a name for the temporary file we'll write times to (we'll hold
        #off writing anything until we know this isn't a dummy run
        dyn_times_file = os.path.join(input_dir.name, 'dyn_times.dat')
        cmd_args += ['-t', f'{dyn_times_file}']

    if dyn_noise_values is not None:
        #Get a name for the temporary file we'll write noise to (we'll hold
        #off writing anything until we know this isn't a dummy run
        dyn_noise_file = os.path.join(input_dir.name, 'dyn_noise.dat')
        cmd_args += ['--dyn_noise', f'{dyn_noise_file}']

    if IAUC_times:
        IAUC_str = ','.join(f'{i:3.2f}' for i in IAUC_times)
        cmd_args += ['--iauc', IAUC_str]

    if IAUC_at_peak is not None:
        cmd_args += ['--iauc_peak', str(int(IAUC_at_peak))]

    load_params = False
    if init_params is not None:
        if n_samples > 1 and init_params.shape[0] == n_samples:
            input_params_file = os.path.join(input_dir.name, 'input_params.dat')
            load_params = True
            cmd_args += ['--init_params_file', f'{input_params_file}']
        else:
            init_str = ','.join(f'{i:5.4f}' for i in init_params)           
            cmd_args += ['--init_params', init_str]
        
    if fixed_params is not None:
        fixed_str = ','.join(str(f) for f in fixed_params)       
        cmd_args += ['--fixed_params', fixed_str]
        
        if fixed_values is not None:
            fixed_str = ','.join(f'{f:5.4f}' for f in fixed_values)           
            cmd_args += ['--fixed_values', fixed_str]

    if lower_bounds:
        bounds_str = ','.join(f'{b:5.4f}' for b in lower_bounds)       
        cmd_args += ['--lower_bounds', bounds_str]

    if upper_bounds:
        bounds_str = ','.join(f'{b:5.4f}' for b in upper_bounds)       
        cmd_args += ['--upper_bounds', bounds_str]

    if relative_limit_params:
        relative_str = ','.join(str(r) for r in relative_limit_params)       
        cmd_args += ['--relative_limit_params', relative_str]
        
        if relative_limit_values:
            relative_str = ','.join(f'{r:5.4f}' for r in relative_limit_values)           
            cmd_args += ['--relative_limit_values', relative_str]

    if max_iter is not None:
        cmd_args += ['--max_iter', str(max_iter)]

    if opt_type is not None:
        cmd_args += ['--opt_type', opt_type]

    if B1_values is not None:
        cmd_args += ['--B1_correction']

    if quiet is not None:
        cmd_args += ['--quiet', str(int(quiet))]

    #Args structure complete, convert to string for printing
    cmd_str = ' '.join(cmd_args)

    if dummy_run:
        #Don't actually run anything, just print the command
        print('***********************Madym-lite dummy run **********************')
        print(cmd_str)
        
        model_params = []
        model_fit = []
        iauc = []
        error_codes = []
        Ct_m = []
        Ct_s = []
        return model_params, model_fit, iauc, error_codes, Ct_m, Ct_s

    #Write input values to a temporary file
    np.savetxt(input_file, input_data, fmt='%6.5f')

    #Write input params to a temporary file
    if load_params:
        discard = ~np.isfinite(init_params) | np.isnan(init_params)
        init_params[discard] = 0
        np.savetxt(input_params_file, init_params, fmt='%6.5f')

    #Write dynamic times to a temporary file if we need to
    if dyn_times is not None:
        np.savetxt(dyn_times_file, dyn_times, fmt='%6.5f')
        
    #Write noise vals to a temporary file if we need to
    if dyn_noise_values is not None:
        np.savetxt(dyn_noise_file, dyn_noise_values, fmt='%6.5f')

    #At last.. we can run the command
    print('***********************Madym-lite running **********************')
    print(cmd_str)
    result = subprocess.Popen(cmd_args, shell=False,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while True:
        out = result.stdout.readline().decode("utf-8")
        if out == '' and result.poll() is not None:
            break
        if out:
            print(f"{out}", end='')

    if result.returncode:        
        input_dir.cleanup()
        if delete_output:
            output_temp_dir.cleanup()
        raise RuntimeError(f'madym_lite failed to execute, returning code {result.returncode}.'
            f' Command ran was: {cmd_str}')

    #Now load the output from madym lite and extract data to match this
    #functions outputs
    outputData = np.atleast_2d(np.loadtxt(full_out_path))

    error_codes = outputData[:,0:2]
    model_fit = outputData[:,2]

    n_iauc = len(IAUC_times) + int(IAUC_at_peak)
    iauc = outputData[:, 3:3+n_iauc]

    #Workout which columns hold parameter values
    n_cols = outputData.shape[1]

    param_col1 = n_iauc + 3
    param_col2 = n_cols
    if output_Ct_mod:
        if output_Ct_sig:
            #Have another n_dyns of dynamic concentration values
            Ct_s = outputData[:, -n_dyns:]
            param_col2 -= n_dyns
                  
        
        #After the params we have n_dyns of model concentration values
        Ct_m = outputData[:, param_col2-n_dyns:param_col2]
        param_col2 -= n_dyns       

    model_params = outputData[:, param_col1:param_col2]

    #Finally, tidy up any temporary files - the temp input directory does
    #this for us if we call cleanup
    input_dir.cleanup()

    if delete_output:
        output_temp_dir.cleanup()

    #Return the fit parameters
    return model_params, model_fit, iauc, error_codes, Ct_m, Ct_s
    

    #-----------------------------------------------------------------------
    #Test function to run if no inputs
def test(plot_output=True):
    '''
    Test the main run function on some synthetically generated Tofts model
    time-series
    Inputs:
        plot_output: if true, plots the fitted time-series. Set to false to run on non-
        interactive settings (eg installation on CSF)
    '''
    import matplotlib.pyplot as plt
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
    model_params_C, model_fit_C, _, _, CmC_t,_ = run(
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

    if plot_output:
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
#--------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------