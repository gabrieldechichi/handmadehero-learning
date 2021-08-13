@echo off

mkdir %rt%\build
pushd %rt%\build
cl -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib
popd