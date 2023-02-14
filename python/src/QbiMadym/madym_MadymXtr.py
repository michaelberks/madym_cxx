'''
Wrapper to C++ tool madym_DicomConvert
'''
import os
from tempfile import TemporaryDirectory
import warnings
import subprocess
import numpy as np

from QbiMadym.utils import local_madym_root, add_option

def run(
    config_file:str=None, 
    cmd_exe:str = None,
    T1_method:str = None,
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
    make_t1 : bool = None,
    make_dyn : bool = None,
    temp_res : float = None,
    TR : float = None,
    FA : float = None,
    VFAs : list = None,
    TIs : list = None,
    Bvalues : list = None,
    dyn_times:np.array = None,
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
            File names of signal volumes from which baseline T1 is mapped
        T1_dir : str = None,
            Folder to which T1 input volumes saved
        DWI_vols : list default None, 
            File names of signal volumes for DWI models
        DWI_dir : str = None,
            Folder to which DWI input volumes saved
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
        make_t1 : bool = None
            Make T1 input images from dicom series
        make_dyn : bool = None
            Make dynamic images from dynamic series
        temp_res : float = None
            Time in seconds between volumes in the DCE sequence, used to fill acquisition time not set in dynTimeTag
        TR : float = None,
            Repetition time
        FA : float = None,
            Flip-angle of dynamic series
        VFAs : list = None,
            List of flip-angles for baseline T1 mapping images
        TIs : list = None,
            List of inversion times for baseline T1 mapping images
        Bvalues : list = None,
            List of B-values for DWI signal images
        dyn_times:np.array = None,
            Time associated with each dynamic signal (in mins)
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
    
    add_option('string', cmd_args, '--config', config_file)

    add_option('string', cmd_args, '--cwd', working_directory)

    add_option('string', cmd_args, '--T1_method', T1_method)

    add_option('string_list', cmd_args, '--T1_vols', T1_vols)

    add_option('string', cmd_args, '--T1_dir', T1_dir)

    add_option('string_list', cmd_args, '--DWI_vols', DWI_vols)

    add_option('string', cmd_args, '--DWI_dir', DWI_dir)

    add_option('string', cmd_args, '-d', dynamic_basename)

    add_option('string', cmd_args, '--dyn_dir', dyn_dir)

    add_option('string', cmd_args, '--sequence_format', sequence_format)

    add_option('int', cmd_args, '--sequence_start', sequence_start)

    add_option('int', cmd_args, '--sequence_step', sequence_step)

    add_option('int', cmd_args, '--n_dyns', n_dyns)

    add_option('bool', cmd_args, '--make_t1', make_t1)

    add_option('bool', cmd_args, '--make_dyn', make_dyn)

    add_option('float', cmd_args, '--temp_res', temp_res)

    add_option('float', cmd_args, '--TR', TR)

    add_option('float', cmd_args, '--FA', FA)

    add_option('float_list', cmd_args, '--VFAs', VFAs)

    add_option('float_list', cmd_args, '--TI', TIs)

    add_option('float_list', cmd_args, '--Bvalues', Bvalues)

    add_option('bool', cmd_args, '--help', help)

    add_option('bool', cmd_args, '--version', version)

    if dyn_times is not None:
        #Get a name for the temporary file we'll write times to (we'll hold
        #off writing anything until we know this isn't a dummy run
        t_dir = TemporaryDirectory()
        dyn_times_file = os.path.join(t_dir.name, 'dyn_times.dat')
        add_option('string', cmd_args, '-t', dyn_times_file)

    #Args structure complete, convert to string for printing
    cmd_str = ' '.join(cmd_args)

    if dummy_run:
        #Don't actually run anything, just print the command
        print('***********************Madym dummy run **********************')
        print(cmd_str)
        result = []
        return result

    #Write dynamic times to a temporary file if we need to
    if dyn_times is not None:
        np.savetxt(dyn_times_file, dyn_times, fmt='%6.5f')

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

    if dyn_times is not None:
        t_dir.cleanup()

    #Given we don't actually need to process the result, it might be better here
    #to throw a warning on non-zero return, and just let the caller decide what to
    #do? In madym-lite we throw an exception because if madym didn't execute proeprly
    #we can't load in and collate the model parameters etc, but that' snot an issue
    #here.
    if result.returncode:        
        warnings.warn(f'madym failed to execute, returning code {result.returncode}.')

    #Return the result structure
    return result