'''
Python interface to Madym, with some associated DCE analysis functionality

Notes:
The version is read directly from the latest git repo tag, which is itself
set auto-magically by semantic-release in the project's GitLab CI/CD pipeline
'''
import os
import git

def version_from_git():
    ''' Read version from latest git tag '''
     
    #Get repo path and create repo object 
    repo_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..', '..', '..'))

    try:
        repo = git.Repo(repo_path)

        #Check tags against head
        head_idx = -1
        version = str(repo.tags[head_idx]).split('v')[1]
    except:
        version = 'unknown'
    
    return version

def version_from_file(version_path):
    with open(version_path, 'r') as f:
        v = f.readlines()

    return v[-1].strip().split('v')[1]

version_path = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'VERSION'))

if os.path.exists(version_path):
    __version__ = version_from_file(version_path)
else:
    __version__ = version_from_git()


__all__ = ['madym_AIF', 'madym_DCE', 'madym_DCE_lite', 'madym_T1', 'run_madym_tests', 'utils']