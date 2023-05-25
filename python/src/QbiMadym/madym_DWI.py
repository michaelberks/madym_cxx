'''
Wrapper to C++ tool madym_DCE
'''
import os
import warnings
import subprocess
import numpy as np

from QbiMadym.utils import local_madym_root, add_option

def run(
    config_file:str=None,
    model:str=None, 
    output_dir:str = None,
    cmd_exe:str = None,
    DWI_vols:list = None,
    Bvals_thresh:list = None,
    roi_name:str = None,
    img_fmt_r:str = None,
    img_fmt_w:str = None,
    nifti_scaling:bool = None,
    nifti_4D:bool = None,
    use_BIDS:bool = None,
    voxel_size_warn_only:bool = None,
    overwrite:bool = None,
    program_log_name:str = None,
    audit_dir:str = None,
    audit_name:str = None,
    config_out:str = None,
    error_name:str = None,
    no_log:bool = None,
    no_audit:bool = None,
    quiet:bool = None,
    help:bool = None,
    version:bool = None,
    working_directory:str = None,
    dummy_run:bool = False):
    '''
    MADYM wrapper function to call C++ tool madym_DWI. Fits
    diffusion models to signal images acquired a varying B-values
    stored in Analyze/NIFTI format images,
    saving the model parameters also in Analyze/NIFTI format. 
    The result of the system call to madym_DWI is returned.
    
    Note: This wrapper allows setting of all optional parameters the full C++ function takes.
    However rather than setting default values for the parameters in this wrapper (which python
    makes super easy and would be nice), the defaults are largely set to None (apart from boolean)
    flags, so that if they're not set in the wrapper they will use the C++ default. This is to avoid
    inconsistency between calling the C++ function from the wrapper vs directly (and also between
    the Matlab and python wrappers). If in time we find one wrapper in predominantly used, we may
    instead choose to set the defaults here (again doing this in python would be preferable due
    to the ease with which it deals with optional parameters)

    Mandatory inputs (if not set will run the test function on synthetic data):
        config_file (str) - Path to file setting options OR

        model (str) - Model to fit, specified by its name in CAPITALS,
            see notes for options

        output_dir (str) - Folder in which output maps will be saved 
        If output_dir doesn't exist, it will be created. If it exists and
        is not empty, will generate error unless 'overwrite' flag set
    
    Optional inputs:
        cmd_exe : str  default None,
        DWI_vols: list = None
            File names of signal volumes to from which baseline T1 is mapped
        Bvals_thresh:list = None
            Thresholds used in IVIM fitting
        roi_name : str = None,
           Path to ROI map
        img_fmt_r : str = None
            Image format for reading input
        img_fmt_w : str = None
            Image format for writing output
        nifti_scaling:bool = None,
            If set, applies intensity scaling and offset when reading/writing NIFTI images
        nifti_4D : bool = None,
            If set, reads NIFTI 4D images for DWI input
        use_BIDS : bool = None,
            If set, writes images using BIDS json meta info
        voxel_size_warn_only : bool = None
            If true, only warn if voxel sizes don't match for subsequent images
        overwrite : bool = False,
            Set overwrite existing analysis in output dir
        program_log_name : str = None, 
            Program log file name
        audit_dir : str = None,
            Folder in which audit output is saved
        audit_name : str = None, 
            Audit file name
        error_name : str = None,
            Error codes image file name
        config_out : str = None,
            Filename of output config file, will be appended with datetime
        no_log arg : bool = False,
            Switch off program logging
        no_audit : bool = False,
            Switch off audit logging
        quiet : bool = False,
            Do not display logging messages in cout
        help : bool = None,
            Display help and exit
        version : bool = None,
            Display version and exit
        working_directory : str = None,
            Sets the current working directory for the system call, allows setting relative input paths for data
        dummy_run : bool = False
            Don't run any thing, just print the cmd we'll run to inspect
    
     Outputs:
          [result] returned by the system call to the Madym executable.
          These may be operating system dependent, however status=0 should
          mean an error free operation. If status is non-zero an error has
          probably occurred, the result of which may be set in result. In any
          event, it is best to check the output_dir, and any program logs that
          have been saved there.
    
     Examples:
    
     Notes:
    
     Created: 02-Feb-2021
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk  
     Copyright: (C) University of Manchester''' 

    if model is None and config_file is None:
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
        
        cmd_exe = os.path.join(madym_root,'madym_DWI')

    #Set up initial cmd string
    cmd_args = [cmd_exe]
    
    #Check if a config file exists
    add_option('string', cmd_args, '--config', config_file)

    #Set the working dir
    add_option('string', cmd_args, '--cwd', working_directory)

    #Set output directory
    add_option('string', cmd_args, '-o', output_dir)

    add_option('string', cmd_args, '--roi', roi_name)

    #Set image formats
    add_option('string', cmd_args, '--img_fmt_r', img_fmt_r)

    add_option('string', cmd_args, '--img_fmt_w', img_fmt_w)

    add_option('bool', cmd_args, '--nifti_scaling', nifti_scaling)

    add_option('bool', cmd_args, '--nifti_4D', nifti_4D)

    add_option('bool', cmd_args, '--use_BIDS', use_BIDS)

    add_option('bool', cmd_args, '--voxel_size_warn_only', voxel_size_warn_only)

    add_option('string_list', cmd_args, '--DWI_vols', DWI_vols)
        
    add_option('string', cmd_args, '--DWI_model', model)

    add_option('float_list', cmd_args, '--Bvals_thresh', Bvals_thresh)

    #Now go through all the other optional parameters, and if they've been set,
    #set the necessary option flag in the cmd string
    add_option('bool', cmd_args, '--overwrite', overwrite)

    add_option('bool', cmd_args, '--no_audit', no_audit)

    add_option('bool', cmd_args, '--no_log', no_log)

    add_option('bool', cmd_args, '--quiet', quiet)

    add_option('bool', cmd_args, '--help', help)

    add_option('bool', cmd_args, '--version', version)

    add_option('string', cmd_args, '--program_log', program_log_name)

    add_option('string', cmd_args, '--audit', audit_name)

    add_option('string', cmd_args, '--audit_dir', audit_dir)

    add_option('string', cmd_args, '--config_out', config_out)

    add_option('string', cmd_args, '-E', error_name)

    #Args structure complete, convert to string for printing
    cmd_str = ' '.join(cmd_args)

    if dummy_run:
        #Don't actually run anything, just print the command
        print('***********************Madym dummy run **********************')
        print(cmd_str)
        result = []
        return result

    #Otherwise we can run the command:
    print('***********************Madym DCE running **********************')
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

    #Given we don't actually need to process the result, it might be better here
    #to throw a warning on non-zero return, and just let the caller decide what to
    #do? In madym-lite we throw an exception because if madym didn't execute proeprly
    #we can't load in and collate the model parameters etc, but that' snot an issue
    #here.
    if result.returncode:        
        warnings.warn(f'madym failed to execute, returning code {result.returncode}.')

    #Return the result structure
    return result

