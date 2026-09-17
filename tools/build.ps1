param(
    [string]$Project = 'all',
    [ValidateSet(1, 2, 3)][int]$LogLevel = 3
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'enter-environment.ps1')
$repoRoot = Split-Path $PSScriptRoot
$projects = @('112-blink', '121-blink-reg', '122-led-button',
    '123-led-button-debounce', '131-hello-usb', '133-led-button-usb', '134-led-module')
if ($Project -ne 'all') {
    if ($Project -notin $projects) { throw "Unknown project: $Project" }
    $projects = @($Project)
}
foreach ($name in $projects) {
    $source = Join-Path $repoRoot $name
    $folder = if ($LogLevel -eq 3) { 'build' } else { "build-log$LogLevel" }
    $build = Join-Path $source $folder
    $configure = @('-S', $source, '-B', $build, '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', "-Dpicotool_DIR=$env:picotool_DIR")
    if ($name -eq '134-led-module') { $configure += "-DLOG_LEVEL=$LogLevel" }
    & cmake @configure
    if ($LASTEXITCODE -ne 0) { throw "Configuration failed: $name" }
    & cmake --build $build --parallel 4
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $name" }
    $uf2 = Join-Path $build ($name.Replace('-', '_') + '.uf2')
    if (-not (Test-Path $uf2)) { throw "UF2 missing: $uf2" }
    Write-Output "Built $uf2"
}
