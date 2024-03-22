@echo off
git fetch
git checkout -b "release-%1" "origin/release-%1"
git describe --tags --abbrev=0

cd ..\madym_binaries
git pull

setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cd ..\madym_cxx_builds\msvc2019
cmake .
cmake --build . --config Release
cmake --install . --config Release
cpack .
xcopy "madym_v%1*.*" "..\..\madym_binaries\public\windows"
cd ..\msvc2019_no_gui
cmake .
cmake --build . --config Release
cmake --install . --config Release
cpack .
xcopy "madym_v%1*-no_gui.*" "..\..\madym_binaries\public\windows"

cd ..\..\madym_binaries
git status
git add .
git commit -m "Adding windows installers for v%1"
git push
endlocal