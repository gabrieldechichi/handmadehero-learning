# 1 - Create a file name config.ps1 and copy the content below
# 2 - Replace the variables with the paths of your local machine

$project_vars = @{ 
    vs_devshell_path = "C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\Tools\Microsoft.VisualStudio.DevShell.dll";
    vs_shell_hash = "bf560706";
    project_root = "C:\dev\src\github.com\gabrieldechichi\handmade-hero"
}

Set-Variable -Name 'project_vars' -Value $project_vars -Scope global