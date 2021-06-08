---
title: 'Madym: A C++ toolkit for quantitative DCE-MRI analysis'
tags:
  - C++
  - DCE-MRI
  - tracer-kinetic models
  - pharmacokinetics
authors:
  - name: Michael Berks^[corresponding author]
    orcid: 0000-0003-4727-2006
    affiliation: 1
  - name: Geoff J M Parker
    orcid: 0000-0003-2934-2234
    affiliation: "2, 3"
  - name: Ross Little
    affiliation: 1
  - name: Sue Cheung
    affiliation: 1
affiliations:
 - name: Quantitative Biomedical Imaging Laboratory, Division of Cancer Sciences, Manchester, UK
   index: 1
 - name: Centre for Medical Image Computing, University College London, London, UK
   index: 2
 - name: Bioxydyn Ltd, Manchester, UK
   index: 3
date: 27 May 2021
bibliography: paper.bib

---

# Summary
In dynamic contrast-enhanced magnetic resonance imaging (DCE-MRI) 
a sequence of MRI images are acquired to measure the passage of a
contrast-agent within a tissue of interest. Quantitative DCE-MRI (DCE-MRI), in which
one or more tracer-kinetic models are fitted to the contrast-agent
concentration time-series, enables the estimation of clinically useful parameters of tissue microvasculature.

`Madym` is a C++ toolkit for quantitative DCE-MRI analysis developed at the University of Manchester. It comprises 
a set of command line tools and a graphical user-interface based on an extendable 
C++ library. It is cross-platform, and requires few external libraries to 
build from source. Pre-built binaries for Windows, MacOS and Linux are available 
on request. We have also developed complementary interfaces in Matlab and python
(available in separate open-source repositories [@madym-matlab], [@madym-python]), 
that allow the flexibility of developing in those scripting languages, 
while allowing C++ to do the heavy-duty computational work of tracer-kinetic model fitting.

# Statement of need
`Madym` has been designed with the following principles:

- Ease-of-use: `Madym` supports many advanced features for DCE-MRI analysis, however
the tools have been designed to be usable by anyone, including clinical scientists
with no software/programming knowledge. Extensive documentation is provided on the 
project wiki [@madym-wiki],
and an example test set is included with the toolkit, including walk through 
instructions of how to perform a standard analysis on these data.

- Reproducible research: even the simplest DCE-MRI analysis pipeline
requires configuring many parameters (*ie* typically more than 20), which in some packages
may be implicitly encoded in sub-methods, and
may therefore differ in non-transparent ways between different implementations of the 
same analysis pipeline. Wherever possible, `Madym` exposes all parameters as configurable options,
with a single source file specifying their default values used throughout the toolkit.
A consistent interface is provided for configuring individual options, either via
input config files, setting options directly at the command-line or adjusting interactively
in the GUI. Whenever an analysis is run, the complete configuration - including the final
set of parameter option values, the version of `Madym` used and the machine ID on which the analysis
was run is saved with the output results. Thus `Madym` provides both flexibility in
configuring analyses to individual datasets, while supporting reproducibility with a complete
record of how results were obtained. In doing so we support the aims of the
ISMRM (International Society for Magnetic Resonance in Medicine) Reproducible Research Study Group [@stikov:2019], and
have listed `Madym` on the ISMRM MR-hub [@ismrm-mr-hub]. 

- Extensibility: `Madym` includes several of the most commonly used tracer-kinetic models
as standard, including the Patlak [@patlak:1983], extend-Tofts [@tofts:1997] and two compartment exchange models [@sourbron:2009], as well
as more complex models for fitting contrast-agents that are actively metabolised by tissue
and/or require dual vascular supply functions [@berks:2021]. However these are by no means an exhaustive list
and, by decoupling model optimisation from the model definitions, the toolkit has been designed to 
make adding new models very easy, simply by sub-classing the main abstract model class. Instructions
for doing so are provided in the project wiki. Extending $T_1$ fitting methods (currently variable flip-angle and
inversion recovery methods are supported),
 or even adding a new command-line tool, are designed and documented in the same way.

 - Performance: `Madym` is designed with the aim of voxel-wise model fitting (where a model is fitted to
 individual tissue voxels rather than spatially averaged regions-of-interest). 3D MRI images have many 
 hundreds of thousands of voxels (*eg* a typical image may have dimensions 128 x 128 x 40 = 655,360 voxels).
 By using C++ and externally developed open-source optimisation library (ALGLIB, [@alglib]), on a standard desktop
 `Madym` requires $\approx 10 {\mu}s$ per voxel to estimate baseline $T_1$ (allowing $T_1$ mapping of whole volumes in a few seconds) 
 and $< 30 ms$ per voxel to fit the extended-Tofts model (so a typical tumour of 500-1,000 voxels can be analysed in 20-30 seconds, 
 while whole organs can be fitted in a few hours). 

`Madym` has been developed over approximately 20 years and has been used to perform 
DCE-MRI analyses in more than 20 research papers and many more conference abstracts
(landmark papers include [@jayson_natcom:2018] and [@oconnor_nrco:2012], see the project wiki for a more complete list). Until
this year, these used previous non-open source versions, however the first paper
using `Madym` as an open-source toolkit has just been published [@berks:2021], and we hope will be the
first of many in the future.

# Acknowledgements

Code updates to the open source version of `Madym` have been made entirely by Michael Berks, with significant input on the design
and features of the new version from the remaining authors. We would also like to thank Gio Buonnacorsi, Anita Banerji and Angela Caunce
for their work in developing previous non-public versions, following Geoff Parker's original design.

# References
