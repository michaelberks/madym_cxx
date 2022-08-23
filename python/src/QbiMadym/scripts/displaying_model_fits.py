'''
This script demonstrates visually inspecting DCE fits from madym.

We show how to do this using QbiPy's data loading function get_dyn_vals.

We also show how to use QbiPy's DCE_fit_viewer tool for showing
modelled concentration time-series for individual voxels. The tool
can be run directly from the command line by specifying:
    1) The name of the folder containing the output from a call to madym_DCE. Note
    madym_DCE must have been run with both Ct_sig and Ct_mod set to true, so that it
    saved a time-series of images for both signal derived and modelled
    contrast agent concentration

    2) The format of the indexing for the time-series naming. This should match
    the sequence_format option used in the madym_DCE call. This will typically be
    01d, 02d, 03d etc. For example, 02d denotes a naming dyn_01.hdr, dyn_02.hdr etc

    3) The extension type of the output madym images - either .hdr, .nii or .nii.gz

So for example, from a terminal cd'd to the location of your data, call:

python -m QbiPy.tools.DCE_fit_viewer.DCE_fit_viewer_tool test_dataset/madym_output/ETM_auto 01d .hdr

In this script, we generate an application window an instace of the 
DCE_fit_viewer_tool object manually. It can be run either interactively
or as a regualr python script from the command line, in which case the 
first argument should be path to the folder containing the Madym examples data.

IMPORTANT: To run this script, please first run the run_examples_test_dataset.py
script. This will check Madym is installed properly on your system, and we 
will use the outputs of that script here.
'''
#%%
import sys
import os

#%%
#Set path to the dataset - set this to the examples folder in which the
#test_dataset and config files are stored. Alternatively leave data_dir empty and 
# cd into your examples folder. If you call this script from the command
#line you can pass this as the first argument
if len(sys.argv) > 0:
    data_dir = sys.argv[1]
else:
    data_dir = ''

DCE_dir = os.path.join(data_dir, 'test_dataset', 'madym_output', 'ETM_auto')

# %%
# An alternative way to display fits without using the tools is to use
# QbiPy's data loading functions to load in fit data directly. We do this
#first because running the tool (a QApplication) from inside a notebook
#can have funny affects on subsequent cells
import matplotlib.pyplot as plt
import numpy as np
from QbiPy.dce_models.data_io import get_dyn_vals

#Load the signal-derived and modelled concentration time-series
Ct_sig_root = os.path.join(DCE_dir, 'Ct_sig', 'Ct_sig')
Ct_mod_root = os.path.join(DCE_dir, 'Ct_mod', 'Ct_mod')
roi_path = os.path.join(DCE_dir, 'ROI.hdr')
Ct_sig = get_dyn_vals(Ct_sig_root, 75, roi_path, '01d') # (294, 75) array
Ct_mod = get_dyn_vals(Ct_mod_root, 75, roi_path, '01d') # (294, 75) array

#Plot the first 6 voxels
rows = 2
cols = 3
print(f'Showing model fits for the first {rows*cols} voxels...')
fig, axs = plt.subplots(rows,cols)
for vox in range(rows*cols):
    ax = axs[vox // cols, vox % cols]
    h1, = ax.plot(Ct_sig[vox,], 'b-', linewidth = 3)
    h2, = ax.plot(Ct_mod[vox,], 'r--', linewidth = 3)
    ax.set_xlabel('Dynamic sequence')
    ax.set_ylabel('C(t)')
    ax.set_title(f'Voxel {vox+1}')
    
plt.legend((h1, h2), ['C(t) sig.', 'C(t) mod.'])
plt.tight_layout()
plt.show()

#Plot the average time-series
print('Showing the average time-series...')
plt.figure()
h1, = plt.plot(np.mean(Ct_sig, axis=0), 'b-')
h2, = plt.plot(np.mean(Ct_mod, axis=0), 'r--')
plt.xlabel('Dynamic sequence')
plt.ylabel('C(t)')
plt.legend((h1,h2), ['C(t) sig. avg', 'C(t) mod. avg'])
plt.title('Mean average of concentration time-series')
plt.show()
#%%
# Below we create an instance of the viewer tool - note this can be
# tempermental running in a notebook. Calling the tool directly from
# the command line is recommended instead, eg:
# python -m QbiPy.tools.DCE_fit_viewer.DCE_fit_viewer_tool test_dataset/madym_output/ETM_auto 01d .hdr
from PyQt5.QtWidgets import QApplication
from PyQt5 import QtCore

from QbiPy.tools.DCE_fit_viewer import DCE_fit_viewer_tool 

# Create an appliction object and launch the tool. The tools displays the 
# TK model fit for every voxel in the ROI mask of the madym folder
print('Launching the fit viewer GUI...')
if __name__ == '__main__':
    app = QtCore.QCoreApplication.instance()
    if app is None:
        app = QApplication(sys.argv)


    myapp = DCE_fit_viewer_tool.DCEFitViewerTool(
        DCE_dir, index_format = '01d', image_format='.hdr')
    myapp.show()
    myapp.load_data()
    app.exec_()



# %%
