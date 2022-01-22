@echo off

rem assumes that:
rem   this is ran from the project root folder
rem   we have access to git 
rem   we can build from the commandline

if not exist vcpkg goto clone_vcpkg
goto build_vcpkg

:clone_vcpkg
git clone https://github.com/microsoft/vcpkg.git

rem possibly we should checkout a specific release

:build_vcpkg
if exist vcpkg\vcpkg.exe goto exit

cd vcpkg
call bootstrap-vcpkg.bat
cd ..

:exit