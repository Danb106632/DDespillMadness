@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
setlocal enabledelayedexpansion

REM Define Nuke version to path mappings
set "NUKE[14.1]=C:\Program Files\Nuke14.1v6"
set "NUKE[15.2]=C:\Program Files\Nuke15.2v1"
set "NUKE[16.0]=C:\Program Files\Nuke16.0v3"
set "NUKE[13.2]=C:\Program Files\Nuke13.2v9"


REM Define list of versions to loop through
set "VERSIONS=13.2 14.1 15.2 16.0"

for %%V in (%VERSIONS%) do (
    set "NUKEVERISON=!NUKE[%%V]!"

    echo.
    echo === Building with Nuke version %%V ===
    echo Path: !NUKEVERISON!

    set "build_dir=D:\NDK\DDespillMadness\%%V"

    del "D:\NDK\DDespillMadness\CMakeCache.txt" > nul 2>&1

    cmake -DCMAKE_PREFIX_PATH="!NUKEVERISON!" -A x64 -S "D:\NDK\DDespillMadness" -B "D:\NDK\DDespillMadness"
    cmake --build . --config Release

    if %ERRORLEVEL% NEQ 0 (
    	echo Build failed. Aborting.
    	exit /b %ERRORLEVEL%
    )

    mkdir ""D:\NDK\DDespillMadness\Release\%%V\"
    move "D:\NDK\DDespillMadness\Release\DDespillMadness.dll" "D:\NDK\DDespillMadness\Release\%%V\DDespillMadness.dll"

    mkdir "C:\Users\danie\.nuke\DDespillMadness\%%V" 2>nul & copy /Y "D:\NDK\DDespillMadness\Release\%%V\DDespillMadness.dll" "C:\Users\danie\.nuke\DDespillMadness\%%V\DDespillMadness.dll"
)

echo.
echo ✅ All builds completed.

call "C:\Program Files\Nuke16.0v3\Nuke16.0.exe" --nukex