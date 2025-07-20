
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cmake -DCMAKE_PREFIX_PATH="C:\Program Files\Nuke16.0v3" -A x64 -S "D:\NDK\D_DespillMadness" -B "D:\NDK\D_DespillMadness"
cmake --build . --config Release

copy /Y "D:\NDK\D_DespillMadness\Release\D_DespillMadness.dll" "C:\Users\danie\.nuke\D_DespillMadness\16.0\D_DespillMadness.dll"