#--------------------------------------------------------------------------
def test(plot_output=True):
    '''
    Test the main run function on some synthetically DWI signal series
    Inputs:
        plot_output: if true, plots the fitted time-series. Set to false to run on non-
        interactive settings (eg installation on CSF)
    '''
    import matplotlib.pyplot as plt
    from tempfile import TemporaryDirectory
    from QbiPy.image_io.analyze_format import read_analyze_img, write_analyze
    from QbiPy.image_io.xtr_files import write_xtr_file

    #Run tests for ADC and IVIM models
    sigma = 1
    B_vals = np.array([0.0, 20.0, 40.0, 60.0, 80.0, 100.0, 300.0, 500.0, 800.0])
    nBvals = B_vals.size

    #Generate ADC test data with Rician noise added
    S0 = 100
    ADC = 0.8e-3
    signals = ADC_model(B_vals, S0, ADC)
    signals_n = add_rician_noise(signals, sigma)

    #Write these signals to Analyze images
    test_dir = TemporaryDirectory()
    DWI_dir = os.path.join(test_dir.name, 'DWI_test')
    os.makedirs(DWI_dir)

    B_val_names = []
    for i_b in range(nBvals):
        
        #Write out 1x1 concentration maps and xtr files
        Bval_name = os.path.join(DWI_dir, f'Bval_{i_b}')
        B_val_names += [Bval_name + '.hdr']

        write_analyze(np.atleast_3d(signals_n[i_b]), Bval_name + '.hdr')
        write_xtr_file(Bval_name + '.xtr', append=False,
            B=B_vals[i_b])  

    #Use madym lite to fit this data
    run(
        model = 'ADC', 
        DWI_vols = B_val_names,
        img_fmt_r = 'ANALYZE',
        img_fmt_w = 'ANALYZE',
        output_dir = DWI_dir,
        no_audit = True,
        overwrite = True)

    S0_f = read_analyze_img(os.path.join(DWI_dir, 'S0.hdr'))[0,0]
    ADC_f = read_analyze_img(os.path.join(DWI_dir, 'ADC.hdr'))[0,0]

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

    #Write these signals to Analyze images
    B_val_names = []
    for i_b in range(nBvals):
        
        #Write out 1x1 concentration maps and xtr files
        Bval_name = os.path.join(DWI_dir, f'Bval_{i_b}')
        B_val_names += [Bval_name + '.hdr']

        write_analyze(np.atleast_3d(signals_n[i_b]), Bval_name + '.hdr')
        write_xtr_file(Bval_name + '.xtr', append=False,
            B=B_vals[i_b])  

    #Use madym lite to fit this data
    run(
        model = 'IVIM', 
        DWI_vols = B_val_names,
        Bvals_thresh = Bvals_thresh,
        img_fmt_r = 'ANALYZE',
        img_fmt_w = 'ANALYZE',
        output_dir = DWI_dir,
        no_audit = True,
        overwrite = True)

    S0_f = read_analyze_img(os.path.join(DWI_dir, 'S0.hdr'))[0,0]
    D_f = read_analyze_img(os.path.join(DWI_dir, 'd.hdr'))[0,0]
    f_f = read_analyze_img(os.path.join(DWI_dir, 'f.hdr'))[0,0]
    D_star_f = read_analyze_img(os.path.join(DWI_dir, 'dstar.hdr'))[0,0]
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