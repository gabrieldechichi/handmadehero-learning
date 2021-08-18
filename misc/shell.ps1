. "$PSScriptRoot\config.ps1"

Import-Module $project_vars.vs_devshell_path
Enter-VsDevShell $project_vars.vs_shell_hash

Push-Location $project_vars.project_root