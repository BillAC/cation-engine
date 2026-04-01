@echo off
setlocal

:: Path to vcvars64.bat as provided by the user
set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VCVARS_PATH%" (
    echo Error: vcvars64.bat not found at "%VCVARS_PATH%"
    echo Please update the path in this script.
    exit /b 1
)

:: Call vcvars64.bat to set up MSVC environment
call "%VCVARS_PATH%"

:: Create build directory
if not exist build_msvc mkdir build_msvc
cd build_msvc

:: Configure with CMake
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..

:: Build
cmake --build . --config Release

:: Copy data files to bin directory
if not exist bin mkdir bin
copy /y ..\ligands.csv bin\
copy /y ..\metals.csv bin\

echo.
echo Build complete. Executables are in build_msvc/bin/
echo Data files copied to build_msvc/bin/
pause
