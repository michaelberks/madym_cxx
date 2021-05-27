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
    affiliation: "2 3"
  - name: Ross A Little
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

`Madym` is a C++ toolkit for quantitative DCE-MRI analysis developed in the 
Quantitative Biomedical Imaging Lab at the University of Manchester. It comprises 
a set of command line tools and a graphical user-interface based on an extendable 
C++ library. It is cross-platform, and requires few external libraries to 
build from source. Pre-built binaries for Windows, MacOS and Linux are available 
on request. We have also developed complementary interfaces in Matlab and python
(available in separate open-source repositories [@madym-matlab], [@madym-python]), 
that allow the flexibility of developing in those scripting languages, 
while allowing C++ to do the heavy-duty computational work of tracer-kinetic model fitting.

`Madym` has been designed with the following principles: 

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
