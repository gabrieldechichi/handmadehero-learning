& "$PSScriptRoot/../misc/config.ps1"

$build_path = "$($project_vars.project_root)\build"

if (!(Test-Path $build_path)) {
    mkdir $build_path
}

pushd $build_path
cl -FC -Zi '../code/win32/win32_handmade.cpp' user32.lib Gdi32.lib
popd