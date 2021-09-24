function Format-Color([hashtable] $Colors = @{}, [switch] $SimpleMatch) {
    $lines = ($input | Out-String) -replace "`r", "" -split "`n"
    foreach ($line in $lines) {
        $color = ''
        foreach ($pattern in $Colors.Keys) {
            if (!$SimpleMatch -and $line -match $pattern) { $color = $Colors[$pattern] }
            elseif ($SimpleMatch -and $line -like $pattern) { $color = $Colors[$pattern] }
        }
        if ($color) {
            Write-Host -ForegroundColor $color $line
        }
        else {
            Write-Host $line
        }
    }
}


& "$PSScriptRoot/../misc/config.ps1"

$build_path = "$($project_vars.project_root)\build"

if (!(Test-Path $build_path)) {
    mkdir $build_path
}

pushd $build_path
cl -FC -Zi '../code/win32/win32_handmade.cpp' user32.lib Gdi32.lib | Format-Color @{ 'warning' = 'Yellow'; 'error' = 'Red' }
popd