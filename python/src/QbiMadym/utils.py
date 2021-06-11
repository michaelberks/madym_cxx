''' 
# Created: 01-May-2019
# Author: Michael Berks 
# Email : michael.berks@manchester.ac.uk 
# Phone : +44 (0)161 275 7669 
# Copyright: (C) University of Manchester
'''
import os
import platform
import tkinter as tk
from tkinter import filedialog, messagebox
import warnings
import subprocess
from shutil import copyfile

#-------------------------------------------------------------------------------------------
def latest_madym_version():
    ''' LATEST_MADYM_VERSION return the most current madym version available a string. MB has
    responsibility for maintaining. Note this may NOT be the version you have
    installed locally. Call local_madym_version to check that.
       [version] = latest_madym_version()
    
     Outputs:
          version (str) - version in v(Major).(Minor).(Patch) form of latest
          madym version available on the GitLab madym_cxx repo
    
     Example: [version] = latest_madym_version()
    
     Notes:
    
     See also: LOCAL_MADYM_VERSION
    
    '''
    #Not really best practice to have impor tin function, but gitlab
    #module doesn't port with Anaconda and here is the only place
    #we need it
    try:
        import gitlab
    except ImportError as error:
        # Output expected ImportErrors.
        print("latest_madym_version requires python module gitlab installed.")
        print("To install run: pip install --upgrade python-gitlab")
        print("See https://python-gitlab.readthedocs.io/en/stable/install.html for details")
        print(error.__class__.__name__ + ": " + error.message)
        return

    gl = gitlab.Gitlab('https://gitlab.com/')
    project = gl.projects.list(search='madym_cxx')[0]
    version = project.tags.list()[0].name.rstrip()
            
    print(f'Latest Madym version on GitLab is {version}')
    return version

#--------------------------------------------------------------------------------
def local_madym_root(empty_check:bool=True):
    '''LOCAL_MADYM_ROOT customises a path based on users local machine
       [madym_root] = local_madym_root
    
     Inputs:
           empty_check (bool - True) - if true warns if madym root not set
    
     Outputs:
          madym_root - Path to the madym binaries on this machine
    
    
     Example: cmd_exe = [local_madym_root 'madym_lite']
    
     Notes: Please set your local root in an environment variable MADYM_ROOT.
     You can do this either by setting a system (or user) wide environment 
     variable, or (the easier and preferred way), adding a line
     SETENV('MADYM_ROOT', '<replace_with_your_local_path>') into your Matlab
     startup script. You don't have a startup script? Set one up now!! o)
    '''

    madym_root = os.getenv('MADYM_ROOT')
    if empty_check and not madym_root:
        warnings.warn(f'MADYM_ROOT environment variable not set.')
   
    return madym_root

#--------------------------------------------------------------------------------
def local_madym_version(cmd_exe:str=None):
    '''LOCAL_MADYM_VERSION return version of madym installed on this machine
       [version] = local_madym_version()
    
     Inputs:
           cmd_exe (str - []) - can be set if non-default installation of
           madym, otherwise defaults to madym_lite at local_madym_root
    
     Outputs:
           version (str) - Madym version in (Major).(Minor).(Patch)
    
     Example:
    
     Notes:
    '''
    if cmd_exe is None:
        madym_root = local_madym_root(False)

        if not madym_root:
            print('MADYM_ROOT not set. This could be because the'
                ' madym tools were installed in a different python/conda environment.'
                ' Please check your installation. To run from a local folder (without requiring MADYM_ROOT)'
                ' you must set the cmd_exe argument')
            raise ValueError('cmd_exe not specified and MADYM_ROOT not found.')

        cmd_exe = os.path.join(madym_root, 'madym_DCE_lite')

    result = subprocess.run([cmd_exe, '-v'], 
        shell=False, 
        stderr=subprocess.PIPE, 
        stdout=subprocess.PIPE, 
        encoding='utf-8')
    version = result.stdout.rstrip()
    return version

