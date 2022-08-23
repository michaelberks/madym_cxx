'''
Wrapper to C++ tool madym_AIF
'''
import os
import warnings
import subprocess
import numpy as np

from QbiMadym.utils import local_madym_root, add_option

def run(
    config_file:str=None, 
    output_dir:str = None,
    cmd_exe:str = None,
    T1_vols:list = None,
    T1_method:str = None,
    dynamic_basename:str = None,
    dyn_dir:str = None,
    sequence_format:str = None,
    sequence_start:str = None,
    sequence_step:str = None,
    n_dyns:int = None,
    input_Ct:bool = None,
    T1_name:str = None,
    M0_name:str = None,
    B1_name:str = None,
    B1_scaling:float = None,
    B1_correction:bool = None,
    r1_const:float = None,
    M0_ratio:bool = None,
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
    nifti_scaling:bool = None,
    nifti_4D:bool = None,
    overwrite:bool = None,
    program_log_name:str = None,
    audit_dir:str = None,
    audit_name:str = None,
    config_out:str = None,
    error_name:str = None,
    no_log:bool = None,
    no_audit:bool = None,
    quiet:bool = None,
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
        dyn_dir : str default None, 
            Folder containing dynamic volumes, can be omitted if inlcuded in dynamic_basename
        sequence_format : str default None, 
            Format for converting dynamic series index to string, eg %01u
        sequence_start : int default None, 
            Start index for dynamic series file names
        sequence_step : int default None, 
            Step between indexes of filenames in dynamic series
        n_dyns : int default 0, 
            Number of dynamic sequence maps to load. If <=0, loads all maps in dynamic dir matching -dyn pattern
        input_Ct : bool default None, 
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
        M0_ratio : bool = None, 
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
        nifti_scaling:bool = None,
            If set, applies intensity scaling and offset when reading/writing NIFTI images
        nifti_4D : bool = None,
            If set, reads NIFTI 4D images for T1 mapping and dynamic inputs
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
    
    #   Check if a config file exists
    add_option('string', cmd_args, '--config', config_file)    

    #   Set the working dir
    add_option('string', cmd_args, '--cwd', working_directory)    

    #   Set output directory
    add_option('string', cmd_args, '-o', output_dir)    

    #   Set the dynamic names
    add_option('string', cmd_args, '-d', dynamic_basename)    

    add_option('string', cmd_args, '--dyn_dir', dyn_dir)    

    #   Set the format
    add_option('string', cmd_args, '--sequence_format', sequence_format)    

    add_option('int', cmd_args, '--sequence_start', sequence_start)    

    add_option('int', cmd_args, '--sequence_step', sequence_step)    

    #   Set the number of dynamics
    add_option('int', cmd_args, '--n_dyns', n_dyns)    

    #   Set image formats
    add_option('string', cmd_args, '--img_fmt_r', img_fmt_r)    

    add_option('string', cmd_args, '--img_fmt_w', img_fmt_w)   

    add_option('bool', cmd_args, '--nifti_scaling', nifti_scaling)

    add_option('bool', cmd_args, '--nifti_4D', nifti_4D)

    add_option('bool', cmd_args, '--Ct', input_Ct) 

    add_option('string', cmd_args, '--T1', T1_name)

    add_option('string', cmd_args, '--M0', M0_name)

    add_option('string_list', cmd_args, '--T1_vols', T1_vols)    
        
    add_option('float', cmd_args, '--T1_noise', T1_noise)    
        
    add_option('string', cmd_args, '--T1_method', T1_method)
    
    add_option('float', cmd_args, '--r1', r1_const)    

    add_option('bool', cmd_args, '--M0_ratio', M0_ratio)    

    #B1 correction options
    add_option('string', cmd_args, '--B1', B1_name)    

    add_option('bool', cmd_args, '--B1_correction', B1_correction)    

    add_option('float', cmd_args, '--B1_scaling', B1_scaling)    

    #Now go through all the other optional parameters, and if they've been set,
    #set the necessary option flag in the cmd string
    add_option('bool', cmd_args, '--overwrite', overwrite)    

    add_option('bool', cmd_args, '--no_audit', no_audit)    

    add_option('bool', cmd_args, '--no_log', no_log)    

    add_option('bool', cmd_args, '--quiet', quiet)    

    add_option('int', cmd_args, '-i', injection_image)    

    add_option('string', cmd_args, '--roi', roi_name)    

    add_option('string', cmd_args, '--aif_map', aif_map)    

    add_option('float', cmd_args, '--TR', TR)    

    add_option('string', cmd_args, '--aif_slices', aif_slices)    

    add_option('string', cmd_args, '--aif_x_range', aif_x_range)    

    add_option('string', cmd_args, '--aif_y_range', aif_y_range)    

    add_option('float', cmd_args, '--min_T1_blood', min_T1_blood)    

    add_option('float', cmd_args, '--peak_time', peak_time)    

    add_option('float', cmd_args, '--prebolus_noise', prebolus_noise)    

    add_option('int', cmd_args, '--prebolus_min_images', prebolus_min_images)    

    add_option('float', cmd_args, '--select_pct', select_pct)    

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
    print('***********************Madym AIF running **********************')
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