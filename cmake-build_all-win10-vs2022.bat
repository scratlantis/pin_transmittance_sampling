:start
rmdir /s /q build
mkdir build
cd build
cmake -Dbuild_all_versions=true -G "Visual Studio 17 2022" ..
cd ..
echo off
set proceed=o
set /p proceed=open project/rerun/exit? [o/r/e] (default - %proceed%)?:

if %proceed%==r (goto start)
if %proceed%==o (start devenv.exe .\build\master_thesis.sln)