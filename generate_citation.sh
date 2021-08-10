#!/bin/bash
git fetch
curr_date=$(date +'%Y-%m-%d')
version=$(git describe --tags --abbrev=0 | sed 's/v//g')
doi='10.5281/zenodo.5176080'
echo "cff-version: 1.2.0" > CITATION.cff
echo 'message: "If you use this software, please cite it as below."' >> CITATION.cff
echo 'authors:' >> CITATION.cff
echo '- family-names: "Berks"' >> CITATION.cff
echo '  given-names: "Michael"' >> CITATION.cff
echo '  orcid: "https://orcid.org/0000-0003-4727-2006"' >> CITATION.cff
echo '- family-names: "Parker"' >> CITATION.cff
echo '  given-names: "Geoff J M"' >> CITATION.cff
echo '  orcid: "https://orcid.org/0000-0003-2934-2234"' >> CITATION.cff
echo '- family-names: "Little"' >> CITATION.cff
echo '  given-names: "Ross"' >> CITATION.cff
echo '- family-names: "Cheung"' >> CITATION.cff
echo '  given-names: "Susan"' >> CITATION.cff
echo 'title: "Madym: quantitative analysis software for perfusion-MRI"' >> CITATION.cff
echo "version: $version" >> CITATION.cff
echo "doi: $doi" >> CITATION.cff
echo "date-released: $curr_date" >> CITATION.cff
echo 'url: "https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx"' >> CITATION.cff
cat CITATION.cff