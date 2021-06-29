# escape=`

# Use the latest Windows Server Core image with .NET Framework 4.8.
FROM mcr.microsoft.com/dotnet/framework/sdk:4.8-windowsservercore-ltsc2019

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN `
    # Download the Build Tools bootstrapper.
    curl -SL --output vs_buildtools.exe https://aka.ms/vs/16/release/vs_buildtools.exe `
    `
    # Install Build Tools with the --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended workload, excluding workloads and components with known issues.
    && (start /w vs_buildtools.exe --quiet --wait --norestart --nocache modify `
        --installPath "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools" `
        --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended `
        --add Microsoft.VisualStudio.Component.Git `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.10240 `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.10586 `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.14393 `
        --remove Microsoft.VisualStudio.Component.Windows81SDK `
        || IF "%ERRORLEVEL%"=="3010" EXIT 0) `
    `
    # Cleanup
    && del /q vs_buildtools.exe

# Rather than trying to install the deps from cmd line, *much* easier just to copy local
# build context. The downside is this image can't be built from scratch on any host machine
# but that shouldn't be a problem for our current needs    
COPY Qt Qt

COPY boost boost

COPY DCMTK DCMTK

COPY zlib zlib

COPY CMake CMake

COPY NSIS NSIS

ENV BOOST_ROOT C:\\boost\\boost_1_76_0

#Put NSIS where CMake expects it to be, and add git to the path
RUN `
    move "C:\\NSIS" "C:\\Program Files (x86)\\" && `
    setx PATH "%PATH%;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\Common7\\IDE\\CommonExtensions\\Microsoft\\TeamFoundation\\Team Explorer\\Git\\cmd;C:\\Program Files (x86)\\NSIS\\Bin"
    
# Define the entry point for the docker container, running vcvars64 to set-up MSVC x64 build environment
ENTRYPOINT ["C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat", "&&", "cmd"]