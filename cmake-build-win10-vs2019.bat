:start
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
cd ..
echo off
set proceed=o
set /p proceed=open project/rerun/exit? [o/r/e] (default - %proceed%)?:

if %proceed%==r (goto start)
if %proceed%==o (start devenv.exe .\build\VkBase.sln)