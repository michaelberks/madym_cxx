'''
Wrapper to C++ tool madym_DCE_lite
'''
import numpy as np
import warnings
from tempfile import TemporaryDirectory
import subprocess
import os
from QbiMadym.utils import local_madym_root, add_option

def run(model, signals, B_values,
    cmd_exe:str = None,
    output_dir:str = None, 
    output_name:str = 'madym_analysis.dat',
    Bvals_thresh:list = None,
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
        signals : np.array,
            2D array (Nsamples x NBvalues), signals at varying B-values, one series per row

        B_values : np.array
            B-values can either by a single array of size n_signals, in which case
            the same values are used for all samples, or the array size must be 
            (Nsamples x NBvalues) matching signals array 
    
     Optional inputs:
        cmd_exe : str default None, 
            Allows specifying madym_lite executable, if empty uses local Madym installation
        output_dir : str default None, 
            Output path, will use temp dir if empty
        output_name : str default 'madym_analysis.dat',  
            Name of output file
        Bvals_thresh:list = None
            Thresholds used in IVIM fitting
        dummy_run : bool default False 
            Don't run any thing, just print the cmd we'll run to inspect
    
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
    
     Examples:
    
     Notes:
       
     Created: 20-Feb-2019
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk 
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
        
        cmd_exe = os.path.join(madym_root,'madym_DWI_lite')

    #Get size number of signals - if we've been given a single column
    #vector, transpose into a row. Should n_voxels x n_B_vals
    signals = np.atleast_2d(signals)
    if signals.shape[1]==1:
        signals = np.transpose(signals)
    n_samples, n_signals = signals.shape

    #B-values can either by a single array of size n_signals, in which case
    # the same values are used for all samples, so repeat the B-values into
    # an n_samples x n_signals array, or the array size must already match 
    # the signals array 
    if B_values.size == n_signals:
        B_values = np.repeat(B_values.reshape(1,n_signals), n_samples, 0)
    elif B_values.shape != (n_samples, n_signals):
        ValueError('Size of B values array does not match size of signals array')

    #Combine the signals and B-values arrays
    combined_input = np.concatenate((B_values, signals), axis=1)

    #Check for bad samples, these can screw up Madym as the lite version
    #of the software doesn't do the full range of error checks Madym proper
    #does. So chuck them out now and warn the user
    discard_samples = np.any(np.isnan(combined_input), 1)
    if any(discard_samples):
        warnings.warn('Samples with NaN values found,'
            ' these will be set to zero for model-fitting')
        combined_input[discard_samples,:] = 0

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
        '--DWI_model', model,#
        '--data', f'{input_file}',
        '--n_DWI', str(n_signals),
        '-o', f'{output_dir}',
        '-O', output_name]


    add_option('float_list', cmd_args, '--Bvals_thresh', Bvals_thresh)

    #Args structure complete, convert to string for printing
    cmd_str = ' '.join(cmd_args)
        
    if dummy_run:
        #Don't actually run anything, just print the command
        print('***********************Madym-DWI-lite dummy run **********************')
        print(cmd_str)
        
        model_params = []
        model_fit = []
        error_codes = []
        return model_params, model_fit, error_codes

    #Write input values to a temporary file
    np.savetxt(input_file, combined_input, fmt='%6.5f')


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

    model_params = outputData[:, 0:-2]
    model_fit = outputData[:,-2]      
    error_codes = outputData[:,-1]

    #Finally, tidy up any temporary files - the temp input directory does
    #this for us if we call cleanup
    input_dir.cleanup()

    if delete_output:
        output_temp_dir.cleanup()

    #Return the fit parameters
    return model_params, model_fit, error_codes
    

#-----------------------------------------------------------------------
#Test function to run if no inputs
def test(plot_output=True):
    '''
    Test the main run function on some synthetically DWI signal series
    Inputs:
        plot_output: if true, plots the fitted time-series. Set to false to run on non-
        interactive settings (eg installation on CSF)
    '''
    import matplotlib.pyplot as plt
    
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
    if plot_output:
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

    if plot_output:
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

#--------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------
def ADC_model(B_values, S0, ADC):
    '''
    Compute modelled signal

    Inputs:
        B_values - 1-D array of B-values

        S0 - ADC model parameter, scalar.

        ADC - ADC model parameter, n-D array or scalar.

    Outputs:
        signals - 2D array, numel(B_values) x nSamples, signals computed 
        from IVIM model, 1 sample of B-values per row
    '''

    return S0 * np.exp(-ADC * B_values)

#--------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------
def IVIM_model(B_values, S0, D, f, D_star, full = True):
    '''
        IVIM_MODEL Compute modelled signal
    [signals] = IVIM_model(B_values, S0, D, f, D_star, full)

    Inputs:
        B_values - 1-D array of B-values

        S0 - IVIM model parameter, scalar.

        D - IVIM model parameter, n-D array or scalar.

        f - IVIM model parameter, n-D array or scalar.

        D_star - IVIM model parameter, n-D array or scalar.

        full - bool, default true. Set true if using the full model. If
        false, assume D* >> D, so that the 2nd exponential term is ignored


    Outputs:
        signals - 2D array, numel(B_values) x nSamples, signals computed 
        from IVIM model, 1 sample of B-values per row
    '''
    if not full:
        #If not using the full model, set D* to infinity, so 2nd term collapses
        #to zero
        D_star = np.Inf

    return S0 * (
        (1 - f) * np.exp(-D * B_values) +
        f * np.exp(-D_star * B_values))

#--------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------
def add_rician_noise(data_in, sigma):
    '''Add Rician noise to a sample of data
       [data_out] = add_rician_noise(data_in, sigma)
    
     Inputs:
          data_in - N-d array of data
    
          sigma - standard deviation of Rician noise
    
    
     Outputs:
          data_out - N-d array of same size as data_in, with Rician noise
          added
    
    
    '''
    return np.abs( data_in +
        sigma * np.random.randn(*(data_in.shape)) + 
        1j *sigma * np.random.randn(*(data_in.shape)) )