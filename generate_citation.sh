#!/bin/bash
git fetch
curr_year=$(date +'%Y')
version=$(git describe --tags --abbrev=0)
echo "Berks, M., Parker, G. M. B., Little, R., & Cheung, S. ($curr_year). Madym: quantitative analysis software for perfusion-MRI (Version $version) [Software]. Available from https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx." > CITATION.cff
cat CITATION.cff