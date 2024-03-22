#!/bin/sh
cd ../madym_binaries
git pull

cd ../madym_cxx_builds/macOS10
xcrun stapler staple "madym_v$1-Darwin-20.6.0.dmg"
cp madym_v$1*.* "../../madym_binaries/public/mac_os11"

cd ../macOS10_no_gui
xcrun stapler staple "madym_v$1-Darwin-20.6.0-no_gui.dmg"
cp madym_v$1*-no_gui.* "../../madym_binaries/public/mac_os11"

cd ../../madym_binaries
git status
git add .
git commit -m "Adding mac_os11 installers for v$1"
git push