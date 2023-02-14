'''
Wrapper to C++ tool madym_DicomConvert
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
    T1_dir:str = None,
    DWI_vols:list = None,
    DWI_dir:str = None,
    dynamic_basename:str = None,
    dyn_dir:str = None,
    sequence_format:str = None,
    sequence_start:str = None,
    sequence_step:str = None,
    n_dyns:int = None,
    img_fmt_w:str = None,
    img_dt_type:int = None,
    nifti_scaling:bool = None,
    dicom_dir : str = None,
    dicom_series_file : str = None,
    T1_input_series : list = None,
    DWI_series : list = None,
    dyn_series : int = None,
    single_series : int = None,
    dicom_filter : str = None,
    slice_filter_tag: str = None,
    slice_filter_match_value:str = None,
    single_vol_names : list = None,
    vol_name : str = None,
    sort : bool = None,
    make_t1 : bool = None,
    make_DWI : bool = None,
    make_single : bool = None,
    make_dyn : bool = None,
    make_t1_means : bool = None,
    make_Bvalue_means : bool = None,
    make_dyn_mean : bool = None,
    flip_x : bool = None,
    flip_y : bool = None,
    flip_z : bool = None,
    scale_tag : str = None,
    offset_tag : str = None,
    dicom_scale : float = None,
    dicom_offset : float = None,
    acquisition_time_tag : str = None,
    acquisition_time_required : bool = None,
    FA_tag : str = None,
    FA_required : bool = None,
    TR_tag : str = None,
    TR_required : bool = None,
    TI_tag : str = None,
    TI_required : bool = None,
    TE_tag : str = None,
    TE_required : bool = None,
    B_tag : str = None,
    B_required : bool = None,
    grad_ori_tag : str = None,
    grad_ori_required : bool = None,
    temp_res : float = None,
    repeat_prefix : str = None,
    mean_suffix : str = None,
    overwrite:bool = None,
    program_log_name:str = None,
    audit_dir:str = None,
    audit_name:str = None,
    config_out:str = None,
    no_log:bool = None,
    no_audit:bool = None,
    quiet:bool = None,
    help:bool = None,
    version:bool = None,
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
        img_fmt_w : str = None
            Image format for writing output
        nifti_scaling:bool = None,
            If set, applies intensity scaling and offset when reading/writing NIFTI images
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
        flip_z : bool = None
            Reverse order of slices in 3D volume
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
        acquisition_time_required : bool = None,
            If set to true, throws program warning if acquisition time not found in a DICOM header
        FA_tag : str = None,
            Custom Dicom tag key (group,element) for FlipAngle, only required if protocol 
            doesn't use standard FlipAngle field (0018,1314)
        FA_required : bool = None,
            If set to true, throws program warning if FA not found in a DICOM header
        TR_tag : str = None,
            Custom Dicom tag key (group,element) for TR, only required if protocol 
            doesn't use standard RepetitionTime field (0018,0080)
        TR_required : bool = None,
            If set to true, throws program warning if TR not found in a DICOM header
        TI_tag : str = None,
            Custom Dicom tag key (group,element) for TI, only required if protocol 
            doesn't use standard InversionTime field (0018,0082)
        TI_required : bool = None,
            If set to true, throws program warning if TI not found in a DICOM header
        TE_tag : str = None,
            Custom Dicom tag key (group,element) for TE, only required if protocol 
            doesn't use standard EchoTime field (0018,0081)
        TE_required : bool = None,
            If set to true, throws program warning if TE not found in a DICOM header
        B_tag : str = None,
            Custom Dicom tag key (group,element) for diffusion B-value, only required if 
            protocol doesn't use standard DCM_DiffusionBValue field (0018,9087)
        B_required : bool = None,
            If set to true, throws program warning if B-value not found in a DICOM header
        grad_ori_tag : str = None,
            Custom Dicom tag key (group,element) for diffusion gradient orientation, 
            only required if protocol 
            doesn't use standard DCM_DiffusionGradientOrientation field (0018,9089)
        grad_ori_required : bool = None,
            If set to true, throws program warning if gradient orientation not found in a DICOM header
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
        config_out : str = None,
            Filename of output config file, will be appended with datetime
        no_log arg : bool = None,
            Switch off program logging
        no_audit : bool = None,
            Switch off audit logging
        quiet : bool = None,
            Do not display logging messages in cout
        help : bool = None,
            Display help and exit
        version : bool = None,
            Display version and exit
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
    
    #Set all the other commands
    add_option('string', cmd_args, '--config', config_file)

    add_option('string', cmd_args, '--cwd', working_directory)

    add_option('string', cmd_args, '-o', output_dir)

    add_option('string', cmd_args, '-d', dynamic_basename)

    add_option('string', cmd_args, '--dyn_dir', dyn_dir)

    add_option('string', cmd_args, '--sequence_format', sequence_format)

    add_option('int', cmd_args, '--sequence_start', sequence_start)

    add_option('int', cmd_args, '--sequence_step', sequence_step)

    add_option('int', cmd_args, '--n_dyns', n_dyns)

    add_option('string', cmd_args, '--img_fmt_w', img_fmt_w)

    add_option('int', cmd_args, '--img_dt_type', img_dt_type)

    add_option('bool', cmd_args, '--nifti_scaling', nifti_scaling)

    add_option('string_list', cmd_args, '--T1_vols', T1_vols)

    add_option('string', cmd_args, '--T1_dir', T1_dir)

    add_option('string_list', cmd_args, '--DWI_vols', DWI_vols)

    add_option('string', cmd_args, '--DWI_dir', DWI_dir)

    add_option('string', cmd_args, '--dicom_dir', dicom_dir)

    add_option('string', cmd_args, '--dicom_series', dicom_series_file)

    add_option('int_list', cmd_args, '--T1_series', T1_input_series)

    add_option('int', cmd_args, '--dyn_series', dyn_series)

    add_option('int_list', cmd_args, '--single_series', single_series)

    add_option('int_list', cmd_args, '--DWI_series', DWI_series)

    add_option('string', cmd_args, '--dicom_filter', dicom_filter)

    add_option('string', cmd_args, '--slice_filter_tag', slice_filter_tag)

    add_option('string', cmd_args, '--slice_filter_match_value', slice_filter_match_value)

    add_option('string', cmd_args, '--vol_name', vol_name)

    add_option('string_list', cmd_args, '--single_vol_names', single_vol_names)

    add_option('bool', cmd_args, '--sort', sort)

    add_option('bool', cmd_args, '--make_t1', make_t1)

    add_option('bool', cmd_args, '--make_DWI', make_DWI)

    add_option('bool', cmd_args, '--make_single', make_single)

    add_option('bool', cmd_args, '--make_dyn', make_dyn)

    add_option('bool', cmd_args, '--make_t1_means', make_t1_means)

    add_option('bool', cmd_args, '--make_dyn_mean', make_dyn_mean)

    add_option('bool', cmd_args, '--make_Bvalue_means', make_Bvalue_means)

    add_option('bool', cmd_args, '--flip_x', flip_x)

    add_option('bool', cmd_args, '--flip_y', flip_y)

    add_option('bool', cmd_args, '--flip_z', flip_z)

    add_option('string', cmd_args, '--scale_tag', scale_tag)

    add_option('string', cmd_args, '--offset_tag', offset_tag)

    add_option('float', cmd_args, '--dicom_scale', dicom_scale)

    add_option('float', cmd_args, '--dicom_offset', dicom_offset)

    add_option('string', cmd_args, '--acquisition_time_tag', acquisition_time_tag)

    add_option('bool', cmd_args, '--acquisition_time_required', acquisition_time_required)

    add_option('string', cmd_args, '--FA_tag', FA_tag)

    add_option('bool', cmd_args, '--FA_required', FA_required)

    add_option('string', cmd_args, '--TR_tag', TR_tag)

    add_option('bool', cmd_args, '--TR_required', TR_required)

    add_option('string', cmd_args, '--TI_tag', TI_tag)

    add_option('bool', cmd_args, '--TI_required', TI_required)

    add_option('string', cmd_args, '--TE_tag', TE_tag)

    add_option('bool', cmd_args, '--TE_required', TE_required)

    add_option('string', cmd_args, '--B_tag', B_tag)

    add_option('bool', cmd_args, '--B_required', B_required)

    add_option('string', cmd_args, '--grad_ori_tag', grad_ori_tag)

    add_option('bool', cmd_args, '--grad_ori_required', grad_ori_required)

    add_option('float', cmd_args, '--temp_res', temp_res)

    add_option('string', cmd_args, '--repeat_prefix', repeat_prefix)

    add_option('string', cmd_args, '--mean_suffix', mean_suffix)

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