#-------------------------------------------------------------------------------------
def set_madym_root(madym_root:str=None, 
    add_to_conda_env:bool = True, add_to_bashrc:bool = None, add_to_bash_profile:bool = None):
    '''
    SET_MADYM_ROOT sets customised path to Madym executables on users local machine
       [] = local_madym_root(madym_root)
    
     Inputs:
        madym_root (str-[]) - Path to the madym binaries on this machine. If
          empty, will generate a UI fial dialog to choose.

        add_to_conda_env : bool, default True
            Write an entry in an environemnt variable activation script for the current conda
            environment (if one activated), creating the script in $CONDA_PREFIX/etc/conda/activate.d
            if it doesn't already exist. Also unsets the variable in the decativation scripts.
        add_to_bashrc : bool, default None (linux only)
            If true, add an entry to .bashrc, so MADYM_ROOT is set for all terminals. If None will
            prompt for user input
        add_to_bash_profile : bool, default None (MacOS only)
            If true, add an entry to .bash_profile, so MADYM_ROOT is set for all terminals. If None will
            prompt for user input

        
    
     Outputs:
    
    
     Example: set_madym_root('~code/cxx_bin/madym')
    
     Notes:
    '''
    #If user hasn't supplied madym_root, get one from file dialog
    if madym_root is None:
        root = tk.Tk()
        root.withdraw()
        madym_root = filedialog.askdirectory(
            title='Select location for madym tools'
        )
        root.destroy()    
        if not madym_root:
            return

    #First check if madym root already exists
    old_madym_root = local_madym_root(False)

    if old_madym_root:
        if madym_root == old_madym_root:
            print(f'Madym root already set to {madym_root}, nothing to do')
            return
        else:
            root = tk.Tk()
            root.withdraw()
            replace = messagebox.askyesno(
                'Madym root already set', f'Replace {old_madym_root} as madym root?')
            root.destroy()
            if not replace:
                print(f'Leaving madym root already set as {old_madym_root}')
                return

    #Set scripting commands for platform
    operating_system = platform.system()
    is_windows = operating_system == 'Windows'
    if is_windows:
        script_ext = '.bat'
        set_cmd = 'set'
        unset_cmd_pre = 'set'
        unset_cmd_post = '='
    else: #For linux/mac should be ok, but what about other platforms?
        script_ext = '.sh'
        set_cmd = 'export'
        unset_cmd_pre = 'unset'
        unset_cmd_post = ''
    
    if add_to_conda_env:
        #Get conda installation location and paths to activate/deactivate dirs
        conda_prefix = os.getenv('CONDA_PREFIX')
        if conda_prefix:
            activate_dir = os.path.join(conda_prefix, 'etc', 'conda', 'activate.d')
            deactivate_dir = os.path.join(conda_prefix, 'etc', 'conda', 'deactivate.d')
            os.makedirs(activate_dir, exist_ok=True)
            os.makedirs(deactivate_dir, exist_ok=True)
            
            #Set paths to activate scripts
            activate_env_path = os.path.join(activate_dir, 'env_vars'+script_ext)
            deactivate_env_path = os.path.join(deactivate_dir, 'env_vars'+script_ext)

            #For linux/mac, need to make sure script env is set at start of script
            if  not is_windows:
                if not os.path.exists(activate_env_path):
                    with open(activate_env_path, "w") as env_file:
                        print(f"#!/bin/sh", file=env_file)

                if not os.path.exists(deactivate_env_path):
                    with open(deactivate_env_path, "w") as env_file:
                        print(f"#!/bin/sh", file=env_file)

            #Append activate script to set MADYM_ROOT environment variable
            with open(activate_env_path, "a") as env_file:
                print(f"{set_cmd} MADYM_ROOT={madym_root}", file=env_file)

            #Append deactivate script to remove MADYM_ROOT environment variable
            with open(deactivate_env_path, "a") as env_file:
                print(f"{unset_cmd_pre} MADYM_ROOT{unset_cmd_post}", file=env_file)
        else:
            warnings.warn('Can''t add MADYM_ROOT to conda environment'
                '$CONDA_PREFIX not set, suggesting no conda environment is activated.'
            )

    if operating_system == 'Linux':
        if add_to_bashrc is None:
            root.withdraw()
            add_to_bashrc = messagebox.askyesno(
                'Add Madym root to .bashrc', 
                'Do you want to add MADYM_ROOT to .bashrc?'
                'This will ensure it is set for all terminal sessions')
            root.destroy()
        
        if add_to_bashrc:
            with open(os.path.expanduser('~/.bashrc'), "a") as bash_file:
                print(f"{set_cmd} MADYM_ROOT={madym_root}", file=bash_file)

    if operating_system == 'Darwin':
        if add_to_bash_profile is None:
            root.withdraw()
            add_to_bash_profile = messagebox.askyesno(
                'Add Madym root to .bash_profile', 
                'Do you want to add MADYM_ROOT to .bash_profile?'
                'This will ensure it is set for all terminal sessions')
            root.destroy()
        
        if add_to_bash_profile:
            with open(os.path.expanduser('~/.bash_profile'), "a") as bash_file:
                print(f"{set_cmd} MADYM_ROOT={madym_root}", file=bash_file)
            with open(os.path.expanduser('~/.zprofile'), "a") as zsh_file:
                print(f"{set_cmd} MADYM_ROOT={madym_root}", file=zsh_file)

    #Set the variable for this session (so we don't need to reactivate)
    os.environ['MADYM_ROOT'] = madym_root
    print(f'New madym root set to {madym_root}')

#----------------------------------------------------------------------------
def check_madym_updates():
    '''CHECK_MADYM_UPDATES checks whether the local madym version matches the 
     latest version available from the share drive.
       [] = check_madym_updates()
    
     Outputs:
          up_to_date (bool) - true if local version matched GitLab madym_cxx latest
          version, false otherwise
    
     Example: [version] = check_madym_updates();'''
    latest_version = latest_madym_version()
    local_version = local_madym_version()

    up_to_date = latest_version == local_version

    if up_to_date:
        print(f'Your madym version {local_version} is up-to-date')
    else:
        print(
            f'Your madym version {local_version} does not match the latest available version {latest_version}')
        print('Run install_madym to update')
    






