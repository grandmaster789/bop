@echo off

rem using ctest didn't work for me, but it should probably be:
rem "C:\Program Files\CMake\bin\ctest.exe" .. -C Release

cd build\bin
unittest
cd ..\..