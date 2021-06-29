'''
Wrapper to C++ tool madym_AIF
'''
import os
import warnings
import subprocess
import numpy as np

from QbiMadym.utils import local_madym_root

def run(
    config_file:str=None, 
    output_dir:str = None,
    cmd_exe:str = None,
    T1_vols:list = None,
    T1_method:str = None,
    dynamic_basename:str = None,
    sequence_format:str = None,
    sequence_start:str = None,
    sequence_step:str = None,
    n_dyns:int = 0,
    input_Ct:bool = True,
    T1_name:str = None,
    M0_name:str = None,
    B1_name:str = None,
    B1_scaling:float = None,
    B1_correction:bool = False,
    r1_const:float = None,
    M0_ratio:bool = True,
    TR:float = None,
    injection_image:int = None,
    T1_noise:float = None,
    roi_name:str = None,
    aif_map:str = None,
    aif_slices : str = None,
    aif_x_range : str = None,
    aif_y_range : str = None,
    min_T1_blood : float = None,
    peak_time : float = None,
    prebolus_noise : float = None,
    prebolus_min_images : int = None,
    select_pct : float = None,
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
    MADYM wrapper function to call C++ tool madym_AIF. Auto detects an AIF given an input
    sequence of DCE-MRI volumes in Analyze/NIFTI format.
    
    Note: This wrapper allows setting of all optional parameters the full C++ function takes.
    However rather than setting default values for the parameters in this wrapper (which python
    makes super easy and would be nice), the defaults are largely set to None (apart from boolean)
    flags, so that if they're not set in the wrapper they will use the C++ default. This is to avoid
    inconsistency between calling the C++ function from the wrapper vs directly (and also between
    the Matlab and python wrappers).

    Inputs (if none set will run the test function on synthetic data):
        config_file (str) - Path to file setting options OR

        output_dir (str) - Folder in which output maps will be saved 
            If output_dir doesn't exist, it will be created. If it exists and
            is not empty, will generate error unless 'overwrite' flag set
        cmd_exe : str  default None,

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
        input_Ct : bool default True, 
            Flag specifying input dynamic sequence are concentration (not signal) maps
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
        M0_ratio : bool = True, 
            Flag to use ratio method to scale signal instead of supplying M0
        TR : float default None,  
            TR of dynamic series (in ms), only required if T1 method is inversion recovery
        injection_image : int = None, 
            Injection image
        T1_noise : float = None, 
            T1 noise threshold
        roi_name : str = None,
           Path to ROI map
        aif_map : str = None, 
            Path to precomputed AIF voxel mask if not using population AIF
        aif_slices : str = None,
            Slices used to automatically measure AIF
        aif_x_range : str = None,
            Range of voxels to consider as AIF candidates in x-axis
        aif_y_range : str = None,
            Range of voxels to consider as AIF candidates in y-axis
        min_T1_blood : float = None
            Minimum T1 to be considered as potential blood voxel
        peak_time : float = None
            Time window post bolus for peak to arrive
        prebolus_noise : float = None
            Estimate of noise on the prebolus signal used when unable to compute
        prebolus_min_images : int = None
            Minimum number of images required to estimate prebolus noise
        select_pct : float = None
            Percentage of candidates to select
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
       
    
     Notes:
    
     Created: 14-May-2021
     Author: Michael Berks 
     Email : michael.berks@manchester.ac.uk  
     Copyright: (C) University of Manchester''' 

    if cmd_exe is None:
        madym_root = local_madym_root()

        if not madym_root:
            print('MADYM_ROOT not set. This could be because the'
                ' madym tools were installed in a different python/conda environment.'
                ' Please check your installation. To run from a local folder (without requiring MADYM_ROOT)'
                ' you must set the cmd_exe argument')
            raise ValueError('cmd_exe not specified and MADYM_ROOT not found.')
        
        cmd_exe = os.path.join(madym_root,'madym_AIF')

    #Set up initial cmd string
    cmd_args = [cmd_exe]
    
    if config_file:
        cmd_args += ['-c', config_file]
    
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

    #Now set any args that require option inputs
    if input_Ct:
        cmd_args += ['--Ct']
    
    #Check if we have pre-computed T1
    if T1_name:
        cmd_args += ['--T1', T1_name]
                      
    #And if we're not using the ratio method, an M0 map
    if M0_ratio:
        cmd_args += ['--M0_ratio'] 

    if M0_name:
        cmd_args += ['--M0', M0_name]

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

    if injection_image is not None:
        cmd_args += ['--inj', str(injection_image)]

    if roi_name:
        cmd_args += ['--roi', roi_name]

    if aif_map:
        cmd_args += ['--aif_map', aif_map]

    if aif_slices:
        cmd_args += ['--aif_slices', aif_slices]
    
    if aif_x_range:
        cmd_args += ['--aif_x_range', aif_x_range]
    
    if aif_y_range:
        cmd_args += ['--aif_y_range', aif_y_range]
    
    if min_T1_blood:
        cmd_args += ['--min_T1_blood', f'{min_T1_blood:4.3f}']
    
    if peak_time:
        cmd_args += ['--peak_time', f'{peak_time:4.3f}']
    
    if prebolus_noise:
        cmd_args += ['--prebolus_noise', f'{prebolus_noise:4.3f}']
    
    if prebolus_min_images:
        cmd_args += ['--prebolus_min_images', str(prebolus_min_images)]
    
    if select_pct:
        cmd_args += ['--select_pct', f'{select_pct:4.3f}']

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
    print('***********************Madym AIF running **********************')
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