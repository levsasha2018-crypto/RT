$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path (Split-Path (Split-Path $PSScriptRoot))
$env:PICO_SDK_PATH = Join-Path $taskRoot 'work/pico-sdk'
$env:picotool_DIR = Join-Path $taskRoot 'work/tools/picotool/picotool'
$pythonScripts = python -c "import sysconfig; print(sysconfig.get_path('scripts', 'nt_user'))"
$cmakeBin = python -c "import cmake; print(cmake.CMAKE_BIN_DIR)"
$makeBin = Join-Path $env:LOCALAPPDATA 'Microsoft/WinGet/Packages/ezwinports.make_Microsoft.Winget.Source_8wekyb3d8bbwe/bin'
$toolPaths = @(
    (Join-Path $taskRoot 'work/tools/arm/bin'),
    (Join-Path $taskRoot 'work/tools/git/cmd'),
    $env:picotool_DIR,
    $cmakeBin,
    $pythonScripts,
    $makeBin
)
$env:PATH = ($toolPaths -join ';') + ';' + $env:PATH
if (-not (Test-Path (Join-Path $env:PICO_SDK_PATH 'pico_sdk_init.cmake'))) {
    throw 'SDK not found. Install Pico SDK 2.3.0 and adjust enter-environment.ps1.'
}
