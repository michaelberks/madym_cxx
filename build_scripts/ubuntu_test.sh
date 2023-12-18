#!/bin/sh

#Get the previous version from binaries repo
OLD_VER=`cat ../madym_binaries/VERSION | sed 's/v//g'`

#Replace instances of the old version with the new version
cd ../madym_binaries
echo Replacing references to $OLD_VER with $1 
find . -type f -name '*.html' | xargs sed -i "s/$OLD_VER/$1/g"
sed -i "s/$OLD_VER/$1/g" .gitlab-ci.yml VERSION
