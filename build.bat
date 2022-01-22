@echo off

rem default behavior (no arguments) is build in debug mode
if [%1]==[] goto debug

if /I %1==clean goto clean
if /I %1==release goto release
if /I %1==debug goto debug

echo Usage: build (debug / release / clean)
goto exit

:debug
set BUILD_TYPE_ARG="Debug"
goto build

:release
set BUILD_TYPE_ARG="Release"
goto build

:build
set PROJECT_ROOT_WIN=%CD%
set PROJECT_ROOT=%PROJECT_ROOT_WIN:\=/%
set TOOLCHAIN_ARG="%PROJECT_ROOT%/vcpkg/scripts/buildsystems/vcpkg.cmake"

echo Project root    = %PROJECT_ROOT%
echo VCPKG toolchain = %TOOLCHAIN_ARG%
echo Build type      = %BUILD_TYPE_ARG%

if not exist out mkdir out
cd out

if not exist build mkdir build
cd build

if not exist "x64-%BUILD_TYPE_ARG%" mkdir "x64-%BUILD_TYPE_ARG%"
cd "x64-%BUILD_TYPE_ARG%"

echo Working directory: %CD%

"C:\Program Files\CMake\bin\cmake.exe" -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_ARG% -DCMAKE_BUILD_TYPE=%BUILD_TYPE_ARG% "%PROJECT_ROOT%""
"C:\Program Files\CMake\bin\cmake.exe" --build .

cd ..
cd ..

goto exit

:clean
if exist out rd out /s /q
goto exit

:exit