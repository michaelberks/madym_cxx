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
        --remove Microsoft.VisualStudio.Component.Windows10SDK.10240 `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.10586 `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.14393 `
        --remove Microsoft.VisualStudio.Component.Windows81SDK `
        || IF "%ERRORLEVEL%"=="3010" EXIT 0) `
    `
    # Cleanup
    && del /q vs_buildtools.exe
    
RUN `
    #Download chocolatey
    powershell -Command Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    
RUN `
    #Install, git, boost and dcmtk
    choco install git boost-msvc-14.2 -y
    
RUN `
    "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat"
    
RUN ` #Build zlib
    git clone https://github.com/madler/zlib/ `
    && mkdir zlib_build `
    && cd zlib_build `
    && cmake ..\\zlib `
    && cmake --build . --config Release `
    && cmake --install . --config Release `
    && cd ..
    
RUN ` #Build dcmtk, installing from chocolatey didn't work for me 
    git clone https://github.com/DCMTK/dcmtk `
    && mkdir dcmtk_build `
    && cd dcmtk_build `
    && cmake ..\\dcmtk -D DCMTK_COMPILE_WIN32_MULTITHREADED=ON `
    && cmake --build . --config Release `
    && cmake --install . --config Release `
    && cd ..

# Define the entry point for the docker container.
ENTRYPOINT ["cmd"]