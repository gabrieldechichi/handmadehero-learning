@echo off

mkdir %project_root%\build
pushd %project_root%\build
cl -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib
popd