
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cmake -DCMAKE_PREFIX_PATH="C:\Program Files\Nuke16.0v3" -A x64 -S "D:\NDK\DDespillMadness" -B "D:\NDK\DDespillMadness"
cmake --build . --config Release

move "D:\NDK\DDespillMadness\Release\DDespillMadness.dll" "D:\NDK\DDespillMadness\Release\16.0\DDespillMadness.dll"
copy /Y "D:\NDK\DDespillMadness\Release\16.0\DDespillMadness.dll" "C:\Users\danie\.nuke\DDespillMadness\16.0\DDespillMadness.dll"

call "C:\Program Files\Nuke16.0v3\Nuke16.0.exe" --nukex