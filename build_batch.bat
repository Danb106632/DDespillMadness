@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
setlocal enabledelayedexpansion

REM Define Nuke version to path mappings
set "NUKE[14.0]=C:\Program Files\Nuke14.0v6"
set "NUKE[14.1]=C:\Program Files\Nuke14.1v6"
set "NUKE[15.2]=C:\Program Files\Nuke15.2v1"
set "NUKE[16.0]=C:\Program Files\Nuke16.0v3"


REM Define list of versions to loop through
set "VERSIONS=14.0 14.1 15.2 16.0"

for %%V in (%VERSIONS%) do (
    set "NUKEVERISON=!NUKE[%%V]!"

    echo.
    echo === Building with Nuke version %%V ===
    echo Path: !NUKEVERISON!

    set "build_dir=D:\NDK\D_DespillMadness\%%V"

    del "D:\NDK\D_DespillMadness\CMakeCache.txt" > nul 2>&1

    cmake -DCMAKE_PREFIX_PATH="!NUKEVERISON!" -A x64 -S "D:\NDK\D_DespillMadness" -B "D:\NDK\D_DespillMadness"
    cmake --build . --config Release

    mkdir ""D:\NDK\D_DespillMadness\Release\%%V\"
    move "D:\NDK\D_DespillMadness\Release\D_DespillMadness.dll" "D:\NDK\D_DespillMadness\Release\%%V\D_DespillMadness.dll"

    mkdir "C:\Users\danie\.nuke\D_DespillMadness\%%V" 2>nul & copy /Y "D:\NDK\D_DespillMadness\Release\%%V\D_DespillMadness.dll" "C:\Users\danie\.nuke\D_DespillMadness\%%V\D_DespillMadness.dll"
)

echo.
echo ✅ All builds completed.

call "C:\Program Files\Nuke16.0v3\Nuke16.0.exe" --nukex