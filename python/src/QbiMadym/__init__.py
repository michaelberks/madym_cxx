'''
Python interface to Madym, with some associated DCE analysis functionality

Notes:
The version is read directly from the latest git repo tag, which is itself
set auto-magically by semantic-release in the project's GitLab CI/CD pipeline
'''

import git
repo = git.Repo('.')

__version__= str(repo.tags[-1]).split('v')[1]
__all__ = ['madym_AIF', 'madym_DCE', 'madym_DCE_lite', 'madym_T1', 'run_madym_tests', 'utils']