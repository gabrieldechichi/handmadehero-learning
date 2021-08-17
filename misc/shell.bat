@echo off
call %~dp0\config.bat
call %vcvarsall_path% x64
pushd %project_root%