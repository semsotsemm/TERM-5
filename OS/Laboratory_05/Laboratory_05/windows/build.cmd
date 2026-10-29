@echo off
setlocal
cd /d "%~dp0"
set "LAB_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%LAB_VSWHERE%" exit /b 1
for /f "usebackq tokens=*" %%i in (`"%LAB_VSWHERE%" -latest -requires Microsoft.Component.MSBuild -property installationPath`) do set "LAB_VS=%%i"
if not defined LAB_VS exit /b 1
"%LAB_VS%\MSBuild\Current\Bin\MSBuild.exe" Laboratory_05.slnx /m /p:Configuration=Release /p:Platform=x64 /v:minimal
if errorlevel 1 exit /b 1
copy /y tasks.csv bin\x64\Release\tasks.csv >nul
echo Build complete: bin\x64\Release
