#!/bin/sh

#Get the previous version from binaries repo
OLD_VER=`cat ../madym_binaries/VERSION | sed 's/v//g'`

git fetch
git checkout -b "release-$1" "origin/release-$1"
git describe --tags --abbrev=0

#Check binaries repo is up to date to avoid conflicts
cd ../madym_binaries
git pull

#Build the release version with GUI and copy the installer to the binaries repo
cd ../madym_cxx_builds/unix_release
cmake .
make all
make install
make package
cp madym_v$1*.* "../../madym_binaries/public/ubuntu"

#Build the release version w/o GUI and copy the installer to the binaries repo
cd ../unix_release_no_gui
cmake .
make all
make install
make package
cp madym_v$1*-no_gui.* "../../madym_binaries/public/ubuntu"

#Replace instances of the old version with the new version
cd ../../madym_binaries
echo Replacing references to $OLD_VER with $1 
find . -type f -name '*.html' | xargs sed -i "s/$OLD_VER/$1/g"
sed -i "s/$OLD_VER/$1/g" .gitlab-ci.yml VERSION

#Commit and push the new installers in the binaries repo
git status
git add .
git commit -m "Adding ubuntu installers for v$1"
git pull
git push

#Build and push a new docker image with the latest release version
cd public/ubuntu
docker build --build-arg MADYM_VERSION=$1 -t registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_release:$1 -t registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_release:latest .
docker push -a registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_release
