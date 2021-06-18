#!/bin/bash
echo Running version create script
echo "#This file is auto-generated in GitLab CI and should not be edited manually" > src/QbiMadym/VERSION
git fetch
git describe --tags --abbrev=0 >> src/QbiMadym/VERSION