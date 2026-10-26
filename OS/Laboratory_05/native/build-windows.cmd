@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>nul
if errorlevel 1 (
    echo Open x64 Native Tools Command Prompt for Visual Studio, then run this file.
    exit /b 1
)
if not exist bin mkdir bin
for %%p in (a x b c) do (
    cl /nologo /std:c++17 /O2 /W4 /EHsc /MT /utf-8 /Fo"bin\Lab-05%%p.obj" /Fe"bin\Lab-05%%p.exe" "Lab-05%%p.cpp"
    if errorlevel 1 exit /b 1
)
echo Build complete. Executables are in native\bin.
