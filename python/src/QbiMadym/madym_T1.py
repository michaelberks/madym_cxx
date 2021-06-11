'''
Wrapper to C++ tool madym_T1
'''
import os
import warnings
import subprocess
import numpy as np
from tempfile import TemporaryDirectory

from QbiPy.image_io.analyze_format import read_analyze_img, write_analyze
from QbiMadym.utils import local_madym_root

def  run(
    config_file = None,
    cmd_exe:str = None,
    T1_vols:list = None,
    ScannerParams:np.array = None,
    signals:np.array = None,
  	TR:float = None,
    B1_name:str = None,
    B1_scaling:float = None,
    B1_values:np.array = None,
    method:str = None,
    output_dir:str = None,
    output_name:str = 'madym_analysis.dat',
    noise_thresh:float = None,
    roi_name:str = None,
    program_log_name:str = None,
    audit_dir:str = None,
    audit_name:str = None,
    config_out:str = None,
    error_name:str = None,
    no_log:bool = False,
    no_audit:bool = False,
    quiet:bool = False,
    img_fmt_r:str = None,
    img_fmt_w:str = None,
    overwrite:bool = False,
    working_directory:str = None,
    return_maps:bool = False,
    dummy_run:bool = False
):
    '''
    MADYM_T1 wrapper function to call C++ T1 calculator, applying the
    variable flip angle method. Inputs can be paths to analyze-format images,
    or numeric arrays
       [model_params, model_fit, error_codes, model_conc, dyn_conc] = 
           run(model, input_data)
    
    Note: This wrapper allows setting of all optional parameters the full C++ function takes.
    However rather than setting default values for the parameters in this wrapper (which python
    makes super easy and would be nice), the defaults are largely set to None (apart from boolean)
    flags, so that if they're not set in the wrapper they will use the C++ default. This is to avoid
    inconsistency between calling the C++ function from the wrapper vs directly (and also between
    the Matlab and python wrappers). If in time we find one wrapper in predominantly used, we may
    instead choose to set the defaults here (again doing this in python would be preferable due
    to the ease with which it deals with optional parameters) 
    
    Inputs:
        config_file: str = None
            Path to file setting options OR
        cmd_exe : str = None,
            Path to the C++ executable to be run.
            One of T1_vols or ScannerParams must be set (if neither, the test function will be run on
        synthetic data). If T1_vols given, calculate_T1 will be run, if FA values are given
        calculate_T1_lite will be called. In the _lite case, signals must also be set

        T1_vols : list default None, 
			Variable flip angle file names, comma separated (no spaces)
        ScannerParams : np.array default None, 
		    ScannerParams, either single vector used for all samples, or 2D array, 1 row per sample
        signals : np.array default None, 
			Signals associated with each FA, 1 row per sample
        TR : float default None, 
			TR in msecs, required if directly fitting (otherwise will be taken from FA map headers)
        B1_name : str default None,
            Path to B1 correction map
        B1_scaling:float default None,
            Value applied to scaled values in B1 correction map
        B1_values:np.array default None,
            B1 correction values, 1D array of length n_samples
        method : str default 'VFA',
			T1 method to use to fit, see notes for options
        output_dir : str default None, 
			Output path, will use temp dir if empty
        output_name : str default 'madym_analysis.dat', 
			 Name of output file
        noise_thresh : float default None, 
			PD noise threshold
        roi_name : str default None,
			Path to ROI map
        program_log_name : str = None, 
            Program log file name
        audit_dir : str = None,
            Folder in which audit output is saved
        audit_name : str = None, 
            Audit file name
        error_name : str default None,
			Error codes image file name
        config_out : str = None,
            Filename of output config file, will be appended with datetime
        no_log arg : bool = False,
            Switch off program logging
        no_audit : bool = False,
            Switch off audit logging
        quiet : bool = False,
            Do not display logging messages in cout
        img_fmt_r : str = None
            Image format for reading input
        img_fmt_w : str = None
            Image format for writing output
        overwrite : bool default False,
			Set overwrite existing analysis in output dir ON
        working_directory : str = None,
            Sets the current working directory for the system call, allows setting relative input paths for data
        return_maps : bool = False
            If true, attempt to load output T1/M0 maps. Only currently works for Analyze output
        dummy_run : bool default False 
			Don't run any thing, just print the cmd we'll run to inspect
    
     Outputs:
          
          T1 (1D array, Nsamples x 1 or []) - if fitting to numeric data,
          vector of T1 values computed for each input sample.
    
          M0 (1D array, Nsamples x 1 or []) - if fitting to numeric data,
          vector of M0 values computed for each input sample.
    
          result - returned by the system call to the Madym executable.
          These may be operating system dependent, however status=0 should
          mean an error free operation. If status is non-zero an error has
          probably occurred, the result of which may be set in result. In any
          event, it is best to check the output_dir, and any program logs that
          have been saved there.
    
     Examples:
       Fitting to full volumes:
    
       Fitting to numeric data:
    
    
     Notes:
    
       All T1 methods implemented in the main MaDym and MaDym-Lite C++ tools are
       available to fit. Currently these are VFA (+ optional B1 correction) and
       IR
    
     Created: 20-Feb-2019
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk 
     Phone : +44 (0)161 275 7669 
     Copyright: (C) University of Manchester'''

    #Parse inputs, check if using full or lite version 
    use_lite = config_file is None and T1_vols is None

    if use_lite:
        if ScannerParams is None:
            test()
            return

        if signals is None:
            raise ValueError('Must supply either map names, or both FA and signal data')

    if cmd_exe is None:
        madym_root = local_madym_root()

        if not madym_root:
            print('MADYM_ROOT not set. This could be because the'
                ' madym tools were installed in a different python/conda environment.'
                ' Please check your installation. To run from a local folder (without requiring MADYM_ROOT)'
                ' you must set the cmd_exe argument')
            raise ValueError('cmd_exe not specified and MADYM_ROOT not found.')
        
        cmd_exe = os.path.join(madym_root,'madym_T1')

        if use_lite:
            cmd_exe += '_lite'

        cmd_args =  [cmd_exe]

    #Set up output directory
    delete_output = False
    if config_file is None and output_dir is None:
        output_temp_dir = TemporaryDirectory()
        output_dir = output_temp_dir.name
        delete_output = True

    #Check if fitting to full volumes saved on disk, or directly supplied data
    if not use_lite:

        if config_file:
            cmd_args += ['--config', config_file]
        
        #Set up FA map names
        if T1_vols:
            #Set VFA files in the options string 
            t1_input_str = ','.join(T1_vols)   
            cmd_args += ['--T1_vols', t1_input_str]

        if output_dir:
            cmd_args += [ '-o', output_dir]

        if method:
            cmd_args += ['-T', method]

        if B1_name:
            cmd_args += ['--B1', B1_name]

        if B1_scaling is not None:
            cmd_args += ['--B1_scaling', B1_scaling]
        
        if noise_thresh is not None:
            cmd_args += ['--T1_noise', f'{noise_thresh:5.4f}']

        if roi_name is not None:
            cmd_args += ['--roi', roi_name]
        
        if error_name is not None:
            cmd_args += ['--err', error_name]

        if program_log_name:
            cmd_args += ['--log', program_log_name]

        if audit_name:
            cmd_args += ['--audit', audit_name]

        if audit_dir:
            cmd_args += ['--audit_dir', audit_dir]

        if config_out:
            cmd_args += ['--config_out', config_out]

        if no_log:
            cmd_args += ['--no_log']

        if no_audit:
            cmd_args += ['--no_audit']

        if quiet:
            cmd_args += ['--quiet']

        if img_fmt_r:
            cmd_args += ['--img_fmt_r', img_fmt_r]

            if img_fmt_w:
                cmd_args += ['--img_fmt_w', img_fmt_w]
        
        if overwrite:
            cmd_args += ['--overwrite']
        
    else:
        #Fit directly supplied FA and signal data using madym_T1_lite
        
        #Get a name for the temporary file we'll write input data to (we'll hold
        #off writing anything until we know this isn't a dummy run). For python
        #we'll do this differently to Matlab, using the tempdir function to create
        #an input directory we'll then cleanup at the end
        input_dir = TemporaryDirectory()
        input_file = os.path.join(input_dir.name, 'signals.dat') 

        nSamples, nScannerParams = signals.shape

        cmd_args += [
            '--data', input_file,
            '--n_T1', str(nScannerParams),
            '--TR', f'{TR:4.3f}',
            '-o', output_dir,
            '-O', output_name]
        
        #Check size off scanner params - these can either be set per voxels, in which
        #case ScannerParams should be an nSamples x nScannerParams array, or for all voxels, in
        #in which case we'll replicate them into an nSamples x nScannerParams for the
        #madym-lite input
        if ScannerParams.size == nScannerParams:
            ScannerParams = np.repeat(ScannerParams.reshape(1,nScannerParams), nSamples, 0)

        elif ScannerParams.shape == (nSamples, nScannerParams):
            raise ValueError(
                'Size of ScannerParams array does not match size of signals array')

        if method:
            cmd_args += ['-T', method]
        
        #If B1 values supplied, append them to the signals and set B1_correction
        #flag
        if B1_values is not None:
            signals = np.concatenate(
                (signals, np.atleast_1d(B1_values).reshape((nSamples,1))),
                1
            )
            cmd_args += ['--B1_correction']

        if quiet:
            cmd_args += ['--quiet']
        
        #Check for bad samples, these can screw up Madym as the lite version
        #of the software doesn't do the full range of error checks Madym proper
        #does. So chuck them out now and warn the user
        discard_samples = np.any(
            np.isnan(ScannerParams) |
            ~np.isfinite(ScannerParams) |
            np.isnan(signals) |
            ~np.isfinite(signals), 1)
        
        if np.any(discard_samples):
            warnings.warn('Samples with NaN values found,'
                'these will be set to zero for model-fitting')
            ScannerParams[discard_samples,:] = 0
            signals[discard_samples,:] = 0

        #Combine input by concatenating horizontally
        combined_input = np.concatenate((ScannerParams, signals), axis=1)

    #Args structure complete, convert to string for printing
    cmd_str = ' '.join(cmd_args)

    if dummy_run:
        #Don't actually run anything, just print the command
        print('***********************Madym-T1 dummy run **********************')
        print(cmd_str)
        
        T1 = []
        M0 = []
        error_codes = []
        result = []
        return T1, M0, error_codes, result

    #For the lite method, no we can write the temporary files
    if use_lite:
        #Write input values to a temporary file
        np.savetxt(input_file, combined_input, fmt='%6.5f')

    #At last.. we can run the command
    print('***********************Madym-T1 running **********************')
    if working_directory:
        print(f'Working directory = {working_directory}')
        
    print(cmd_str)
    result = subprocess.Popen(cmd_args, shell=False, cwd=working_directory,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while True:
        out = result.stdout.readline().decode("utf-8")
        if out == '' and result.poll() is not None:
            break
        if out:
            print(f"{out}", end='')

    if use_lite:
        #Now load the output from calculate T1 lite and extract data to match this
        #functions outputs
        full_output_path = os.path.join(output_dir, f'{method}_{output_name}')
        output_data = np.atleast_2d(np.loadtxt(full_output_path))
        T1 = output_data[:,0]
        M0 = output_data[:,1]
        error_codes = output_data[:,2]

        #Tidy up temporary input files
        input_dir.cleanup()
        
    elif return_maps:
        if working_directory is None:
            working_directory = ""

        if output_dir is None:
            output_dir = ""

        T1_path = os.path.join(working_directory,output_dir, 'T1.hdr')
        M0_path = os.path.join(working_directory,output_dir, 'M0.hdr')
        T1 = read_analyze_img(T1_path)
        M0 = read_analyze_img(M0_path)
        
        if error_name is not None:
            error_path = os.path.join(working_directory,output_dir, error_name+'.hdr')
            error_codes = read_analyze_img(error_path)
        else:
            error_codes = []
    else:
        T1 = []
        M0 = []
        error_codes = []

    if delete_output:
        #Tidy up temporary output files
        output_temp_dir.cleanup()
    
    return T1, M0, error_codes, result
##
#Test function to run if no inputs
def test(plot_output=True):
    '''
    Test the main run function on some synthetically generated T1, M0 and signal
    values
    Inputs:
        plot_output: if true, plots the fitted signals. Set to false to run on non-
        interactive settings (eg installation on CSF)
    '''
    import matplotlib.pyplot as plt
    from QbiPy.image_io.xtr_files import write_xtr_file
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
    
    if plot_output:
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

    print('Parameter estimates for calculate_T1_lite (actual,fit)')
    for i_sample in range(6):
        print(f' T1: ({T1[i_sample]}, {T1_fit[i_sample]:4.1f})')
        print(f' M0: ({M0[i_sample]}, {M0_fit[i_sample]:4.1f})')
    
    #Now save the flip-angle data at Analyze images and apply the full
    #volume method
    fa_dir = TemporaryDirectory()
    T1_vols = []
    for i_fa in range(3):
        FA_name = os.path.join(fa_dir.name, f'FA_{i_fa+1}')
        T1_vols +=  [FA_name + '.hdr']
        xtr_name = FA_name + '.xtr'

        write_analyze(signals[:,i_fa], T1_vols[i_fa])
        write_xtr_file(xtr_name, 
            FlipAngle=FAs[i_fa],
            TR=TR,
            TimeStamp=120000.0)
    
    T1_fit, M0_fit,_,_ = run(
        T1_vols = T1_vols, 
        method = 'VFA',
        noise_thresh = 0,
        img_fmt_r = 'ANALYZE',
        img_fmt_w = 'ANALYZE',
        return_maps = True,
        no_audit=True,
        overwrite = True)
    signals_fit = signal_from_T1(T1_fit, M0_fit, FAs, TR)
    
    #Clean up files
    fa_dir.cleanup()
    
    if plot_output:
        plt.figure(figsize=(16,8))
        for i_sample in range(6):
            plt.subplot(2,3,i_sample+1)
            plt.plot(FAs, signals[i_sample,:], 'go')
            plt.plot(FAs, signals_fit[i_sample,:], 'r*')
            plt.plot(FAs, signals_fit[i_sample,:], 'b-')
            plt.title('Parameter estimates (actual,fit)\n'
                f' T1: ({T1[i_sample]}, {T1_fit[0,i_sample]:4.1f}),\n'
                f' M0: ({M0[i_sample]}, {M0_fit[0,i_sample]:4.1f})')
            
            if not i_sample:
                plt.legend(['Signals', 'Fit to signals',' '])
            
            plt.xlabel('Flip angle (degrees)')
            plt.ylabel('Signal intensity')        
        plt.tight_layout()

    print('Parameter estimates for calculate_T1 (actual,fit)')
    for i_sample in range(6):
        print(f' T1: ({T1[i_sample]}, {T1_fit[0,i_sample]:4.1f})')
        print(f' M0: ({M0[i_sample]}, {M0_fit[0,i_sample]:4.1f})')
    
    return
