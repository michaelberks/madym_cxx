#!/bin/sh
git fetch
git checkout -b "release-$1" "origin/release-$1"
git describe --tags --abbrev=0

cd ../madym_cxx_builds/macOS10
cmake . -D CMAKE_BUILD_TYPE=Release -D CMAKE_OSX_DEPLOYMENT_TARGET=10.10
make all
make install
make package
xcrun altool --notarize-app --primary-bundle-id "com.qbi-lab.madym.mac10.gui.dmg" -f "madym_v$1-Darwin-20.6.0.dmg" -p "@keychain:AC_PASSWORD"

cd ../macOS10_no_gui
cmake . -D CMAKE_BUILD_TYPE=Release -D CMAKE_OSX_DEPLOYMENT_TARGET=10.10 
make all
make install
make package
xcrun altool --notarize-app --primary-bundle-id "com.qbi-lab.madym.mac10.no-gui.dmg" -f "madym_v$1-Darwin-20.6.0-no_gui.dmg" -p "@keychain:AC_PASSWORD"