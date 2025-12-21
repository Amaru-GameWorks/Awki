@echo off

if not exist Build mkdir Build
pushd Build
cmake .. -G "Visual Studio 18 2026" -A x64 -Wno-dev
popd
pause