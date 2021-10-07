'''
Wrapper to C++ tool madym_DicomConvert
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
    T1_dir:str = None,
    dynamic_basename:str = None,
    sequence_format:str = None,
    sequence_start:str = None,
    sequence_step:str = None,
    n_dyns:int = 0,
    img_fmt_w:str = None,
    dicom_dir : str = None,
    dicom_series_file : str = None,
    T1_input_series : list = None,
    dyn_series : int = None,
    single_series : int = None,
    dicom_filter : str = None,
    vol_name : str = None,
    sort : bool = None,
    make_t1 : bool = None,
    make_single : bool = None,
    make_dyn : bool = None,
    make_t1_means : bool = None,
    make_dyn_mean : bool = None,
    flip_x : bool = None,
    flip_y : bool = None,
    scale_tag : str = None,
    offset_tag : str = None,
    dicom_scale : float = None,
    dicom_offset : float = None,
    acquisition_time_tag : str = None,
    temp_res : float = None,
    repeat_prefix : str = None,
    mean_suffix : str = None,
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
    dummy_run:bool = None):
    '''
    MADYM wrapper function to call C++ tool madym_DicomConvert. Converts original dicom
    image slices into 3D Nifti/Analyze format images for input to the main Madym
    analysis tools, additionally create the XTR files of meta-information 
    required by Madym.
    
    Note: This wrapper allows setting of all optional parameters the full C++ function takes.
    However rather than setting default values for the parameters in this wrapper (which python
    makes super easy and would be nice), the defaults are largely set to None (apart from boolean)
    flags, so that if they're not set in the wrapper they will use the C++ default. This is to avoid
    inconsistency between calling the C++ function from the wrapper vs directly (and also between
    the Matlab and python wrappers). If in time we find one wrapper in predominantly used, we may
    instead choose to set the defaults here (again doing this in python would be preferable due
    to the ease with which it deals with optional parameters)

    Optional inputs:
        config_file (str)
            Path to file setting options
        cmd_exe : str  default None,
        T1_vols : list default None, 
            File names of generated signal volumes to from which baseline T1 is mapped
        T1_dir : str = None,
            Folder to which T1 input volumes saved
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
        img_fmt_w : str = None
            Image format for writing output
        dicom_dir : str = None
            Folder containing DICOM data
        dicom_series_file : str = None
            Filename to/from which dicom series data is written/read
        T1_input_series : list(int) = None
            Indices of the dicom series for each T1 input
        dyn_series : int = None
            Index of the dicom series for the dynamic DCE time-series
        single_series : int = None
            Index of the dicom series for converting a generic single volume
        dicom_filter : str = None
            File filter for dicom sort (eg IM_)
        vol_name : str = None
            Output filename for converting a single dicom volume
        sort : bool = None
            "Sort the files in Dicom dir into separate series, writing out the series information")
        make_t1 : bool = None
            Make T1 input images from dicom series
        make_single : bool = None
            Make single 3D image from dicom series
        make_dyn : bool = None
            Make dynamic images from dynamic series
        make_t1_means : bool = None
            Make mean of each set of T1 input repeats
        make_dyn_mean : bool = None
            Make temporal mean of dynamic images
        flip_x : bool = None
            Flip dicom slices horizontally before copying into 3D image volume
        flip_y : bool = None
            Flip dicom slices vertically before copying into 3D image volume
        scale_tag : str = None
            Dicom tag key (group,element) for rescale slope, in hexideciaml form - for Philips this is (0x2005, 0x100e)
        offset_tag : str = None
            Dicom tag key (group,element) for rescale intercept, in hexideciaml form - for Philips this is (0x2005, 0x100d)
        dicom_scale : float = None
            Additional scaling factor applied to the dicom data
        dicom_offset : float = None
            Additional offset factor applied to the dicom data
        acquisition_time_tag : str = None
            Dicom tag key (group,element) for acquisition time, if empty uses DCM_AcquisitionTime
        temp_res : float = None
            Time in seconds between volumes in the DCE sequence, used to fill acquisition time not set in dynTimeTag
        repeat_prefix : str = None
            Prefix of image name for repeats in DICOM series, appended with sequence_format index and stored in series name folder")
        mean_suffix : str = None
            Suffix of image name for mean of repeats in DICOM series, appended to series name
        overwrite : bool = None,
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
        no_log arg : bool = None,
            Switch off program logging
        no_audit : bool = None,
            Switch off audit logging
        quiet : bool = None,
            Do not display logging messages in cout
        working_directory : str = None,
            Sets the current working directory for the system call, allows setting relative input paths for data
        dummy_run : bool = None
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

    if cmd_exe is None:
        madym_root = local_madym_root()

        if not madym_root:
            print('MADYM_ROOT not set. This could be because the'
                ' madym tools were installed in a different python/conda environment.'
                ' Please check your installation. To run from a local folder (without requiring MADYM_ROOT)'
                ' you must set the cmd_exe argument')
            raise ValueError('cmd_exe not specified and MADYM_ROOT not found.')
        
        cmd_exe = os.path.join(madym_root,'madym_DicomConvert')

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

    if T1_vols:
        #Set VFA files in the options string
        t1_str = ','.join(T1_vols)
        cmd_args += ['--T1_vols', t1_str]

    if T1_dir:
        cmd_args += ['--T1_dir', T1_dir]

    if img_fmt_w:
        cmd_args += ['--img_fmt_w', img_fmt_w]

    if dicom_dir:
        cmd_args += ['--dicom_dir', dicom_dir]

    if dicom_series_file:
        cmd_args += ['--dicom_series_file', dicom_series_file]

    if T1_input_series:
        T1_str = ','.join(str(i) for i in T1_input_series)
        cmd_args += ['--T1_series', T1_str]

    if dyn_series:
        cmd_args += ['--dyn_series', str(dyn_series)]

    if single_series:
        cmd_args += ['--single_series', str(single_series)]

    if dicom_filter:
        cmd_args += ['--dicom_filter', dicom_filter]

    if vol_name:
        cmd_args += ['--vol_name', vol_name]

    if sort is not None:
        cmd_args += ['--sort', str(int(sort))]

    if make_t1 is not None:
        cmd_args += ['--make_t1', str(int(make_t1))]

    if make_single is not None:
        cmd_args += ['--make_single', str(int(make_single))]

    if make_dyn is not None:
        cmd_args += ['--make_dyn', str(int(make_dyn))]

    if make_t1_means is not None:
        cmd_args += ['--make_t1_means', str(int(make_t1_means))]

    if make_dyn_mean is not None:
        cmd_args += ['--make_dyn_mean', str(int(make_dyn_mean))]

    if flip_x is not None:
        cmd_args += ['--flip_x', str(int(flip_x))]

    if flip_y is not None:
        cmd_args += ['--flip_y', str(int(flip_y))]

    if scale_tag:
        cmd_args += ['--scale_tag', scale_tag]

    if offset_tag:
        cmd_args += ['--offset_tag', offset_tag]

    if dicom_scale:
        cmd_args += ['--dicom_scale', f'{dicom_scale:4.3f}']

    if dicom_offset:
        cmd_args += ['--dicom_offset', f'{dicom_offset:4.3f}']

    if acquisition_time_tag:
        cmd_args += ['--acquisition_time_tag', acquisition_time_tag]

    if temp_res:
        cmd_args += ['--temp_res', f'{temp_res:4.3f}']

    if repeat_prefix:
        cmd_args += ['--repeat_prefix', repeat_prefix]

    if mean_suffix:
        cmd_args += ['--mean_suffix', mean_suffix]

    if program_log_name:
        cmd_args += ['--log', program_log_name]

    if audit_name:
        cmd_args += ['--audit', audit_name]

    if audit_dir:
        cmd_args += ['--audit_dir', audit_dir]

    if config_out:
        cmd_args += ['--config_out', config_out]

    if no_log is not None:
        cmd_args += ['--no_log', str(int(no_log))]

    if no_audit is not None:
        cmd_args += ['--no_audit', str(int(no_audit))]

    if quiet is not None:
        cmd_args += ['--quiet', str(int(quiet))]

    if error_name:
        cmd_args += ['--err', error_name]

    if overwrite is not None:
        cmd_args += ['--overwrite', str(int(overwrite))]

    #Args structure complete, convert to string for printing
    cmd_str = ' '.join(cmd_args)

    if dummy_run:
        #Don't actually run anything, just print the command
        print('***********************Madym dummy run **********************')
        print(cmd_str)
        result = []
        return result

    #Otherwise we can run the command:
    print('***********************Madym Dicom Convert running **********************')
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