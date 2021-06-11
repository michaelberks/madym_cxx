import os
import warnings
import subprocess
import numpy as np

from QbiMadym.utils import local_madym_root

def run(
    config_file:str=None,
    model:str=None, 
    output_dir:str = None,
    cmd_exe:str = None,
    no_optimise:bool = False,
    T1_vols:list = None,
    T1_method:str = None,
    dynamic_basename:str = None,
    sequence_format:str = None,
    sequence_start:str = None,
    sequence_step:str = None,
    n_dyns:int = 0,
    input_Ct:bool = False,
    output_Ct_sig:bool = False,
    output_Ct_mod:bool = False,
    T1_name:str = None,
    M0_name:str = None,
    B1_name:str = None,
    B1_scaling:float = None,
    B1_correction:bool = False,
    r1_const:float = None,
    M0_ratio:bool = False,
    TR:float = None,
    dose:float = None,
    injection_image:int = None,
    hct:float = None,
    T1_noise:float = None,
    first_image:int = None,
    last_image:int = None,
    roi_name:str = None,
    aif_name:str = None,
    aif_map:str = None,
    pif_name:str = None,
    IAUC_times:np.array = None,
    IAUC_at_peak:bool = False,
    param_names:list = None,
    init_params:np.array = None,
    fixed_params:np.array = None,
    fixed_values:np.array = None,
    relative_limit_params:np.array = None,
    relative_limit_values:np.array = None,
    init_maps_dir:str = None,
    init_map_params:np.array = None,
    residuals:str = None,
    max_iter:int = None,
    dyn_noise:bool = False,
    test_enhancement:bool = False,
    img_fmt_r:str = None,
    img_fmt_w:str = None,
    overwrite:bool = False,
    program_log_name:str = None,
    audit_dir:str = None,
    audit_name:str = None,
    config_out:str = None,
    error_name:str = None,
    no_log:bool = False,
    no_audit:bool = False,
    quiet:bool = False,
    working_directory:str = None,
    dummy_run:bool = False):
    '''
    MADYM wrapper function to call C++ tool madym_DCE. Fits
    tracer-kinetic models to DCE time-series stored in Analyze/NIFTI format images,
    saving the model parameters and modelled concentration time-series also
    in Analyze/NIFTI format. The result of the system call to madym_DCE is returned.
    
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
        no_optimise : bool default False, 
            Flag to switch off optimising, will just fit initial parameters values for model
        T1_vols : list default None, 
            File names of signal volumes to from which baseline T1 is mapped
        T1_method : str default None
            Method used for T1 mapping eg. VFA
        dynamic_basename : str default None, 
            Template name for dynamic sequences eg. dynamic/dyn_
        sequence_format : str default None, 
            Format for converting dynamic series index to string, eg %01u
        sequence_start : int default None, 
            Start index for dynamic series file names
        sequence_step : int default None, 
            Step between indexes of filenames in dynamic series
        n_dyns : int default 0, 
            Number of dynamic sequence maps to load. If <=0, loads all maps in dynamic dir matching -dyn pattern
        input_Ct : bool default False, 
            Flag specifying input dynamic sequence are concentration (not signal) maps
        output_Ct_sig : bool = False,
            Flag requesting concentration (derived from signal) are saved to output
        output_Ct_mod : bool = False, 
            Flag requesting modelled concentration maps are saved to output
        T1_name : str default None,
            Path to T1 map
        M0_name : str default None,
            Path to M0 map
        B1_name : str default None,
            Path to B1 correction map
        B1_scaling:float default None,
            Value applied to scaled values in B1 correction map
        B1_correction:bool default False,
            Flag to turn B1 correction on
        r1_const : float = None, 
            Relaxivity constant of concentration in tissue (in ms)
        M0_ratio : bool = False, 
            Flag to use ratio method to scale signal instead of supplying M0
        TR : float default None,  
            TR of dynamic series (in ms), only required if T1 method is inversion recovery
        dose : float = None, 
            Concentration dose (mmole/kg)
        injection_image : int = None, 
            Injection image
        hct : float = None, 
            Haematocrit correction
        T1_noise : float = None, 
            T1 noise threshold
        first_image : int = None, 
            First image used to compute model fit
        last_image : int = None, 
            Last image used to compute model fit
        roi_name : str = None,
           Path to ROI map
        aif_name : str = None, 
            Path to precomputed AIF if not using population AIF
        aif_map : str = None, 
            Path to precomputed AIF voxel mask if not using population AIF
        pif_name : str = None, 
            Path to precomputed PIF if not deriving from AIF
        IAUC_times : np.array = [60,90,120],
            Times (in s) at which to compute IAUC values
        IAUC_at_peak : bool default False
            Flag requesting IAUC computed at peak signal   
        param_names : list = None,
            Names of model parameters to be optimised, used to name the output parameter maps
        init_params : np.array = None,
            Initial values for model parameters to be optimised, either as single vector, or 2D array NSamples x NParams
        fixed_params : np.array = None,
            Parameters fixed to their initial values (ie not optimised)
        fixed_values : np.array = None,
            Values for fixed parameters (overrides default initial parameter values)
        relative_limit_params : np.array = None,
            Parameters with relative limits on their optimisation bounds
        relative_limit_values : np.array = None,
            Values for relative bounds, sets lower/upper bound as init param -/+ relative limit
        init_maps_dir : str = None, 
            Path to directory containing maps of parameters to initialise fit (overrides init_params)
        init_map_params : np.array = None,
            Parameters initialised from maps (if empty and init_maps_dir set, all params from maps)
        residuals : str = None, 
            Path to residuals map used to threshold new fits
        max_iter: int = None
            Maximum number of iterations to run model fit for
        dyn_noise : bool = False,
            Set to use varying temporal noise in model fit
        test_enhancement : bool = False, 
            Set test-for-enhancement flag
        img_fmt_r : str = None
            Image format for reading input
        img_fmt_w : str = None
            Image format for writing output
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
       Fitting to concentration time-series. If using a population AIF, you
       must supply a vector of dynamic times. A population AIF (PIF) is used
       if the aifName (pifName) option is left empty.
       [result] = 
           madym_DCE.run(model = "2CXM", output_dir = 'C:\QBI\mdm_analysis\')
    
     Notes:
       Tracer-kinetic models:
    
       All models available in the main MaDym and MaDym-Lite C++ tools are
       available to fit. 

       See the madym_cxx project wiki for more details:
        https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/dce_models
    
     Created: 20-Feb-2019
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk 
     Phone : +44 (0)161 275 7669 
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
        
        cmd_exe = os.path.join(madym_root,'madym_DCE')

    #Set up initial cmd string
    cmd_args = [cmd_exe]
    
    if config_file:
        cmd_args += ['-c', config_file]

    if model:
        cmd_args += ['-m', model]
    
    if output_dir:
        cmd_args += ['-o', output_dir] 

    #Set the dynamic names
    if dynamic_basename:
        cmd_args += ['--dyn', dynamic_basename]

    if sequence_format:
        cmd_args += ['--sequence_format', sequence_format]

    if sequence_start is not None:
        cmd_args += ['--sequence_start', str(sequence_start)]

    if sequence_step is not None:
        cmd_args += ['--sequence_step', str(sequence_step)]

    if n_dyns > 0:
        cmd_args += ['-n', str(n_dyns)]

    if input_Ct:
        cmd_args += ['--Ct']
        
    #T1/M0 options to load from maps
    if T1_name:
        cmd_args += ['--T1', T1_name]
                    
    #And if we're not using the ratio method, an M0 map
    if M0_ratio:
        cmd_args += ['--M0_ratio']

    if M0_name:
        cmd_args += ['--M0', M0_name]

    #T1 method to compute from signal inputs
    if T1_vols:
        #Set VFA files in the options string
        t1_str = ','.join(T1_vols)
        cmd_args += ['--T1_vols', t1_str]
        
    if T1_noise is not None:
        cmd_args += ['--T1_noise', f'{T1_noise:5.4f}'] 

    if T1_method:
        cmd_args += ['--T1_method', T1_method]
    
    #Set any other options required to convert signal to concentration
    if r1_const is not None:
        cmd_args += ['--r1', f'{r1_const:4.3f}']  

    if TR:
        cmd_args += ['--TR', f'{TR:4.3f}']  

    #Now go through all the other optional parameters, and if they've been set,
    #set the necessary option flag in the cmd string
    if B1_name:
        cmd_args += ['--B1', B1_name]

    if B1_scaling is not None:
        cmd_args += ['--B1_scaling', B1_scaling]

    if B1_correction:
        cmd_args += ['--B1_correction']
     
    if dose is not None:
        cmd_args += ['--dose', f'{dose:4.3f}']  

    if hct is not None:
        cmd_args += ['--hct', f'{hct:4.3f}']

    if output_Ct_sig:
        cmd_args += ['--Ct_sig']

    if output_Ct_mod:
        cmd_args += ['--Ct_mod']   

    if no_optimise:
        cmd_args += ['--no_opt']

    if not test_enhancement:
        cmd_args += ['--test_enh']  

    if dyn_noise:
        cmd_args += ['--dyn_noise']

    if injection_image is not None:
        cmd_args += ['--inj', str(injection_image)]

    if first_image is not None:
        cmd_args += ['--first', str(first_image)]

    if last_image is not None:
        cmd_args += ['--last', str(last_image)]

    if roi_name:
        cmd_args += ['--roi', roi_name]

    if aif_name:
        cmd_args += ['--aif', aif_name]

    if aif_map:
        cmd_args += ['--aif_map', aif_map]

    if pif_name:
        cmd_args += ['--pif', pif_name]

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

    if error_name:
        cmd_args += ['--err', error_name] 

    if IAUC_times:
        IAUC_str = ','.join(f'{i:3.2f}' for i in IAUC_times)
        cmd_args += ['--iauc', IAUC_str]

    if IAUC_at_peak:
        cmd_args += ['--iauc_peak']

    if init_maps_dir:
        cmd_args += ['--init_maps', init_maps_dir]

    if init_map_params:
        params_str = ','.join(str(p) for p in init_map_params)       
        cmd_args += ['--init_map_params', params_str]

    elif init_params:
        init_str = ','.join(f'{i:5.4f}' for i in init_params)           
        cmd_args += ['--init_params', init_str]
    
    if param_names:
        param_str = ','.join(param_names)
        cmd_args += ['--param_names', param_str]
    
    if fixed_params:
        fixed_str = ','.join(str(f) for f in fixed_params)       
        cmd_args += ['--fixed_params', fixed_str]
        
        if fixed_values:
            fixed_str = ','.join(f'{f:5.4f}' for f in fixed_values)           
            cmd_args += ['--fixed_values', fixed_str]

    if relative_limit_params:
        relative_str = ','.join(str(r) for r in relative_limit_params)       
        cmd_args += ['--relative_limit_params', relative_str]
        
        if relative_limit_values:
            relative_str = ','.join(f'{r:5.4f}' for r in relative_limit_values)           
            cmd_args += ['--relative_limit_values', relative_str]

    if residuals:
        cmd_args += ['--residuals', residuals]

    if max_iter is not None:
        cmd_args += ['--max_iter', str(max_iter)]

    if img_fmt_r:
        cmd_args += ['--img_fmt_r', img_fmt_r]

    if img_fmt_w:
        cmd_args += ['--img_fmt_w', img_fmt_w]

    if overwrite:
        cmd_args += ['--overwrite']

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
        print('Working directory = {working_directory}')
        
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
    Test the main run function on some synthetically generated Tofts model
    time-series
    Inputs:
        plot_output: if true, plots the fitted time-series. Set to false to run on non-
        interactive settings (eg installation on CSF)
    '''
    import matplotlib.pyplot as plt
    from tempfile import TemporaryDirectory
    from QbiPy.dce_models.dce_aif import Aif
    from QbiPy.dce_models.tissue_concentration import signal_to_concentration, concentration_to_signal
    from QbiPy.dce_models import tofts_model
    from QbiPy.image_io.analyze_format import read_analyze_img, write_analyze
    from QbiPy.image_io.xtr_files import write_xtr_file, mins_to_timestamp

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

    #Convert the concentrations to signals with some notional T1 values and
    #refit using signals as input
    FA = 20
    TR = 3.5
    T1_0 = 1000
    r1_const = 3.4
    S_t0 = 100
    S_tn = concentration_to_signal(
        C_tn, T1_0, S_t0, FA, TR, r1_const, injection_img)

    #Create a temporary directory where we'll run these tests, which we can then cleanup
    #easily at the end
    test_dir = TemporaryDirectory()
    dyn_dir = os.path.join(test_dir.name, 'dynamics')
    os.makedirs(dyn_dir)

    for i_dyn in range(nDyns):
        
        #Write out 1x1 concentration maps and xtr files
        Ct_name = os.path.join(dyn_dir, f'Ct_{i_dyn+1}')
        timestamp = mins_to_timestamp(t[i_dyn])

        write_analyze(np.atleast_3d(C_tn[0,i_dyn]), Ct_name + '.hdr')
        write_xtr_file(Ct_name + '.xtr', append=False,
            FlipAngle=FA,
            TR=TR,
            TimeStamp=timestamp)
        
        #Write out 1x1 signal maps and xtr files
        St_name = os.path.join(dyn_dir, f'St_{i_dyn+1}')

        write_analyze(np.atleast_3d(S_tn[0,i_dyn]), St_name + '.hdr')
        write_xtr_file(St_name + '.xtr', append=False,
            FlipAngle=FA,
            TR=TR,
            TimeStamp=timestamp)     

    T1_name = os.path.join(test_dir.name, 'T1.hdr')
    write_analyze(np.atleast_3d(T1_0), T1_name)

    ##
    #Apply Madym to concentration maps
    Ct_output_dir = os.path.join(test_dir.name, 'mdm_analysis_Ct')
    result = run(
        model = 'ETM',
        output_dir = Ct_output_dir,
        dynamic_basename = os.path.join(dyn_dir, 'Ct_'),
        input_Ct = True,
        output_Ct_sig = True,
        output_Ct_mod = True,
        img_fmt_r = 'ANALYZE',
        img_fmt_w = 'ANALYZE',
        injection_image=injection_img,
        overwrite=True)
    print(f'madym return code = {result.returncode}')

    #Load in the parameter img vols and extract the single voxel from each
    ktrans_fit = read_analyze_img(os.path.join(Ct_output_dir,'Ktrans.hdr'))[0,0]
    ve_fit = read_analyze_img(os.path.join(Ct_output_dir,'v_e.hdr'))[0,0]
    vp_fit = read_analyze_img(os.path.join(Ct_output_dir,'v_p.hdr'))[0,0]
    tau_fit = read_analyze_img(os.path.join(Ct_output_dir,'tau_a.hdr'))[0,0]
    Cs_t = np.zeros([1,100])
    Cm_t = np.zeros([1,100])
    for i_dyn in range(nDyns):
        Cs_t[0,i_dyn] = read_analyze_img(os.path.join(
                Ct_output_dir, 'Ct_sig', f'Ct_sig{i_dyn+1}.hdr'))[0,0]
        Cm_t[0,i_dyn] = read_analyze_img(os.path.join(
                Ct_output_dir, 'Ct_mod', f'Ct_mod{i_dyn+1}.hdr'))[0,0]

    model_sse = np.sum((C_tn-Cm_t)**2)

    print(f'Parameter estimation (actual, fitted concentration)')
    print(f'Ktrans: ({ktrans:3.2f}, {ktrans_fit:3.2f})')
    print(f'Ve: ({ve:3.2f}, {ve_fit:3.2f})')
    print(f'Vp: ({vp:3.2f}, {vp_fit:3.2f})')
    print(f'Tau: ({tau:3.2f}, {tau_fit:3.2f})')
    print('Model fit residuals (should be < 0.01)')
    print(f'Input concentration: {model_sse:4.3f}')
    
    if plot_output:
        plt.figure(figsize=(16,8))
        plt.suptitle('madym test applied')
        plt.subplot(1,2,1)
        plt.plot(t, C_tn.reshape(-1,1))
        plt.plot(t, Cs_t.reshape(-1,1), '--')
        plt.plot(t, Cm_t.reshape(-1,1))
        plt.legend(['C(t)', 'C(t) (output from MaDym)', 'ETM model fit'])
        plt.xlabel('Time (mins)')
        plt.ylabel('Voxel concentration')
        plt.title(f'Input C(t): Model fit SSE = {model_sse:4.3f}')
    
    #
    #Apply Madym to signal maps
    St_output_dir = os.path.join(test_dir.name, 'mdm_analysis_St')
    result = run(
        model = 'ETM',
        output_dir = St_output_dir,
        dynamic_basename = os.path.join(dyn_dir, 'St_'),
        input_Ct = False,
        output_Ct_sig = True,
        output_Ct_mod = True,
        T1_name = T1_name,
        r1_const = r1_const,
        injection_image = injection_img,
        img_fmt_r = 'ANALYZE',
        img_fmt_w = 'ANALYZE',
        overwrite = 1)
    print(f'madym return code = {result.returncode}')

    #Load in the parameter img vols and extract the single voxel from each
    ktrans_fit = read_analyze_img(os.path.join(St_output_dir, 'Ktrans.hdr'))[0,0]
    ve_fit = read_analyze_img(os.path.join(St_output_dir, 'v_e.hdr'))[0,0]
    vp_fit = read_analyze_img(os.path.join(St_output_dir, 'v_p.hdr'))[0,0]
    tau_fit = read_analyze_img(os.path.join(St_output_dir, 'tau_a.hdr'))[0,0]
    Cs_t = np.zeros([1,100])
    Cm_t = np.zeros([1,100])
    for i_dyn in range(nDyns):
        Cs_t[0,i_dyn] = read_analyze_img(os.path.join(
                St_output_dir, 'Ct_sig', f'Ct_sig{i_dyn+1}.hdr'))[0,0]
        Cm_t[0,i_dyn] = read_analyze_img(os.path.join(
                St_output_dir, 'Ct_mod', f'Ct_mod{i_dyn+1}.hdr'))[0,0]

    model_sse = np.sum((C_tn-Cm_t)**2)

    print(f'Parameter estimation (actual, fitted signal)')
    print(f'Ktrans: ({ktrans:3.2f}, {ktrans_fit:3.2f})')
    print(f'Ve: ({ve:3.2f}, {ve_fit:3.2f})')
    print(f'Vp: ({vp:3.2f}, {vp_fit:3.2f})')
    print(f'Tau: ({tau:3.2f}, {tau_fit:3.2f})')
    print('Model fit residuals (should be < 0.01)')
    print(f'Input signal: {model_sse:4.3f}')

    if plot_output:
        plt.subplot(1,2,2)
        plt.plot(t, C_tn.reshape(-1,1))
        plt.plot(t, Cs_t.reshape(-1,1), '--')
        plt.plot(t, Cm_t.reshape(-1,1))
        plt.legend(['C(t)', 'C(t) (output from MaDym)', 'ETM model fit'])
        plt.xlabel('Time (mins)')
        plt.ylabel('Voxel concentration')
        plt.title(f'Input S(t): Model fit SSE = {model_sse:4.3f}')
        plt.show()

    #Delete input and output files - just need to cleanup the temporary dir
    test_dir.cleanup()